#include "preverticalpreprocessor.hh"
#include "xh_scanner.hh"
#include <boost/log/trivial.hpp>
#include <boost/locale.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include "preprocess/base64.hh"
#include "entities.hh"

namespace prevertical2text {
    void addSpace(std::string& plaintext) {
        if (!plaintext.empty() && !std::isspace(plaintext.back())) {
            plaintext.push_back(' ');
        }
    }
    void addNewLine(std::string& plaintext) {
        if (!plaintext.empty() and std::isspace(plaintext.back())) {
            plaintext.back() = '\n';
        } else if (!plaintext.empty()) {
            plaintext.push_back('\n');
        }
    }

    std::string toUTF8(const std::string& text, const std::string& charset) {
        return boost::locale::conv::to_utf<char>(text, charset);
    }

    std::string toUTF8(const char* text, const std::string& charset) {
        return boost::locale::conv::to_utf<char>(text, charset);
    }
	
    void encodeBase64(const std::string& original, std::string& base64){
        preprocess::base64_encode(original, base64);
    }

    void preverticalPreprocessor::process(const std::string& filename, bool boilerplate_removal, bool paragraph_info) {
        std::ifstream stream;
        if (filename == "-") stream = std::ifstream("/dev/stdin");
        else stream = std::ifstream(filename);

        std::string docxml;

        for (std::string line; getline(stream, line);) {
            docxml += line + "\n";
            if (line == "</doc>") {
                markup::instream si(docxml.c_str());
                markup::scanner sc(si);
                std::string payload = si.p;

                std::string plaintext;
                std::string lang;
                std::string url;
                std::string mime;
                std::string encoding_chared;
                std::string title;
                std::string crawldate;
                std::string cfclass;
                std::string heading;
		heading = "no";

                int paragraph_class = 0;

                int t = markup::scanner::TT_SPACE; // just start somewhere that isn't ERROR or EOF
                std::string tag;
                std::string attr;
                std::string value;
                int paragraph_counter;

                while (t != markup::scanner::TT_EOF) {
                    t = sc.get_token();
                    tag = std::string(sc.get_tag_name());
                    attr = std::string(sc.get_attr_name());
                    value = std::string(sc.get_value());
                    switch (t) {
                        case markup::scanner::TT_ERROR:
                            BOOST_LOG_TRIVIAL(trace) << "Prevertical document " << url << ": parsing error";
                            return;
                        case markup::scanner::TT_ATTR:
                            if (tag == "p" and attr == "cfclass")
                                cfclass = value;
                            else if (tag == "p" and attr == "heading")
                                heading = value;
                            if (boilerplate_removal and tag == "p" and attr == "class" and value == "bad")
                                paragraph_class = 1;
                            else if (boilerplate_removal and tag == "p" and attr == "class" and value == "good")
                                paragraph_class = 0;
                            break;
                        case markup::scanner::TT_TAG_END:
                            if (tag == "p") {
                                if (paragraph_class == 0) {
                                    boost::replace_all(plaintext, "\r", " ");
                                    if (std::isspace(plaintext.back()) && plaintext.back() != '\n')
                                        plaintext.pop_back();
                                    if (paragraph_info) {
                                        plaintext.push_back('\t');
                                        plaintext.append(std::to_string(paragraph_counter));
                                        plaintext.push_back('\t');
                                        plaintext.append(title);
                                        plaintext.push_back('\t');
                                        plaintext.append(crawldate);
                                        plaintext.push_back('\t');
                                        plaintext.append(mime);
                                        plaintext.push_back('\t');
                                        plaintext.append(cfclass);
                                        plaintext.push_back('\t');
                                        plaintext.append(heading);
                                    }
                                    addNewLine(plaintext);
                                }
                                paragraph_counter += 1;
                                cfclass = "";
                                heading = "no";
                            }
                            break;
                        case markup::scanner::TT_WORD:
                            if (paragraph_class == 0) plaintext.append(value);
                            break;
                        case markup::scanner::TT_SPACE:
                            if (paragraph_class == 0) addSpace(plaintext);
                            break;
                        default:
                            break;
                    }

                    if (t == markup::scanner::TT_TAG_START and tag == "doc") {
                        paragraph_counter = 0;
                        ++totalRecords;
                    }
                        // Look for the attributes of the doc tag and store them as prevertical document metadata
                    else if (t == markup::scanner::TT_ATTR and tag == "doc") {
                        if (attr == "lang") {
                            lang = value;
                        } else if (attr == "url") {
                            url = value;
                        } else if (attr == "file_type") {
                            mime = value;
                        } else if (attr == "enc_chared") {
                            encoding_chared = value;
                        } else if (attr == "title") {
                            title = value;
                        } else if (attr == "crawl_date"){
                            crawldate = value;
                        }
                    }
                        // Look for the </doc> end tag and write the document data
                    else if (t == markup::scanner::TT_TAG_END and tag == "doc") {
                        std::string base64text;
                        std::string base64html;
                        std::string textwithentities = plaintext;
                        std::string exact_payload;
                        if (!plaintext.empty()){
                            addNewLine(plaintext);
                            exact_payload = payload.substr(0, payload.find("</doc>") + 6);
                            if (exact_payload.size() < 5242880){ // 5MB
                                entities::decodeEntities(textwithentities, plaintext);
                                boost::replace_all(plaintext, "\r\n", " ");
                                boost::replace_all(plaintext, "\r", " ");
                                encodeBase64(plaintext, base64text);
                                encodeBase64(exact_payload, base64html);
                                writer.write(lang, base64text, url, mime, base64html);
                                ++textRecords;
                                textBytes += plaintext.size();
                                totalBytes += exact_payload.size();
                            }
                        }
                    }
                }
                docxml = "";
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
