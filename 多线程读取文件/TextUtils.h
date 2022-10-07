
#include <string>
using namespace std;

void trim(::std::string & str) {
    //left
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    //right
    str.erase(std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), str.end());
}

// 删除注释 ？
string TrimAndEraseComment(string  str) { 
    trim(str);
    return str;
}