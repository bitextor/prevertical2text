#include <iostream>
#include <chrono>
#include <vector>
#include <unordered_set>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/algorithm/string/split.hpp>
#include "src/preverticalpreprocessor.hh"

using namespace prevertical2text;

struct Options {
    std::vector<std::string> preverticals;
    std::string files;
    bool boilerplate{};
    bool paragraph{};
    bool cld2{};
    bool verbose{};
    bool silent{};
    std::string output;
};

void parseArgs(int argc, char *argv[], Options& out) {
    namespace po = boost::program_options;
    po::options_description desc("Arguments");
    desc.add_options()
        ("help,h", po::bool_switch(), "Show this help message")
        ("output,o", po::value(&out.output)->default_value("."), "Output folder")
        ("files,f", po::value(&out.files)->default_value("url,token"), "List of output files separated by commas. Default (mandatory files): 'url,text'. Optional: 'mime,html'")
        ("input,i", po::value(&out.preverticals)->multitoken(), "Input Spiderling prevertical file name(s)")
        ("remove_boilerplate,b", po::bool_switch(&out.boilerplate)->default_value(false), "Remove boilerplate paragraphs from prevertical format, tagged as 'bad'")
        ("paragraph_info,p", po::bool_switch(&out.paragraph)->default_value(false), "Add paragraph index in each b64encoded document paragraph as tab separated column")
        ("cld2,c", po::bool_switch(&out.cld2)->default_value(false), "Use CLD2 detected language attribute instead of trigram model marked on prevertical documents")
        ("verbose,v", po::bool_switch(&out.verbose)->default_value(false), "Verbosity level")
        ("silent,s", po::bool_switch(&out.silent)->default_value(false));

    po::positional_options_description pd;
    pd.add("input", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pd).run(), vm);
    if (argc == 1 || vm["help"].as<bool>()) {
        std::cerr << "Usage: " << argv[0] << " -o <output_folder> [ -f <output_files> ] [ -b ] [ -p ] [ -c ] [ -s ] [ -v ] <prevertical_file>...\n"
                "\n"
                "Options:\n"
                " -o <output_folder>               Output folder, required\n"
                " -f <output_files>                List of output files separated by commas\n"
                "                                  Default (mandatory): \"url,text\"\n"
                "                                  Optional values: \"mime,html\"\n"
                " -b                               Remove boilerplate paragraphs\n"
                " -p                               Add paragraph index in each b64encoded document paragraph\n"
                " -c                               Use CLD2 detected language attribute instead of trigram model marked on prevertical documents\n"
                " -s                               Only output errors\n"
                " -v                               Verbose output (print trace)\n\n";
        exit(1);
    }
    po::notify(vm);
}


int main(int argc, char *argv[]) {
    // parse arguments
    Options options;
    parseArgs(argc,argv, options);

    // configure logging
    boost::log::add_console_log(std::cerr, boost::log::keywords::format = "[%TimeStamp%] [\%Severity%] %Message%");
    boost::log::add_common_attributes();
    auto verbosity_level = options.verbose ? boost::log::trivial::trace :
                           options.silent  ? boost::log::trivial::warning :
                                             boost::log::trivial::info;
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= verbosity_level);

    // prepare list of output files
    std::vector<std::string> files_list;
    boost::algorithm::split(files_list, options.files, [](char c) {return c == ',';});
    std::unordered_set<std::string> output_files(files_list.begin(), files_list.end());

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    preverticalPreprocessor preverticalpproc(options.output, output_files);
    for (const std::string& file : options.preverticals){
        preverticalpproc.process(file, options.boilerplate, options.paragraph, options.cld2);
    }
    preverticalpproc.printStatistics();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    unsigned int hours = std::chrono::duration_cast<std::chrono::hours>(end - start).count();
    unsigned int minutes = std::chrono::duration_cast<std::chrono::minutes>(end - start).count() - hours*60;
    unsigned int seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count() - hours*60*60 - minutes*60;
    BOOST_LOG_TRIVIAL(info) << "elapsed: " << hours << "h" << minutes << "m" << seconds << "s";

    return 0;
}
