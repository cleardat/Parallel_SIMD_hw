#include "md5_SIMD3.h"
#include <iomanip>
#include <assert.h>
#include <chrono>
#include <arm_neon.h>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <cstdint>
using namespace std;
using namespace chrono;
const int NUM_MESSAGES = 4;

Byte *StringProcess_P(string input, int *n_byte) {
    Byte *blocks = (Byte *)input.c_str();
    int length = input.length();
    int bitLength = length * 8;

    int paddingBits = bitLength % 512;
    if (paddingBits > 448) {
        paddingBits = 512 - (paddingBits - 448);
    } else if (paddingBits < 448) {
        paddingBits = 448 - paddingBits;
    } else if (paddingBits == 448) {
        paddingBits = 512;
    }

    int paddingBytes = paddingBits / 8;
    int paddedLength = length + paddingBytes + 8;
    Byte *paddedMessage = (Byte *)std::aligned_alloc(16, paddedLength);
    //assert(((uintptr_t)paddedMessage % 16) == 0 && "paddedMessage is not 16-byte aligned");

    memcpy(paddedMessage, blocks, length);
    paddedMessage[length] = 0x80;
    memset(paddedMessage + length + 1, 0, paddingBytes - 1);

    for (int i = 0; i < 8; ++i) {
        paddedMessage[length + paddingBytes + i] = ((uint64_t)length * 8 >> (i * 8)) & 0xFF;
    }

    *n_byte = paddedLength;
    return paddedMessage;
}

