#include "credentials.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>

namespace {

// ---- SHA-256 ---------------------------------------------------------------------------------
struct Sha256 {
    uint32_t h[8];
    uint8_t buffer[64];
    uint64_t total;
    size_t used;

    Sha256() { reset(); }

    void reset() {
        static const uint32_t start[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                                           0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
        std::memcpy(h, start, sizeof(h));
        total = 0;
        used = 0;
    }

    static uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

    void block(const uint8_t* p) {
        static const uint32_t k[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };

        uint32_t w[64];

        for (int i = 0; i < 16; i++)
            w[i] = ((uint32_t)p[i * 4] << 24) | ((uint32_t)p[i * 4 + 1] << 16) | ((uint32_t)p[i * 4 + 2] << 8) | p[i * 4 + 3];

        for (int i = 16; i < 64; i++) {
            uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];

        for (int i = 0; i < 64; i++) {
            uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            uint32_t ch = (e & f) ^ (~e & g);
            uint32_t t1 = hh + s1 + ch + k[i] + w[i];
            uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t t2 = s0 + maj;

            hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }

        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    void add(const uint8_t* data, size_t size) {
        total += size;

        while (size > 0) {
            size_t take = std::min(size, (size_t)64 - used);
            std::memcpy(buffer + used, data, take);
            used += take;
            data += take;
            size -= take;

            if (used == 64) {
                block(buffer);
                used = 0;
            }
        }
    }

    void finish(uint8_t out[32]) {
        uint64_t bits = total * 8;
        uint8_t one = 0x80;
        uint8_t zero = 0;

        add(&one, 1);
        while (used != 56)
            add(&zero, 1);

        uint8_t length[8];
        for (int i = 0; i < 8; i++)
            length[i] = (uint8_t)(bits >> (56 - 8 * i));
        add(length, 8);

        for (int i = 0; i < 8; i++) {
            out[i * 4] = (uint8_t)(h[i] >> 24);
            out[i * 4 + 1] = (uint8_t)(h[i] >> 16);
            out[i * 4 + 2] = (uint8_t)(h[i] >> 8);
            out[i * 4 + 3] = (uint8_t)h[i];
        }
    }
};

typedef std::vector<uint8_t> Bytes;

Bytes sha256(const Bytes& data) {
    Sha256 s;
    s.add(data.data(), data.size());
    Bytes out(32);
    s.finish(out.data());
    return out;
}

Bytes toBytes(const std::string& text) {
    return Bytes(text.begin(), text.end());
}

std::string toHex(const Bytes& bytes) {
    static const char* digits = "0123456789abcdef";
    std::string out;

    for (uint8_t b : bytes) {
        out += digits[b >> 4];
        out += digits[b & 15];
    }
    return out;
}

// ---- HMAC-SHA256 ---------------------------------------------------------------------------------
Bytes hmac(Bytes key, const Bytes& message) {
    if (key.size() > 64)
        key = sha256(key);
    key.resize(64, 0);

    Bytes inner(64), outer(64);
    for (int i = 0; i < 64; i++) {
        inner[i] = key[i] ^ 0x36;
        outer[i] = key[i] ^ 0x5c;
    }

    inner.insert(inner.end(), message.begin(), message.end());
    Bytes innerHash = sha256(inner);
    outer.insert(outer.end(), innerHash.begin(), innerHash.end());
    return sha256(outer);
}

std::string randomSalt() {
    std::random_device device;
    Bytes bytes(16);

    for (uint8_t& b : bytes)
        b = (uint8_t)(device() & 0xFF);

    return toHex(bytes);
}

const int ROUNDS = 20000;

// A text compare that takes the same time wherever the texts differ.
bool sameText(const std::string& a, const std::string& b) {
    if (a.size() != b.size())
        return false;

    unsigned char diff = 0;
    for (size_t i = 0; i < a.size(); i++)
        diff |= (unsigned char)(a[i] ^ b[i]);

    return diff == 0;
}

}   // namespace


std::string Credentials::sha256Hex(const std::string& text)
{
    return toHex(sha256(toBytes(text)));
}


// PBKDF2 with HMAC-SHA256 (RFC 8018).
std::string Credentials::pbkdf2Hex(const std::string& password, const std::string& salt, int rounds, int bytes)
{
    Bytes key = toBytes(password);
    Bytes result;

    for (uint32_t block = 1; (int)result.size() < bytes; block++) {
        Bytes first = toBytes(salt);
        first.push_back((uint8_t)(block >> 24));
        first.push_back((uint8_t)(block >> 16));
        first.push_back((uint8_t)(block >> 8));
        first.push_back((uint8_t)block);

        Bytes u = hmac(key, first);
        Bytes t = u;

        for (int i = 1; i < rounds; i++) {
            u = hmac(key, u);
            for (size_t j = 0; j < t.size(); j++)
                t[j] ^= u[j];
        }

        result.insert(result.end(), t.begin(), t.end());
    }

    result.resize(bytes);
    return toHex(result);
}


std::string Credentials::makeFingerprint(const std::string& password)
{
    std::string salt = randomSalt();

    return "pbkdf2$" + std::to_string(ROUNDS) + "$" + salt + "$" + pbkdf2Hex(password, salt, ROUNDS, 32);
}


bool Credentials::matches(const std::string& password, const std::string& fingerprint)
{
    size_t a = fingerprint.find('$');
    size_t b = a == std::string::npos ? a : fingerprint.find('$', a + 1);
    size_t c = b == std::string::npos ? b : fingerprint.find('$', b + 1);

    if (c == std::string::npos || fingerprint.substr(0, a) != "pbkdf2")
        return false;

    int rounds = std::atoi(fingerprint.substr(a + 1, b - a - 1).c_str());
    std::string salt = fingerprint.substr(b + 1, c - b - 1);
    std::string hash = fingerprint.substr(c + 1);

    if (rounds < 1 || rounds > 10000000)
        return false;

    return sameText(pbkdf2Hex(password, salt, rounds, 32), hash);
}
