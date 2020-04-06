#include <log4cxx/propertyconfigurator.h>
#include <boost/program_options.hpp>
#include <log4cxx/logger.h>
#include "util/CppFile.h"
#include "util/DirectoryEntryIterator.h"
#include <iostream>

namespace po = boost::program_options;
typedef std::string StringType;
typedef std::vector<StringType> StringStore;

// Declare the supported options.
    po::options_description
GetOptionDescription()
{
    po::options_description data
        ( "log4cxx_10_to_11 {options} {file_or_directory_list}\n\nwhere valid options are"
        );
    data.add_options()
        ("help,h", "produce help message")
        ("quiet,q", "do not print file names")
        ("verbose,v", "list the number of macros to be adjusted in each file")
        ("only_11", "modify files to work with 0.11")
        ("both_10_and_11", "modify files to work with 0.10 and 0.11")
        ("ext,e", po::value<StringStore>(), "add to the list of checked file extensions: default [.cpp, .cxx, .hpp, .h]")
        ;
    return data;
}

// Parse the program line arguments for options
void processArgs(int argc, char** argv, po::variables_map& vm)
{
    po::options_description hidden;
    hidden.add_options()
        ("file-or-dir", po::value<StringStore>(), "file of directory to process")
        ;
    po::positional_options_description p;
    p.add("file-or-dir", -1);
    po::store(po::command_line_parser(argc, argv)
        .options(GetOptionDescription().add(hidden))
        .positional(p)
        .run()
        , vm);
    po::notify(vm);
}

    static log4cxx::LoggerPtr
log_s(log4cxx::Logger::getLogger("main"));

// Scan \c file for issues with LOG4CXX_ macros, and optionally apply changes
int ProcessLog4cxxMacros(CppFile& file, bool fix, bool fix_10_and_11)
{
    int macroCount = 0;
    int fixCount = 0;
    CppFile::FunctionIterator log4cxxMacro(file, "LOG4CXX_");
    for (log4cxxMacro.Start(); !log4cxxMacro.Off(); log4cxxMacro.Forth())
    {
        ++macroCount;
        if (!log4cxxMacro.HasStatementTerminator())
        {
            ++fixCount;
            if (fix)
                log4cxxMacro.AddSemicolon();
            if (fix_10_and_11 && log4cxxMacro.IsCompoundStatementBody())
                log4cxxMacro.InsertBraces();
        }
    }
    return fixCount;
}

int main( int argc, char* argv[] )
{
    bool ok = false;
    try
    {
        po::variables_map vm;
        processArgs(argc, argv, vm);
        bool changeFiles = vm.count("both_10_and_11") || vm.count("only_11");
        bool for_10_and_11 = vm.count("both_10_and_11");
        bool quiet = vm.count("quiet");
        bool verbose = vm.count("verbose");

        if (!vm.count("file-or-dir") || vm.count("help"))
            std::cout << "Requires the directory or file in which to check log4cxx macro usage.\n\n"
                << GetOptionDescription() << "\n";
        else
        {
            log4cxx::PropertyConfigurator::configure("log4cxx_10_to_11.properties");
            StringStore itemStore = vm["file-or-dir"].as<StringStore>();
            StringStore extStore = {".cpp", ".cxx", ".hpp", ".h"};
            if (vm.count("ext"))
            {
                StringStore extra = vm["ext"].as<StringStore>();
                extStore.insert(extStore.end(), extra.begin(), extra.end());
            }
            DirectoryEntrySelectorPtr selector(new ExtensionSelector(extStore.begin(), extStore.end()));
            DirectoryEntryIterator fileIter(itemStore.begin(), itemStore.end(), selector);
            for (fileIter.Start(); !fileIter.Off(); fileIter.Forth())
            {
                CppFile file(fileIter.Item());
                if (!file.IsValid())
                    std::cerr << "Skipping invalid " << fileIter.Item() << "\n";
                 else if (int fixCount = ProcessLog4cxxMacros(file, changeFiles, for_10_and_11))
                 {
                     if (!quiet)
                     {
                        std::cout << fileIter.Item().string();
                        if (verbose)
                            std::cout << ": " << fixCount;
                        std::cout << "\n";
                    }
                    if (changeFiles)
                        file.StoreFile(fileIter.Item());
                }
            }
        }
        ok = true;
    }
    catch (std::exception& ex)
    {
        LOG4CXX_ERROR(log_s, ex.what());
        std::cerr << ex.what();
    }
    return ok ? 0 : 1;
}
