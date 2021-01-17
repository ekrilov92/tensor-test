#ifndef FILESANALIZER_H
#define FILESANALIZER_H

#include <string>
#include <vector>
#include <filesystem>

using namespace std;
using namespace filesystem;


class FilesAnalizer
{
    //������ ��� ������� ������
public:
    void analize_files(const string& source_directory, const vector<path>& include_directories);
private:
    //��������� ��� �������� ���������� �� ����� �����
    struct File {
        string name;
        string path;
        bool exist;

        File(const string& name_, const string& path_, bool exist_);
    };

    //��������� �������������� ����������� ������ �����
    struct Includes {
        //������ ������, ������� ����� ������ ��������, ������������ �����, ������ ���� pair �������� ������ ���� ��� ���
        vector<File> local_files;
        //������ ������, ������� ����� ������ � �������������� ����������� ��� ���������� ������, ������ ���� pair �������� ������ ���� ��� ���
        vector<File> far_files;

        friend ostream& operator<<(ostream& os, const Includes& data) {
            //��� �������
            os << "local_files:" << endl;
            for (const auto& x : data.local_files)
                os << x.name << " " << x.exist << endl;
            os << "far_files:" << endl;
            for (const auto& x : data.far_files)
                os << x.name << " " << x.exist << endl;
            return os;
        }
    };

    vector<File> build_file_list(const path& directory);
    Includes analize_file(const string& file_path);
    void find_files(const string& file_path, Includes& includes, const vector<path>& include_directories);
    void print_output(const string& file_name, const vector<File>& files, const vector<vector<size_t>>& g, int level, vector<size_t>& used_files);
};


#endif // !FILESANALIZER_H

