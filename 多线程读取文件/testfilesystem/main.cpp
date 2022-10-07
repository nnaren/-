#include <iostream>
#include <filesystem>
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = std::filesystem;
namespace boostfs = boost::filesystem;

void TestFileSize(const string & pathName) {
    cout << "TestFileSize" << endl;
    cout << boostfs::path(pathName).string() << " : " << boostfs::file_size(pathName) << endl;
    wcout << fs::path(pathName).wstring() << " : " << fs::file_size(pathName) << endl;  // 中文的问�?
    cout << fs::path(pathName).string() << " : " << fs::file_size(pathName) << endl;
}

void TestIterator(const string & pathName) {
    cout << "TestIterator" << endl;
    try {
        for(fs::directory_iterator dir_itr(pathName); dir_itr != fs::directory_iterator(); ++dir_itr) {
            if ( fs::is_directory(dir_itr->status())) {
                cout << "subdir : " << dir_itr->path().string() << endl;

            } else if (fs::is_regular_file(dir_itr->status())) {
                cout << "file  : " << dir_itr->path().string() << endl;
            }

        }
    } catch (...) {

    }
}

void TestPath(const string& pathName) {
    cout << "TestPath" << endl;
    boostfs::path tmp_dir(pathName);
    cout << tmp_dir.string() << endl;
    auto p = tmp_dir / "new.txt";
    cout << p.string() << " : " << boostfs::file_size(p) << endl;

    cout << p.parent_path() << " - " << p.filename() << " - "
         << p.stem() << " - " << p.extension() << endl;
}

int main(int argc, char *argv[]) {
    TestPath("/home/red/code/Cpp/CoroutinesAndAsync/多线程读取文件/testfilesystem/tmp");
    TestIterator("/home/red/code/Cpp/CoroutinesAndAsync/多线程读取文件/testfilesystem/tmp");
    TestFileSize("./new.txt");
    TestFileSize("/home/red/code/Cpp/CoroutinesAndAsync/多线程读取文件/testfilesystem/tmp/new.txt");
    return 0;
}
    