void MD5Hash_SIMD(string inputs[], bit32 states[][4]) {
    Byte *paddedMessages[NUM_MESSAGES];
    int messageLengths[NUM_MESSAGES];
    int max_blocks = 0;

    // 填充每个消息
    for (int i = 0; i < NUM_MESSAGES; i++) {
        paddedMessages[i] = StringProcess_P(inputs[i], &messageLengths[i]);
        max_blocks = max(max_blocks, messageLengths[i] / 64);
    }

    // 初始化状态
    uint32x4_t state[4];
    state[0] = vdupq_n_u32(0x67452301);
    state[1] = vdupq_n_u32(0xefcdab89);
    state[2] = vdupq_n_u32(0x98badcfe);
    state[3] = vdupq_n_u32(0x10325476);

    // 分块处理
    for (int block = 0; block < max_blocks; block++) {
        uint32x4_t x[16];
        // 创建掩码，标记有效块的 lane
        alignas(16) uint32_t mask_values[NUM_MESSAGES];
        for (int i = 0; i < NUM_MESSAGES; i++) {
            mask_values[i] = (block < messageLengths[i] / 64) ? 0xFFFFFFFF : 0;
        }
        uint32x4_t mask = vld1q_u32(mask_values);

        // 加载当前块的数据
        for (int i1 = 0; i1 < 16; i1++) {
            alignas(16) uint32_t values[NUM_MESSAGES];
            for (int i = 0; i < NUM_MESSAGES; i++) {
                if (block < messageLengths[i] / 64) {
                    values[i] = (paddedMessages[i][4 * i1 + block * 64]) |
                                (paddedMessages[i][4 * i1 + 1 + block * 64] << 8) |
                                (paddedMessages[i][4 * i1 + 2 + block * 64] << 16) |
                                (paddedMessages[i][4 * i1 + 3 + block * 64] << 24);
                } else {
                    values[i] = 0; // 临时置零，掩码将防止其影响状态
                }
            }
            x[i1] = vld1q_u32(values);
        }

        // 保存当前状态
        uint32x4_t prev_state[4] = { state[0], state[1], state[2], state[3] };
        uint32x4_t a = state[0], b = state[1], c = state[2], d = state[3];

        // 第一轮
        FF_SIMD(a, b, c, d, x[0], s11, 0xd76aa478);
        FF_SIMD(d, a, b, c, x[1], s12, 0xe8c7b756);
        FF_SIMD(c, d, a, b, x[2], s13, 0x242070db);
        FF_SIMD(b, c, d, a, x[3], s14, 0xc1bdceee);
        FF_SIMD(a, b, c, d, x[4], s11, 0xf57c0faf);
        FF_SIMD(d, a, b, c, x[5], s12, 0x4787c62a);
        FF_SIMD(c, d, a, b, x[6], s13, 0xa8304613);
        FF_SIMD(b, c, d, a, x[7], s14, 0xfd469501);
        FF_SIMD(a, b, c, d, x[8], s11, 0x698098d8);
        FF_SIMD(d, a, b, c, x[9], s12, 0x8b44f7af);
        FF_SIMD(c, d, a, b, x[10], s13, 0xffff5bb1);
        FF_SIMD(b, c, d, a, x[11], s14, 0x895cd7be);
        FF_SIMD(a, b, c, d, x[12], s11, 0x6b901122);
        FF_SIMD(d, a, b, c, x[13], s12, 0xfd987193);
        FF_SIMD(c, d, a, b, x[14], s13, 0xa679438e);
        FF_SIMD(b, c, d, a, x[15], s14, 0x49b40821);

        // 第二轮
        GG_SIMD(a, b, c, d, x[1], s21, 0xf61e2562);
        GG_SIMD(d, a, b, c, x[6], s22, 0xc040b340);
        GG_SIMD(c, d, a, b, x[11], s23, 0x265e5a51);
        GG_SIMD(b, c, d, a, x[0], s24, 0xe9b6c7aa);
        GG_SIMD(a, b, c, d, x[5], s21, 0xd62f105d);
        GG_SIMD(d, a, b, c, x[10], s22, 0x02441453);
        GG_SIMD(c, d, a, b, x[15], s23, 0xd8a1e681);
        GG_SIMD(b, c, d, a, x[4], s24, 0xe7d3fbc8);
        GG_SIMD(a, b, c, d, x[9], s21, 0x21e1cde6);
        GG_SIMD(d, a, b, c, x[14], s22, 0xc33707d6);
        GG_SIMD(c, d, a, b, x[3], s23, 0xf4d50d87);
        GG_SIMD(b, c, d, a, x[8], s24, 0x455a14ed);
        GG_SIMD(a, b, c, d, x[13], s21, 0xa9e3e905);
        GG_SIMD(d, a, b, c, x[2], s22, 0xfcefa3f8);
        GG_SIMD(c, d, a, b, x[7], s23, 0x676f02d9);
        GG_SIMD(b, c, d, a, x[12], s24, 0x8d2a4c8a);

        // 第三轮
        HH_SIMD(a, b, c, d, x[5], s31, 0xfffa3942);
        HH_SIMD(d, a, b, c, x[8], s32, 0x8771f681);
        HH_SIMD(c, d, a, b, x[11], s33, 0x6d9d6122);
        HH_SIMD(b, c, d, a, x[14], s34, 0xfde5380c);
        HH_SIMD(a, b, c, d, x[1], s31, 0xa4beea44);
        HH_SIMD(d, a, b, c, x[4], s32, 0x4bdecfa9);
        HH_SIMD(c, d, a, b, x[7], s33, 0xf6bb4b60);
        HH_SIMD(b, c, d, a, x[10], s34, 0xbebfbc70);
        HH_SIMD(a, b, c, d, x[13], s31, 0x289b7ec6);
        HH_SIMD(d, a, b, c, x[0], s32, 0xeaa127fa);
        HH_SIMD(c, d, a, b, x[3], s33, 0xd4ef3085);
        HH_SIMD(b, c, d, a, x[6], s34, 0x04881d05);
        HH_SIMD(a, b, c, d, x[9], s31, 0xd9d4d039);
        HH_SIMD(d, a, b, c, x[12], s32, 0xe6db99e5);
        HH_SIMD(c, d, a, b, x[15], s33, 0x1fa27cf8);
        HH_SIMD(b, c, d, a, x[2], s34, 0xc4ac5665);

        // 第四轮
        II_SIMD(a, b, c, d, x[0], s41, 0xf4292244);
        II_SIMD(d, a, b, c, x[7], s42, 0x432aff97);
        II_SIMD(c, d, a, b, x[14], s43, 0xab9423a7);
        II_SIMD(b, c, d, a, x[5], s44, 0xfc93a039);
        II_SIMD(a, b, c, d, x[12], s41, 0x655b59c3);
        II_SIMD(d, a, b, c, x[3], s42, 0x8f0ccc92);
        II_SIMD(c, d, a, b, x[10], s43, 0xffeff47d);
        II_SIMD(b, c, d, a, x[1], s44, 0x85845dd1);
        II_SIMD(a, b, c, d, x[8], s41, 0x6fa87e4f);
        II_SIMD(d, a, b, c, x[15], s42, 0xfe2ce6e0);
        II_SIMD(c, d, a, b, x[6], s43, 0xa3014314);
        II_SIMD(b, c, d, a, x[13], s44, 0x4e0811a1);
        II_SIMD(a, b, c, d, x[4], s41, 0xf7537e82);
        II_SIMD(d, a, b, c, x[11], s42, 0xbd3af235);
        II_SIMD(c, d, a, b, x[2], s43, 0x2ad7d2bb);
        II_SIMD(b, c, d, a, x[9], s44, 0xeb86d391);

        // 选择性更新状态
        state[0] = vbslq_u32(mask, vaddq_u32(prev_state[0], a), prev_state[0]);
        state[1] = vbslq_u32(mask, vaddq_u32(prev_state[1], b), prev_state[1]);
        state[2] = vbslq_u32(mask, vaddq_u32(prev_state[2], c), prev_state[2]);
        state[3] = vbslq_u32(mask, vaddq_u32(prev_state[3], d), prev_state[3]);
    }

    // 字节序调整并存储结果
    for (int i = 0; i < 4; i++) {
        alignas(16) uint32_t values[NUM_MESSAGES];
        vst1q_u32(values, state[i]);
        for (int j = 0; j < NUM_MESSAGES; j++) {
            uint32_t value = values[j];
            states[j][i] = ((value & 0xff) << 24) |
                           ((value & 0xff00) << 8) |
                           ((value & 0xff0000) >> 8) |
                           ((value & 0xff000000) >> 24);
        }
    }

    // 清理内存
    for (int i = 0; i < NUM_MESSAGES; i++) {
        delete[] paddedMessages[i];
    }
}