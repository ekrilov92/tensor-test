﻿#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <filesystem>

using namespace boost;
using namespace std;
using namespace filesystem;
namespace po = boost::program_options;

struct Arguments {
    string source_directory;
    vector<string> include_directories;

    Arguments(const string & source, const vector<string> & include) : source_directory(source), include_directories(include) {
    }
};

Arguments parse_arguments(int argc, char* argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("source", po::value<string>(), "directory with source files")
        ("include,I", po::value<vector<string>>(), "directory with include files");

    po::positional_options_description p;
    p.add("source", 1);

    try {
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("source") == 0) {
            cout << "Directory with source files is not specified" << endl;
            cout << desc;
            return Arguments(string(), {});
        }
        string source = vm["source"].as<string>();
        vector<string> include;
        if (vm.count("include")) {
            include = vm["include"].as<vector<string>>();
        }
        return Arguments(source, include);
    }
    catch (po::error e) {
        cout << "Command line arguments error" << endl;
        cout << desc;
    }
    return Arguments(string(), {});
}

bool check_arguments(const Arguments& args) {
    path source_path(args.source_directory);
    directory_entry source_dir(source_path);
    if (!source_dir.exists()) {
        cout << "Directory with source files not exist";
        return false;
    }
    for (const string & dir_name : args.include_directories) {
        path include_path(dir_name);
        directory_entry include_dir(include_path);
        if (!include_dir.exists()) {
            cout << "Directory with include files not exist";
            return false;
        }
    }
    return true;
}

vector<string> build_file_list(const path & directory) {
    vector<string> result;
    for (auto& p : directory_iterator(directory)) {
        if (p.is_directory()) {
            auto files(build_file_list(p.path()));
            result.reserve(result.size() + std::distance(files.begin(), files.end()));
            result.insert(result.end(), files.begin(), files.end());
            continue;
        }
        if (p.path().extension() == ".h" || p.path().extension() == ".cpp")
            result.push_back(p.path().string());
    }
    return result;
}

int main(int argc, char * argv[])
{
    Arguments args = parse_arguments(argc, argv);
    if (args.source_directory.empty())
        return 2;
    if (!check_arguments(args))
        return 3;
    vector<string> files(build_file_list(args.source_directory));
    for (auto x : files)
        cout << x << endl;

}
