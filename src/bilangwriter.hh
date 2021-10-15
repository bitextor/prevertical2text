#ifndef PREVERTICAL2TEXT_WRITER_HH
#define PREVERTICAL2TEXT_WRITER_HH

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "zlib.h"
#include <string>

namespace prevertical2text {

    class GzipWriter {
        private:
            FILE* dest;
            z_stream s{};
            unsigned char* buf;
            std::size_t compressed;
            void compress(const char* in, std::size_t size, int flush);

        public:
            GzipWriter();
            ~GzipWriter();
            void open(const std::string& filename);
            void write(const char* text, std::size_t size);
            void writeLine(const char* text, std::size_t size);
            void writeLine(const std::string& text);
            bool is_open();
            static const std::size_t BUFFER_SIZE = 4096;
    };

    class BilangWriter {
        private:
            std::string folder;
            std::unordered_map<std::string, GzipWriter> url_files;
            std::unordered_map<std::string, GzipWriter> mime_files;
            std::unordered_map<std::string, GzipWriter> text_files;
            std::unordered_map<std::string, GzipWriter> html_files;
            std::unordered_set<std::string> output_files;


        public:
            explicit BilangWriter(std::string folder) :
                folder(std::move(folder)),
                url_files(),
                mime_files(),
                text_files(),
                html_files(),
                output_files({}) // url and text are mandatory regardless
            {};

            explicit BilangWriter(std::string folder, std::unordered_set<std::string>  output_files) :
                folder(std::move(folder)),
                url_files(),
                mime_files(),
                text_files(),
                html_files(),
                output_files(std::move(output_files))
            {};
            void write(const std::string& lang, const std::string& b64text, const std::string& url, const std::string& mime, const std::string& b64html);
    };


}

#endif
