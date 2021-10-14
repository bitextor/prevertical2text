#include "html.hh"
#include "xh_scanner.hh"
#include <boost/log/trivial.hpp>
#include <fstream>
#include "preprocess/base64.hh"
#include "bilangwriter.hh"
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

    int processHTML(const std::string& filename, const std::string& output, const std::string& output_files){
        BilangWriter writer(output);
        std::string plaintext;
        std::string payload;
        std::string lang;
        std::string url;
        std::string mime;
        const std::string allxml = fileToStr(filename);
        markup::instream si(allxml.c_str());
        markup::scanner sc(si);

        int t = markup::scanner::TT_SPACE; // just start somewhere that isn't ERROR or EOF
        int retval = 0;
        std::string();

        while (t != markup::scanner::TT_EOF and t != markup::scanner::TT_ERROR) {
            t = sc.get_token();
            switch (t) {
                case markup::scanner::TT_ERROR:
                    retval = 1;
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
            if (t == markup::scanner::TT_ATTR and tag == "doc"){
                payload = si.p;
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
            else if (t == markup::scanner::TT_TAG_END and tag == "doc"){
                if (plaintext.back() != '\n') plaintext.push_back('\n');
                std::string base64text;
                std::string base64html;
                std::string textwithentities = plaintext;
                std::string exact_payload;
                if (output_files.find("html") != std::string::npos) exact_payload = payload.substr(0, payload.find("</doc>"));
                entities::decodeEntities(textwithentities, plaintext);
                encodeBase64(plaintext, base64text);
                encodeBase64(exact_payload, base64html);

                writer.write(lang, base64text, url, mime, base64html);
                plaintext = "";
            }
        }

        return retval;
    }

}
