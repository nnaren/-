#include <string>
#include<fstream>
#include <iostream>
using namespace std;
ifstream ReadFileFast(string filename) {
    ifstream file(filename);
    // cout << filename << endl;
    return std::move(file);  // 返回的是拷贝？
}