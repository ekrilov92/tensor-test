#include <boost/program_options.hpp>
#include <iostream>
#include <string>

using namespace boost;
using namespace std;
namespace po = boost::program_options;


int main(int argc, char * argv[])
{
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
            cout << "Directory with source files is not specified";
            return 2;
        }
        string source = vm["source"].as<string>();
        vector<string> include;
        if (vm.count("include")) {
            include = vm["include"].as<vector<string>>();
        }

        for (auto x : include)
            cout << x;
    }
    catch (po::error e) {
        cout << "Command line arguments error" <<endl;
        cout << desc;
    }
}
