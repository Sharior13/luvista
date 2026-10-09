#pragma once

#include <string>

// Decodes .m4a (AAC) into 16-bit PCM in memory using Windows Media Foundation
// (part of Windows, nothing to install, nothing written to disk).
// Windows only; miniaudio plays the result from memory.
namespace M4a
{
    // true if the path ends in .m4a (any letter case)
    bool isM4a(const std::string& path);

    // Decode the whole file. On success *pcm = std::malloc'd memory (free with std::free),
    // *frames/*channels/*sampleRate describe the sound. On failure false + error says why.
    bool decode(const std::string& path, short** pcm, unsigned long long* frames,
        unsigned int* channels, unsigned int* sampleRate, std::string& error);
}
