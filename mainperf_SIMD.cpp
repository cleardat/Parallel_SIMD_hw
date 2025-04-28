#include "PCFG.h"
#include <chrono>
#include <fstream>
#include "md5_SIMD.h"
#include <iomanip>
#include <vector>
#include <cassert>
#include <cstdint>
using namespace std;
using namespace chrono;
const int NUM_MESSAGES = 4;

// g++ -std=c++17 mainperf_SIMD.cpp md5_SIMD.cpp -o perf_SIMD.exe
// g++ -std=c++17 mainperf_SIMD.cpp md5_SIMD.cpp -o perf_SIMD1.exe -O1
// g++ -std=c++17 mainperf_SIMD.cpp md5_SIMD.cpp -o perf_SIMD2.exe -O2

void test_MD5Hash_SIMD() {
    // 模拟 1000 万猜测
    const size_t TOTAL_GUESSES = 10000000;
    const size_t BATCH_SIZE = 1000000; // 每批 100 万猜测
    double time_hash = 0;

    // 生成测试输入（模拟 RockYou 密码）
    vector<string> guesses(BATCH_SIZE);
    for (size_t i = 0; i < BATCH_SIZE; i++) {
        guesses[i] = "password" + to_string(i % 1000); // 模拟 10-20 字节密码
    }

    // 测试循环，模拟 10 批
    for (size_t batch = 0; batch < TOTAL_GUESSES / BATCH_SIZE; batch++) {
        auto start_hash = system_clock::now();
        for (size_t i = 0; i < guesses.size(); i += NUM_MESSAGES) {
            string pw[NUM_MESSAGES];
            alignas(16) bit32 state[NUM_MESSAGES][4];
            assert(((uintptr_t)state % 16) == 0 && "state is not 16-byte aligned");

            // 填充输入
            for (int j = 0; j < NUM_MESSAGES; j++) {
                if (i + j < guesses.size()) {
                    pw[j] = guesses[i + j];
                } else {
                    pw[j] = "";
                }
            }

            // 调用哈希函数
            MD5Hash_SIMD(pw, state);
        }
        auto end_hash = system_clock::now();
        auto duration = duration_cast<microseconds>(end_hash - start_hash);
        time_hash += double(duration.count()) * microseconds::period::num / microseconds::period::den;
    }

    cout << "MD5Hash_SIMD time: " << time_hash << " seconds" << endl;
}

int main() {
    test_MD5Hash_SIMD();
    return 0;
}