#include <boost/program_options.hpp>
#include <iostream>
#include "filesanalizer.h"

using namespace boost;
using namespace std;
using namespace filesystem;

//структура описывающая аргументы командной строки
struct Arguments {
    //папка в которой находятся файлы для анализа
    string source_directory;
    //список папок для поиска зависимостей
    vector<path> include_directories;

    Arguments(const string & source, const vector<path> & include) : source_directory(source), include_directories(include) {
    }
};

Arguments parse_arguments(int argc, char* argv[]) {
    //функция для парсинга аргументов командной строки с обрботкой возможных ошибок
    using namespace boost::program_options;
    options_description desc("Allowed options");
    desc.add_options()
        ("source", value<string>(), "directory with source files")
        ("include,I", value<vector<string>>(), "directory with include files");

    positional_options_description p;
    p.add("source", 1);

    try {
        variables_map vm;
        store(command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        notify(vm);

        if (vm.count("source") == 0) {
            cout << "Directory with source files is not specified" << endl;
            cout << desc;
            return Arguments("", {});
        }
        string source = vm["source"].as<string>();
        vector<string> include;
        vector<path> p_include;
        if (vm.count("include")) {
            include = vm["include"].as<vector<string>>();
        }
        for (const string& x : include)
            p_include.push_back(path(x));
;        return Arguments(source, p_include);
    }
    catch (error e) {
        cout << "Command line arguments error" << endl;
        cout << desc;
    }
    return Arguments("", {});
}

bool check_arguments(const Arguments& args) {
    //функция для проверки корректности введенных аргументов
    path source_path(args.source_directory);
    directory_entry source_dir(source_path);
    if (!source_dir.exists()) {
        cout << "Directory with source files not exist";
        return false;
    }
    for (const path & dir : args.include_directories) {
        directory_entry include_dir(dir);
        if (!include_dir.exists()) {
            cout << "Directory with include files not exist";
            return false;
        }
    }
    return true;
}

int main(int argc, char * argv[])
{
    Arguments args(parse_arguments(argc, argv));
    if (args.source_directory.empty())
        return 2;
    if (!check_arguments(args))
        return 3;
    FilesAnalizer analizer;
    analizer.analize_files(args.source_directory, args.include_directories);
}
