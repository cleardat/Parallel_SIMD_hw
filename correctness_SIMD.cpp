#include "PCFG.h"
#include <chrono>
#include <fstream>
#include "md5_SIMD.h"
#include <iomanip>
using namespace std;
using namespace chrono;

// 编译指令如下：
// g++ correctness_SIMD.cpp train.cpp guessing.cpp md5_SIMD.cpp -o testc_SIMD.exe


// 通过这个函数，你可以验证你实现的SIMD哈希函数的正确性
int main()
{
    uint32x4_t* state;  // 改为 uint32x4_t 类型
    std::string input = "bvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdvabvaisdbjasdkafkasdfnavkjnakdjfejfanjsdnfkajdfkajdfjkwanfdjaknsvjkanbjbjadfajwefajksdfakdnsvjadfasjdva";
    MD5Hash_SIMD(&input, state);
    for (int i1 = 0; i1 < 4; i1 += 1)
    {
        cout << std::setw(8) << std::setfill('0') << hex << state[i1];
    }
    cout << endl;
}