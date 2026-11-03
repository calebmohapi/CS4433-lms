#include "util.h"
#include <vector>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>

namespace {
inline uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

void sha256_transform(uint32_t state[8], const uint8_t block[64]) {
    uint32_t w[64];
    for (int i = 0; i < 16; i++)
        w[i] = (uint32_t(block[i*4]) << 24) | (uint32_t(block[i*4+1]) << 16)
             | (uint32_t(block[i*4+2]) << 8) | uint32_t(block[i*4+3]);
    for (int i = 16; i < 64; i++) {
        uint32_t s0 = rotr(w[i-15],7) ^ rotr(w[i-15],18) ^ (w[i-15] >> 3);
        uint32_t s1 = rotr(w[i-2],17) ^ rotr(w[i-2],19) ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];
    for (int i = 0; i < 64; i++) {
        uint32_t S1 = rotr(e,6) ^ rotr(e,11) ^ rotr(e,25);
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t t1 = h + S1 + ch + K[i] + w[i];
        uint32_t S0 = rotr(a,2) ^ rotr(a,13) ^ rotr(a,22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t t2 = S0 + maj;
        h=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    state[0]+=a; state[1]+=b; state[2]+=c; state[3]+=d;
    state[4]+=e; state[5]+=f; state[6]+=g; state[7]+=h;
}
}

namespace util {

std::string sha256(const std::string& input) {
    uint32_t state[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
                         0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    std::vector<uint8_t> msg(input.begin(), input.end());
    uint64_t bitlen = uint64_t(msg.size()) * 8;
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) msg.push_back(0);
    for (int i = 7; i >= 0; i--) msg.push_back(uint8_t((bitlen >> (i*8)) & 0xff));
    for (size_t i = 0; i < msg.size(); i += 64) sha256_transform(state, &msg[i]);
    char buf[3];
    std::string out;
    out.reserve(64);
    for (int i = 0; i < 8; i++) {
        for (int j = 3; j >= 0; j--) {
            std::snprintf(buf, sizeof(buf), "%02x", (state[i] >> (j*8)) & 0xff);
            out += buf;
        }
    }
    return out;
}

// Production systems should use Argon2id; this salted, iterated SHA-256 is a
// pragmatic stand-in that satisfies NFR3 (never plaintext) without extra deps.
std::string hashPassword(const std::string& password) {
    std::random_device rd;
    std::string salt;
    for (int i = 0; i < 16; i++) salt += "0123456789abcdef"[rd() % 16];
    std::string h = salt + password;
    for (int i = 0; i < 10000; i++) h = sha256(h);
    return salt + "$" + h;
}

bool verifyPassword(const std::string& password, const std::string& stored) {
    auto pos = stored.find('$');
    if (pos == std::string::npos) return false;
    std::string salt = stored.substr(0, pos);
    std::string expected = stored.substr(pos + 1);
    std::string h = salt + password;
    for (int i = 0; i < 10000; i++) h = sha256(h);
    return h == expected;
}

std::string nowIso() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buf);
}

int64_t parseIsoToEpoch(const std::string& iso) {
    if (iso.size() < 19) return 0;
    std::tm tm{};
    try {
        tm.tm_year = std::stoi(iso.substr(0, 4)) - 1900;
        tm.tm_mon  = std::stoi(iso.substr(5, 2)) - 1;
        tm.tm_mday = std::stoi(iso.substr(8, 2));
        tm.tm_hour = std::stoi(iso.substr(11, 2));
        tm.tm_min  = std::stoi(iso.substr(14, 2));
        tm.tm_sec  = std::stoi(iso.substr(17, 2));
    } catch (...) { return 0; }
    tm.tm_isdst = -1;
    return static_cast<int64_t>(std::mktime(&tm));
}

std::string epochToIso(int64_t epoch) {
    std::time_t t = static_cast<std::time_t>(epoch);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buf);
}

std::string addHours(const std::string& iso, int hours) {
    return epochToIso(parseIsoToEpoch(iso) + int64_t(hours) * 3600);
}

int64_t nowEpoch() { return static_cast<int64_t>(std::time(nullptr)); }

}