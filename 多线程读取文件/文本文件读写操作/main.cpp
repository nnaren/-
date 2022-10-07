#include<iostream>
#include<vector>
#include<string>
#include<fstream>

#include <filesystem>
#include <boost/filesystem.hpp>

namespace fs = std::filesystem;
namespace boostfs = boost::filesystem;

using namespace std;

void TestFileSize(const string & pathName) {
    cout << "TestFileSize" << endl;
    cout << boostfs::path(pathName).string() << " : " << boostfs::file_size(pathName) << endl;
}

int main() {
    string pathName("/home/red/code/Cpp/CoroutinesAndAsync/多线程读取文件/resources/hello.cpp");
    int n = boostfs::file_size(pathName);
    ifstream file("/home/red/code/Cpp/CoroutinesAndAsync/多线程读取文件/resources/hello.cpp");
    if(!file) {
        cout << "Error:" << std::endl;
        return -1;
    }

    // 1 原始char[] 
    char* line1 = new char[20];


    string line;
    line.resize(n);
    while(file.read((char *)&line[0], n)) {
        cout << "line:" << endl;
        cout << line << endl;
    }
    file.close();
    return 0;
}