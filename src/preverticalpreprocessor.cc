#include "preverticalpreprocessor.hh"
#include "xh_scanner.hh"
#include <boost/log/trivial.hpp>
#include <fstream>
#include "preprocess/base64.hh"
#include "entities.hh"

namespace prevertical2text {
    void addSpace(std::string& plaintext) {
        if (!plaintext.empty() && !std::isspace(plaintext.back())) {
            plaintext.push_back(' ');
        }
    }

    std::string fileToStr(const std::string& filename){
        std::ifstream t(filename);
        t.seekg(0, std::ios::end);
        std::streamsize size = t.tellg();
        std::string buffer(size, ' ');
        t.seekg(0);
        t.read(&buffer[0], size);
        return buffer;
    }

    void encodeBase64(const std::string& original, std::string& base64){
        preprocess::base64_encode(original, base64);
    }

    void preverticalPreprocessor::process(const std::string& filename){
        std::string plaintext;
        std::string lang;
        std::string url;
        std::string mime;
        const std::string allxml = fileToStr(filename);
        markup::instream si(allxml.c_str());
        markup::scanner sc(si);
        std::string payload = si.p;


        int t = markup::scanner::TT_SPACE; // just start somewhere that isn't ERROR or EOF

        while (t != markup::scanner::TT_EOF) {
            t = sc.get_token();
            switch (t) {
                case markup::scanner::TT_ERROR:
                    BOOST_LOG_TRIVIAL(trace) << "Prevertical document " << url << ": parsing error";
                    return;
                case markup::scanner::TT_TAG_START:
                case markup::scanner::TT_WORD:
                    plaintext.append(sc.get_value());
                    break;
                case markup::scanner::TT_SPACE:
                    addSpace(plaintext);
                    break;
                default:
                    break;
            }
            std::string tag = std::string(sc.get_tag_name());
            if (t == markup::scanner::TT_TAG_START and tag == "doc"){
                ++totalRecords;
            }
            // Look for the attributes of the doc tag and store them as prevertical document metadata
            else if (t == markup::scanner::TT_ATTR and tag == "doc"){
                std::string attr = std::string(sc.get_attr_name());
                if (attr == "lang") {
                    lang = sc.get_value();
                }
                else if (attr == "url"){
                    url = sc.get_value();
                }
                else if (attr == "enc_chared"){
                    mime = sc.get_value();
                }
            }
            // Look for the </doc> end tag and write the document data
            else if (t == markup::scanner::TT_TAG_END and tag == "doc"){
                if (plaintext.back() != '\n') plaintext.push_back('\n');
                std::string base64text;
                std::string base64html;
                std::string textwithentities = plaintext;
                std::string exact_payload;
                exact_payload = payload.substr(0, payload.find("</doc>")+6);
                entities::decodeEntities(textwithentities, plaintext);
                encodeBase64(plaintext, base64text);
                encodeBase64(exact_payload, base64html);

                writer.write(lang, base64text, url, mime, base64html);
                ++textRecords;
                textBytes += plaintext.size();
                totalBytes += exact_payload.size();
                plaintext = "";
                payload = si.p;
            }
        }
    }

    void preverticalPreprocessor::printStatistics() const{
        BOOST_LOG_TRIVIAL(info) << "total records: " << totalRecords;
        BOOST_LOG_TRIVIAL(info) << "text records: " << textRecords;

        BOOST_LOG_TRIVIAL(info) << "total bytes: " << totalBytes;
        BOOST_LOG_TRIVIAL(info) << "text bytes: " << textBytes;
    }

    preverticalPreprocessor::preverticalPreprocessor(const std::string &outputFolder,
                                                     const std::unordered_set<std::string> &output_files) :
        writer(outputFolder, output_files),
                totalRecords(0),
                textRecords(0),
                totalBytes(0),
                textBytes(0){}

}
