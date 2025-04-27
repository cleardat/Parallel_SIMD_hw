#include "PCFG.h"
#include "md5.h"
#include "md5_SIMD.h"
#include <iostream>
#include <iomanip>
#include <string>

// g++ correctness_SIMD.cpp train.cpp guessing.cpp md5.cpp md5_SIMD.cpp -o testc.exe

// 定义 NUM_MESSAGES，确保与 MD5Hash_SIMD 的实现一致
#define NUM_MESSAGES 4

using namespace std;

// 验证 MD5Hash_SIMD 的正确性：对 4 个字符串分别调用 MD5Hash 和 MD5Hash_SIMD，比较哈希结果
int main() {
    // 定义 4 个不同的测试字符串
    // string inputs[NUM_MESSAGES] = {
    //     "bvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdva",
    //     "bvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdva",
    //     "bvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdva",
    //     "bvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdva",
    // };

    string inputs[NUM_MESSAGES] = {
        "a",
        "b",
        "c",
        "d"
    };

    // 串行哈希：对每个字符串调用 MD5Hash
    bit32 serial_states[NUM_MESSAGES][4];
    for (int i = 0; i < NUM_MESSAGES; i++) {
        MD5Hash(inputs[i], serial_states[i]);
    }

    // 并行哈希：对 4 个字符串调用 MD5Hash_SIMD
    bit32 parallel_states[NUM_MESSAGES][4];
    MD5Hash_SIMD(inputs, parallel_states);

    // 比较串行和并行哈希结果
    bool is_correct = true;
    for (int i = 0; i < NUM_MESSAGES; i++) {
        for (int j = 0; j < 4; j++) {
            if (serial_states[i][j] != parallel_states[i][j]) {
                is_correct = false;
                break;
            }
        }
        if (!is_correct) break;
    }

    // 输出哈希结果和验证结论
    cout << "串行哈希结果（MD5Hash）：" << endl;
    for (int i = 0; i < NUM_MESSAGES; i++) {
        cout << "字符串 " << i << " (" << inputs[i].substr(0, 20) << "...): ";
        for (int j = 0; j < 4; j++) {
            cout << setw(8) << setfill('0') << hex << serial_states[i][j];
        }
        cout << endl;
    }

    cout << "\n并行哈希结果（MD5Hash_SIMD）：" << endl;
    for (int i = 0; i < NUM_MESSAGES; i++) {
        cout << "字符串 " << i << " (" << inputs[i].substr(0, 20) << "...): ";
        for (int j = 0; j < 4; j++) {
            cout << setw(8) << setfill('0') << hex << parallel_states[i][j];
        }
        cout << endl;
    }

    cout << "\n验证结果：";
    if (is_correct) {
        cout << "MD5Hash_SIMD 正确，所有 4 个字符串的哈希值一致。" << endl;
    } else {
        cout << "MD5Hash_SIMD 错误，存在哈希值不一致。" << endl;
    }

    return 0;
}