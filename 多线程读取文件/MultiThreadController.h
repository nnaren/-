#include <thread>
#include <mutex>
#include <condition_variable>

#include <string>
#include <functional>

using namespace std;

// struct SingFileText : std::pair<string, string> {
//     SingleFileText() {}
//     SingleFileText(string n, string t) : pair(std::move(n), std::move(t)) {}

//     string & Name() {return first;}
//     string & Text() {return second;}

//     const string & Name() const {return first;}
//     const string & Text() const {return second;}
// };

struct MultiThread_Controller {
    std::function<void (int, int)>              ConfigProcess = [](int total, int append) {};
    std::function<void (int, const string &)>   UpdateProcess = [](int step, const string & tips) { cout << "*" ; };
    std::function<void (void*)>                 Notify        = [](void* data) {};  // 中间临时输出数据
    std::function<void (const string &)>        Log           = [](const string & log) {}; // 出现错误
    std::function<void (int, int)>              Finish        = [](int readedCount, int processedCount) {cout << "读取文件个数: "<< readedCount << ", 处理文件个数: " << processedCount << endl;};  //读取的文件数，处理的文件数

    std::function<bool ()>                      IsUserStopped = []() { return false; };

    int                                         consumerCount = thread::hardware_concurrency();
    size_t                                      maxWaitNum    = 100;
    
    bool                                        blockCaller   = true;
};