#include <iostream>
#include <fstream>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "filesanalizer.h"

using namespace std;
using namespace boost;

void FilesAnalizer::analize_files(const string& source_directory, const vector<path>& include_directories)
{
    /*
        список всех файлов для анализа, потом в этот список будут попадать и другие файлы которые встречаются в дерективах include
        параллельно является списком вершин графа для построения вывода
    */
    vector<File> files(this->build_file_list(source_directory));
    vector<vector<size_t>> g(files.size()); //ребра связности файлов
    //для каждого файла надем все его зависимости, далее найдем существуют ли на самом деле файлы затем обновим граф
    size_t size = files.size();
    for (size_t i = 0; i < size; i++) {
        const File& file = files[i];
        Includes dependencies(this->analize_file(file.path));
        this->find_files(file.path, dependencies, include_directories);
        //cout << file.path << endl << dependencies << "------" << endl;
        auto includes(std::move(dependencies.local_files));
        includes.reserve(includes.size() + std::distance(files.begin(), files.end()));
        includes.insert(includes.end(), dependencies.far_files.begin(), dependencies.far_files.end());
        for (auto& include : includes) {
            auto it = std::find_if(files.begin(), files.end(), [&include](const File& f) {return include.path == f.path; });
            size_t index = 0;
            if (it == files.end()) {
                files.push_back(include);
                index = files.size() - 1;
            }
            else
                index = it - files.begin();
            if (std::find(g[i].begin(), g[i].end(), index) == g[i].end())
                g[i].push_back(index);
        }
    }
    // найдем сколько раз каждый файл включен в другие файлы
    vector<pair<string, size_t>> count;
    for (size_t i = 0; i < files.size(); i++) {
        pair<string, size_t> p = make_pair(files[i].name, 0);
        for (const auto& v : g)
            p.second += std::count(v.begin(), v.end(), i);
        count.push_back(p);
    }
    // вывод результат
    sort(count.begin(), count.end(), [](auto& e1, auto& e2) {
        if (e1.second == e2.second)
            return e1.first < e2.first;
        return e1.second > e2.second;
        });
    for (const auto& el : count) {
        if (el.second != 0)
            continue;
        vector<size_t> used_files;
        print_output(el.first, files, g, 0, used_files);
    }
    cout << endl;
    for (const auto& e : count) {
        cout << e.first << " " << e.second << endl;
    }
}

vector<File> FilesAnalizer::build_file_list(const path& directory) {
    //функция для анализа директории и поиска в ней списка h и cpp файлов
    vector<File> result;
    for (auto& p : directory_iterator(directory)) {
        if (p.is_directory()) {
            auto files(this->build_file_list(p.path()));
            result.reserve(result.size() + std::distance(files.begin(), files.end()));
            result.insert(result.end(), files.begin(), files.end());
            continue;
        }
        if (p.path().extension() == ".h" || p.path().extension() == ".cpp")
            result.push_back(File(p.path().filename().string(), p.path().string(), true));
    }
    return result;
}

Includes FilesAnalizer::analize_file(const string& file_path) {
    //будем считывать исходный файл по одному символу и удалим все комментарии из него
    ifstream file(file_path);
    ofstream tmp_file(file_path + "_tmp");
    // Сделаем временный файл в котором не будет закомментированного кода
    char c;
    while (file.get(c)) {
        // пока смог понять что код символа будет <0 если кодировка файла с BOM
        if (c < 0)
            continue;
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
                // если следующим после / оказался обычный символ то откатим буффер чтобы потом прочитать его снова а сам / запишем в файл
                tmp_file << c;
                file.unget();
            }
        }
        else
            tmp_file << c;
    }
    tmp_file.close();
    file.close();
    //Извлечем из файла все директивы #include
    Includes result;
    file.open(file_path + "_tmp");
    char line[1024];
    string line_str;
    while (!file.eof()) {
        file.getline(line, 1024);
        line_str = line;
        trim(line_str);
        if (starts_with(line_str, "#include")) {
            // Поищем символы " или <
            size_t start_file_name_pos = line_str.find_first_of("\"<", 8);
            if (start_file_name_pos != line_str.npos) {
                size_t end_file_name_pos = line_str.find_first_of("\">", start_file_name_pos + 1);
                string included_file = line_str.substr(start_file_name_pos + 1, end_file_name_pos - start_file_name_pos - 1);
                //Определим в какой из списков для поиска положить файл
                if (line_str.find("\"", 8) != line_str.npos)
                    result.local_files.push_back(File(included_file, included_file, false));
                else
                    result.far_files.push_back(File(included_file, included_file, false));
            }
        }
    }
    file.close();
    remove(file_path + "_tmp");
    return result;
}

void FilesAnalizer::find_files(const string& file_path, Includes& includes, const vector<path>& include_directories) {
    // функция ищет все зависимости файла
    path p(file_path);
    path directory(p.parent_path());
    for (auto& file_entry : includes.local_files) {
        path p = directory / file_entry.name;
        file_entry.path = p.string();
        if (exists(p))
            file_entry.exist = true;
    }
    for (auto& file_entry : includes.far_files) {
        for (const path& include_dir : include_directories) {
            path p = include_dir / file_entry.name;
            file_entry.path = p.string();
            if (exists(p)) {
                file_entry.exist = true;
                break;
            }
        }
    }
}

void FilesAnalizer::print_output(const string& file_name, const vector<File>& files, const vector<vector<size_t>>& g, int level, vector<size_t>& used_files) {
    // функция выводит результат в соответствии с заданием
    for (int i = 0; i < level; i++)
        cout << " ";
    cout << file_name;
    auto it = std::find_if(files.begin(), files.end(), [&file_name](const File& f) {return f.name == file_name; });
    size_t index = it - files.begin();
    if (!files[index].exist)
        cout << " (!)";
    cout << endl;
    if (std::find(used_files.begin(), used_files.end(), index) != used_files.end())
        return;
    used_files.push_back(index);
    if (index < g.size())
        for (size_t included_file : g[index])
            print_output(files[included_file].name, files, g, level + 1, used_files);
    used_files.pop_back();
}
