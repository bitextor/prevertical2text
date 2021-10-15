#ifndef PREVERTICAL2TEXT_PREVERTICALPREPROCESSOR_HH
#define PREVERTICAL2TEXT_PREVERTICALPREPROCESSOR_HH

#include <string>
#include "bilangwriter.hh"


namespace prevertical2text {
    class preverticalPreprocessor {
        private:
            BilangWriter writer;
            unsigned int totalRecords;
            unsigned int textRecords;
            unsigned int totalBytes;
            unsigned int textBytes;

        public:
            explicit preverticalPreprocessor(const std::string& outputFolder, const std::unordered_set<std::string>& output_files = {});
            void process(const std::string& filename);
            void printStatistics() const;
    };
}

#endif
