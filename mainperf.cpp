#include "PCFG.h"
#include <chrono>
#include <fstream>
#include "md5.h"
#include <iomanip>
#include <vector>
#include <cassert>
#include <cstdint>
using namespace std;
using namespace chrono;

// g++ -std=c++17 mainperf.cpp md5.cpp -o perf.exe
// g++ -std=c++17 mainperf.cpp md5.cpp -o perf1.exe -O1
// g++ -std=c++17 mainperf.cpp md5.cpp -o perf2.exe -O2

void test_MD5Hash() {
    // 模拟 1000 万猜测
    const size_t TOTAL_GUESSES = 10000000;
    const size_t BATCH_SIZE = 1000000; // 每批 100 万猜测
    double time_hash = 0;

    // 生成测试输入（模拟 RockYou 密码）
    vector<string> guesses(BATCH_SIZE);
    for (size_t i = 0; i < BATCH_SIZE; i++) {
        guesses[i] = "password" + to_string(i % 1000); // 长度 10-20 字节
    }

    // 测试循环，模拟 10 批
    for (size_t batch = 0; batch < TOTAL_GUESSES / BATCH_SIZE; batch++) {
        auto start_hash = system_clock::now();
        bit32 state[4];
        for (const string& pw : guesses) {
            MD5Hash(pw, state);
        }
        auto end_hash = system_clock::now();
        auto duration = duration_cast<microseconds>(end_hash - start_hash);
        time_hash += double(duration.count()) * microseconds::period::num / microseconds::period::den;
    }

    cout << "MD5Hash time: " << time_hash << " seconds" << endl;
}

int main() {
    test_MD5Hash();
    return 0;
}