#include <iostream>
#include <string>
#include <cstring>
#include <arm_neon.h> // NEON 指令
#include <iomanip>
#include <assert.h>
#include <chrono>
#include <algorithm>
using namespace std;
using namespace chrono;
// 定义了Byte，便于使用
typedef unsigned char Byte;
// 定义了32比特
typedef unsigned int bit32;

// MD5的一系列参数。参数是固定的，其实你不需要看懂这些
#define s11 7
#define s12 12
#define s13 17
#define s14 22
#define s21 5
#define s22 9
#define s23 14
#define s24 20
#define s31 4
#define s32 11
#define s33 16
#define s34 23
#define s41 6
#define s42 10
#define s43 15
#define s44 21


// 内联的F_SIMD函数
inline uint32x4_t F_SIMD(const uint32x4_t& x, const uint32x4_t& y, const uint32x4_t& z) {
    return vorrq_u32(vandq_u32(x, y), vandq_u32(vmvnq_u32(x), z));
}

// 内联的G_SIMD函数
inline uint32x4_t G_SIMD(const uint32x4_t& x, const uint32x4_t& y, const uint32x4_t& z) {
    return vorrq_u32(vandq_u32(x, z), vandq_u32(y, vmvnq_u32(z)));
}

// 内联的H_SIMD函数
inline uint32x4_t H_SIMD(const uint32x4_t& x, const uint32x4_t& y, const uint32x4_t& z) {
    return veorq_u32(veorq_u32(x, y), z);
}

// 内联的I_SIMD函数
inline uint32x4_t I_SIMD(const uint32x4_t& x, const uint32x4_t& y, const uint32x4_t& z) {
    return veorq_u32(y, vorrq_u32(x, vmvnq_u32(z)));
}

// 内联的ROTATELEFT_SIMD函数
inline uint32x4_t ROTATELEFT_SIMD(const uint32x4_t& x, const int n) {
    return vorrq_u32(vshlq_n_u32(x, n), vshrq_n_u32(x, 32 - n));
}

// 内联的FF_SIMD函数
inline void FF_SIMD(uint32x4_t& a, const uint32x4_t& b, const uint32x4_t& c, const uint32x4_t& d, 
                     const uint32x4_t& x, const int s, const uint32_t ac) {
    uint32x4_t temp = F_SIMD(b, c, d);
    temp = vaddq_u32(temp, x);
    temp = vaddq_u32(temp, vdupq_n_u32(ac));
    a = vaddq_u32(a, temp);
    a = ROTATELEFT_SIMD(a, s);
    a = vaddq_u32(a, b);
}

// 内联的GG_SIMD函数
inline void GG_SIMD(uint32x4_t& a, const uint32x4_t& b, const uint32x4_t& c, const uint32x4_t& d, 
                     const uint32x4_t& x, const int s, const uint32_t ac) {
    uint32x4_t temp = G_SIMD(b, c, d);
    temp = vaddq_u32(temp, x);
    temp = vaddq_u32(temp, vdupq_n_u32(ac));
    a = vaddq_u32(a, temp);
    a = ROTATELEFT_SIMD(a, s);
    a = vaddq_u32(a, b);
}

// 内联的HH_SIMD函数
inline void HH_SIMD(uint32x4_t& a, const uint32x4_t& b, const uint32x4_t& c, const uint32x4_t& d, 
                     const uint32x4_t& x, const int s, const uint32_t ac) {
    uint32x4_t temp = H_SIMD(b, c, d);
    temp = vaddq_u32(temp, x);
    temp = vaddq_u32(temp, vdupq_n_u32(ac));
    a = vaddq_u32(a, temp);
    a = ROTATELEFT_SIMD(a, s);
    a = vaddq_u32(a, b);
}

// 内联的II_SIMD函数
inline void II_SIMD(uint32x4_t& a, const uint32x4_t& b, const uint32x4_t& c, const uint32x4_t& d, 
                     const uint32x4_t& x, const int s, const uint32_t ac) {
    uint32x4_t temp = I_SIMD(b, c, d);
    temp = vaddq_u32(temp, x);
    temp = vaddq_u32(temp, vdupq_n_u32(ac));
    a = vaddq_u32(a, temp);
    a = ROTATELEFT_SIMD(a, s);
    a = vaddq_u32(a, b);
}

void MD5Hash_SIMD(string inputs[], bit32 states[][4]);