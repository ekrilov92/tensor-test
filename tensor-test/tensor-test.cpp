#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

using namespace boost;
using namespace std;
using namespace filesystem;
namespace po = boost::program_options;

//структура описывающая аргументы командной строки
struct Arguments {
    //папка в которой находятся файлы для анализа
    string source_directory;
    //список папок для поиска зависимостей
    vector<string> include_directories;

    Arguments(const string & source, const vector<string> & include) : source_directory(source), include_directories(include) {
    }
};

//структура представлюящая зависимости одного файла
struct Includes {
    //список файлов, которые нужно искать локально, относительно файла
    vector<string> local_files;
    //список файлов, которые нужно искать в дополнительных директориях для включаемых файлов
    vector<string> far_files;
};

Arguments parse_arguments(int argc, char* argv[]) {
    //функция для парсинга аргументов командной строки с обрботкой возможных ошибок
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
            return Arguments("", {});
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
    //функция для анализа директории и поиска в ней списка h и cpp файлов
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

void analize_file(const string& file_path) {
    //будем считывать исходный файл по одному символу и удалим все комментарии из него
    ifstream file(file_path);
    ofstream tmp_file(file_path + "_tmp");
    char c;
    while (file.get(c)) {
        //анализ однострочного комментария
        if (c == '/') {
            //если считали / то считаем следующий символ и если там тоже / то игнорируем все до конца строки
            //если там * то игнорируем все до конца многострочного комментария
            char c1;
            file.get(c1);
            if (c1 == '/') {
                char comment_str[1024];
                file.getline(comment_str, 1024);
                //если прочитали всю строку добавим в выходной переход на новую строку.
                // поидее тут могут генерироваться лишние переносы, но это не должно сделать код хуже
                tmp_file << endl;
            }
            else if (c1 == '*') {
                char comment_end[2];
                //Считаем следующие 2 символа для начала запуска алгоритма поиска окончания комментария
                file.get(comment_end[0]);
                file.get(comment_end[1]);
                while (true) {
                    // если нашли конец комментария выходим иначе считываем следующий символ
                    if (comment_end[0] == '*' && comment_end[1] == '/') {
                        break;
                    }
                    else {
                        comment_end[0] = comment_end[1];
                        file.get(comment_end[1]);
                    }
                }
            }
            else {
                // если следующим после / оказался обычный символ то откатим буффер чтобы потом прочитать его снова
                file.unget();
            }
        }
        else
            tmp_file << c;
    }
    tmp_file.close();
    file.close();
}

int main(int argc, char * argv[])
{
    Arguments args(parse_arguments(argc, argv));
    if (args.source_directory.empty())
        return 2;
    if (!check_arguments(args))
        return 3;
    vector<string> files(build_file_list(args.source_directory));
    for (auto x : files)
        cout << x << endl;
    for (auto file_path : files)
        analize_file(file_path);
}
