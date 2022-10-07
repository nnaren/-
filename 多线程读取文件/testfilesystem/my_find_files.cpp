#include <iostream>
#include <filesystem>
#include <boost/filesystem.hpp>
#include <regex>
using namespace std;
namespace fs = std::filesystem;
namespace boostfs = boost::filesystem;



void TestIterator(const string & pathName) {
    cout << "pathName" << endl;
    try {
        for(fs::directory_iterator dir_itr(pathName); dir_itr != fs::directory_iterator(); ++dir_itr) {
            if ( fs::is_directory(dir_itr->status())) {
                cout << "subdir : " << dir_itr->path().string() << endl;
                TestIterator(dir_itr->path().string());  // 递归

            } else if (fs::is_regular_file(dir_itr->status())) {
                cout << "file  : " << dir_itr->path().string() << endl;
                cout << "match regex " << dir_itr->path().filename() << endl;
                regex pt(".*\.cpp");
                string fileName = dir_itr->path().filename().string();
                if(regex_match(fileName, pt)) {
                    cout << "yes" << endl;
                }
            }

        }
    } catch (...) {

    }
}
// 优化点： 相对路径
int main(int argc, char *argv[]) {
    TestIterator("/home/red/code/Cpp/CoroutinesAndAsync/多线程读取文件/resources");
}