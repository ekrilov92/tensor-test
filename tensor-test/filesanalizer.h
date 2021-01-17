#ifndef FILESANALIZER_H
#define FILESANALIZER_H

#include <string>
#include <vector>
#include <filesystem>

using namespace std;
using namespace filesystem;

//структура дл€ хранени€ информации об одном файле
struct File {
    string name;
    string path;
    bool exist;

    File(const string& name_, const string& path_, bool exist_) : name(name_), path(path_), exist(exist_) {}
};

//структура представлю€ща€ зависимости одного файла
struct Includes {
    //список файлов, которые нужно искать локально, относительно файла, второй член pair означает найден файл или нет
    vector<File> local_files;
    //список файлов, которые нужно искать в дополнительных директори€х дл€ включаемых файлов, второй член pair означает найден файл или нет
    vector<File> far_files;

    friend ostream& operator<<(ostream& os, const Includes& data) {
        //дл€ отладки
        os << "local_files:" << endl;
        for (const auto& x : data.local_files)
            os << x.name << " " << x.exist << endl;
        os << "far_files:" << endl;
        for (const auto& x : data.far_files)
            os << x.name << " " << x.exist << endl;
        return os;
    }
};


class FilesAnalizer
{
    // ласса дл€ анализа файлов
public:
    void analize_files(const string& source_directory, const vector<path>& include_directories);
private:
    vector<File> build_file_list(const path& directory);
    Includes analize_file(const string& file_path);
    void find_files(const string& file_path, Includes& includes, const vector<path>& include_directories);
    void print_output(const string& file_name, const vector<File>& files, const vector<vector<size_t>>& g, int level, vector<size_t>& used_files);
};


#endif // !FILESANALIZER_H

