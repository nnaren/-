#include "MultiThreadFileProcessor.h"
#include <iostream>
using namespace std;

// struct Product {
//     Product(string name, ifstream txt) : name(name), txt(std::move(txt)){}
//     Product() { }
//     Product(Product & p) {
//         name = p.name;
//         txt = std::move(p.txt);
//      }
//     void swap(Product& product) {
//         string tmp_name = product.name;
//         ifstream tmp_txt = std::move(product.txt);
//         product.name = name;
//         product.txt = std::move(txt);
//         name = tmp_name;
//         txt = std::move(tmp_txt);
//     }
//     string name;
//     ifstream txt = ifstream();
// };
typedef std::pair<string, ifstream> Product;

int main(int argc, char **argv) {
    cout << "Testing" << endl;
    MultiThread_Controller threadController ;
    vector<fs::path>  paths ;
    fs::path  path1 ("/home/red/code/Cpp/CoroutinesAndAsync/多线程读取文件/resources");
    paths.push_back(path1);
    fs::path  path2 ("/home/red/code/Cpp/CoroutinesAndAsync");
    paths.push_back(path2);
    string dirFilterPattern = "-.git;-.svn";  // 
    string fileFilterPattern = "*.*";
    std::function<bool (Product *, mutex &)> ProductProcessor = [] (Product *p, mutex & m) { 
            {
                unique_lock<mutex> lock(m);
                cout << "ThreadId:" << this_thread::get_id() << endl;
                if (p == nullptr ) {
                    return true;
                }
                // cout << "正在消费: " << p->first  << endl;
                string line;
                line.resize(100);
                while(p->second.getline((char *)&line[0], 20)) {
                    // cout << "line:" << endl;
                    // cout << line << endl;
                }
                return true; 
            }
        };
    MultiThreadFileProcessor<vector<fs::path>, Product>(threadController, paths, dirFilterPattern, fileFilterPattern, ProductProcessor);
    return 0;
}