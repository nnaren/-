#include "MyGenerator_readonly.h"
#include<iostream>

using namespace std;
MyGenerator<int> Gen_StdCoroutine(int x) {
    for (int i = 0; i < x ; i++) {
        co_yield i;
    }
}


int main() {
    auto gen = Gen_StdCoroutine(5);
    for(auto x : gen) {
        cout << x << endl;
    }
}