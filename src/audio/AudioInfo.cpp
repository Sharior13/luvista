#include "AudioInfo.h"
#include "miniaudio.h"                    // only the declarations. The code lives in miniaudioBackend.cpp
#include <cstdint>
#include <fstream>

namespace {

unsigned int bigEndian32(const unsigned char* b)
{
    return ((unsigned int)b[0] << 24) | ((unsigned int)b[1] << 16) | ((unsigned int)b[2] << 8) | (unsigned int)b[3];
}

bool endsWithM4a(const std::string& path)
{
    if (path.size() < 4)
        return false;

    std::string end = path.substr(path.size() - 4);

    for (char& c : end) {
        if (c >= 'A' && c <= 'Z')
            c = (char)(c - 'A' + 'a');
    }

    return end == ".m4a";
}

// miniaudio cannot open .m4a (Windows decodes those while playing), but the length is written in the
// "mvhd" box at moov > mvhd: a time scale (ticks per second) and a duration (ticks).
double m4aSeconds(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file)
        return 0.0;

    file.seekg(0, std::ios::end);
    uint64_t fileSize = (uint64_t)file.tellg();
    uint64_t pos = 0;

    // top level boxes: find "moov"
    while (pos + 8 <= fileSize) {
        unsigned char h[16];
        file.clear();
        file.seekg((std::streamoff)pos);
        file.read((char*)h, 8);

        if (file.gcount() != 8)
            return 0.0;

        uint64_t size = bigEndian32(h);
        uint64_t head = 8;

        if (size == 1) {
            file.read((char*)h + 8, 8);

            if (file.gcount() != 8)
                return 0.0;

            size = ((uint64_t)bigEndian32(h + 8) << 32) | bigEndian32(h + 12);
            head = 16;
        }
        else if (size == 0) {
            size = fileSize - pos;
        }

        if (size < head)
            return 0.0;

        if (h[4] == 'm' && h[5] == 'o' && h[6] == 'o' && h[7] == 'v') {
            // inside moov: boxes again, look for "mvhd"
            uint64_t inner = pos + head;
            uint64_t end = pos + size;

            while (inner + 8 <= end && inner + 8 <= fileSize) {
                unsigned char b[8];
                file.clear();
                file.seekg((std::streamoff)inner);
                file.read((char*)b, 8);

                if (file.gcount() != 8)
                    return 0.0;

                uint64_t innerSize = bigEndian32(b);

                if (innerSize < 8)
                    return 0.0;

                if (b[4] == 'm' && b[5] == 'v' && b[6] == 'h' && b[7] == 'd') {
                    unsigned char body[32];
                    file.read((char*)body, 32);

                    if (file.gcount() < 28)
                        return 0.0;

                    if (body[0] == 1) {                         // version 1: 64 bit times
                        uint64_t scale = bigEndian32(body + 20);
                        uint64_t ticks = ((uint64_t)bigEndian32(body + 24) << 32) | bigEndian32(body + 28);
                        return scale > 0 ? (double)ticks / (double)scale : 0.0;
                    }

                    uint64_t scale = bigEndian32(body + 12);    // version 0: 32 bit times
                    uint64_t ticks = bigEndian32(body + 16);
                    return scale > 0 ? (double)ticks / (double)scale : 0.0;
                }

                inner += innerSize;
            }

            return 0.0;
        }

        pos += size;
    }

    return 0.0;
}

}   // namespace


double AudioInfo::seconds(const std::string& path)
{
    if (endsWithM4a(path))
        return m4aSeconds(path);

    ma_decoder decoder;

    if (ma_decoder_init_file(path.c_str(), nullptr, &decoder) != MA_SUCCESS)
        return 0.0;

    ma_uint64 frames = 0;
    double result = 0.0;

    if (ma_decoder_get_length_in_pcm_frames(&decoder, &frames) == MA_SUCCESS && decoder.outputSampleRate > 0)
        result = (double)frames / (double)decoder.outputSampleRate;

    ma_decoder_uninit(&decoder);
    return result;
}
