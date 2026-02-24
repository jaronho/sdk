#include "uuid.h"

#include <chrono>
#include <memory>
#include <random>
#include <thread>

namespace algorithm
{
/* 线程局部快速随机数生成器 (xoshiro256**) */
static thread_local uint64_t s_rng_state[4] = {0};

static uint64_t rol64(uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

static void initRng()
{
    std::random_device rd;
    s_rng_state[0] = rd();
    s_rng_state[1] = rd();
    s_rng_state[2] = rd();
    s_rng_state[3] = rd();
}

static uint64_t xoshiro256ss()
{
    uint64_t* s = s_rng_state;
    uint64_t result = rol64(s[1] * 5, 7) * 9;
    uint64_t t = s[1] << 17;
    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];
    s[2] ^= t;
    s[3] = rol64(s[3], 45);
    return result;
}

static uint64_t fastRandom()
{
    if (0 == s_rng_state[0])
    {
        initRng();
    }
    return xoshiro256ss();
}

std::array<uint8_t, 16> UUID::generateUUIDv7()
{
    std::array<uint8_t, 16> uuid{};
    /* 获取当前时间戳 */
    auto ntp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    uint64_t timestamp = ntp.time_since_epoch().count();
    /* 前48位(6字节): 大端序时间戳 */
    uuid[0] = (timestamp >> 40) & 0xFF;
    uuid[1] = (timestamp >> 32) & 0xFF;
    uuid[2] = (timestamp >> 24) & 0xFF;
    uuid[3] = (timestamp >> 16) & 0xFF;
    uuid[4] = (timestamp >> 8) & 0xFF;
    uuid[5] = timestamp & 0xFF;
    /* 获取两个64位随机数 */
    uint64_t rand1 = fastRandom();
    uint64_t rand2 = fastRandom();
    /* 第6字节: 版本7(0111) + 高4位随机 */
    uuid[6] = 0x70 | ((rand1 >> 60) & 0x0F);
    /* 第7字节: 8位随机 */
    uuid[7] = (rand1 >> 52) & 0xFF;
    /* 第8字节: 变体10(10xxxxxx) + 高2位随机 */
    uuid[8] = 0x80 | ((rand1 >> 46) & 0x3F);
    /* 剩余随机位 */
    uuid[9] = (rand1 >> 38) & 0xFF;
    uuid[10] = (rand1 >> 30) & 0xFF;
    uuid[11] = (rand1 >> 22) & 0xFF;
    uuid[12] = (rand1 >> 14) & 0xFF;
    uuid[13] = (rand1 >> 6) & 0xFF;
    uuid[14] = ((rand1 & 0x3F) << 2) | ((rand2 >> 62) & 0x03);
    uuid[15] = (rand2 >> 54) & 0xFF;
    return uuid;
}

std::string UUID::generateUUIDv7String()
{
    return uuidToString(generateUUIDv7());
}

void UUID::uuidToString(const std::array<uint8_t, 16>& uuidBytes, char buffer[37])
{
    if (buffer)
    {
        static const char HEX_DIGITS[] = "0123456789abcdef";
        int src = 0, dst = 0;
        /* 8-4-4-4-12格式, 例如: 018e1234-5678-7abc-8def-0123456789ab */
        int dashPos[] = {4, 6, 8, 10};
        int dashIndex = 0;
        for (int i = 0; i < 16; ++i)
        {
            /* 在指定位置添加横线 */
            if (dashIndex < 4 && i == dashPos[dashIndex])
            {
                buffer[dst++] = '-';
                ++dashIndex;
            }
            uint8_t byte = uuidBytes[src++];
            buffer[dst++] = HEX_DIGITS[byte >> 4];
            buffer[dst++] = HEX_DIGITS[byte & 0x0F];
        }
        buffer[dst] = '\0';
    }
}

std::string UUID::uuidToString(const std::array<uint8_t, 16>& uuidBytes)
{
    char buffer[37];
    uuidToString(uuidBytes, buffer);
    return std::string(buffer);
}
} // namespace algorithm
