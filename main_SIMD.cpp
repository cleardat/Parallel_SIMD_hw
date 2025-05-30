#include "PCFG.h"
#include <chrono>
#include <fstream>
#include "md5_SIMD3.h"
#include <iomanip>
using namespace std;
using namespace chrono;
const int NUM_MESSAGES = 4;

// g++ -std=c++17  main_SIMD.cpp train.cpp guessing.cpp md5_SIMD3.cpp -o test3_SIMD.exe 
// g++ -std=c++17  main_SIMD.cpp train.cpp guessing.cpp md5_SIMD3.cpp -o test3_SIMD1.exe -O1
// g++ -std=c++17  main_SIMD.cpp train.cpp guessing.cpp md5_SIMD3.cpp -o test3_SIMD2.exe -O2

int main() {
    double time_hash = 0;  // 用于MD5哈希的时间
    double time_guess = 0; // 哈希和猜测的总时长
    double time_train = 0; // 模型训练的总时长
    PriorityQueue q;
    auto start_train = system_clock::now();
    q.m.train("/guessdata/Rockyou-singleLined-full.txt");
    q.m.order();
    auto end_train = system_clock::now();
    auto duration_train = duration_cast<microseconds>(end_train - start_train);
    time_train = double(duration_train.count()) * microseconds::period::num / microseconds::period::den;

    q.init();
    cout << "here" << endl;
    int curr_num = 0;
    auto start = system_clock::now();
    // 由于需要定期清空内存，我们在这里记录已生成的猜测总数
    int history = 0;
    while (!q.priority.empty()) {
        q.PopNext();
        q.total_guesses = q.guesses.size();
        if (q.total_guesses - curr_num >= 100000) {
            cout << "Guesses generated: " << history + q.total_guesses << endl;
            curr_num = q.total_guesses;

            // 在此处更改实验生成的猜测上限
            int generate_n = 10000000;
            if (history + q.total_guesses > 10000000) {
                auto end = system_clock::now();
                auto duration = duration_cast<microseconds>(end - start);
                time_guess = double(duration.count()) * microseconds::period::num / microseconds::period::den;
                cout << "Guess time:" << time_guess - time_hash << "seconds" << endl;
                cout << "Hash time:" << time_hash << "seconds" << endl;
                cout << "Train time:" << time_train << "seconds" << endl;
                break;
            }
        }
        // 为了避免内存超限，我们在q.guesses中口令达到一定数目时，将其中的所有口令取出并且进行哈希
        // 然后，q.guesses将会被清空。为了有效记录已经生成的口令总数，维护一个history变量来进行记录
        if (curr_num > 1000000) {
            auto start_hash = system_clock::now();
            for (size_t i = 0; i < q.guesses.size(); i += NUM_MESSAGES) {
                string pw[NUM_MESSAGES];
                alignas(16) bit32 state[NUM_MESSAGES][4]; // 确保 state 16 字节对齐
                //assert(((uintptr_t)state % 16) == 0 && "state is not 16-byte aligned");

                // 填充 pw 数组
                for (int j = 0; j < NUM_MESSAGES; j++) {
                    if (i + j < q.guesses.size()) {
                        pw[j] = q.guesses[i + j];
                    } else {
                        pw[j] = ""; // 填充空字符串
                    }
                }

                // 调用 SIMD 哈希函数
                MD5Hash_SIMD(pw, state);

                // 可选：输出或验证哈希结果
                /*
                for (int j = 0; j < NUM_MESSAGES && i + j < q.guesses.size(); j++) {
                    cout << "Guess: " << pw[j] << "\tHash: ";
                    for (int k = 0; k < 4; k++) {
                        cout << hex << setw(8) << setfill('0') << state[j][k];
                    }
                    cout << endl;
                }
                */
            }
            // 在这里对哈希所需的总时长进行计算
            auto end_hash = system_clock::now();
            auto duration = duration_cast<microseconds>(end_hash - start_hash);
            time_hash += double(duration.count()) * microseconds::period::num / microseconds::period::den;
            // 记录已经生成的口令总数
            history += curr_num;
            curr_num = 0;
            q.guesses.clear();
        }
    }
}