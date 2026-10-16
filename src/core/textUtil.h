#pragma once
#include <string>

// Small helpers for the text of song tags and file names. No screen, no files.
namespace TextUtil
{
    bool isUtf8(const std::string& s);                      // true when s is valid UTF-8
    std::string lower(std::string s);                       // small letters (plain English letters only)
    std::string trim(const std::string& s);                 // spaces, tabs and line breaks cut off both ends
    std::string cleanTag(const std::string& s);             // trimmed, runs of spaces made single, line breaks made spaces

    // Text read from a tag that was written in the wrong code page often shows up as "Ã©" instead of "e-acute".
    // When the text is really UTF-8 that was read as Latin-1, this gives the real text back, otherwise it returns s unchanged.
    std::string repairMojibake(const std::string& s);

    // true when the text is hard to trust: broken characters, control characters, rows of '?' from a lossy
    // conversion, or leftovers of a wrong code page. The caller then falls back to the file name.
    bool looksGarbled(const std::string& s);

    // a Latin-1 / Windows-1252 byte as a UTF-8 string
    void appendCp1252(std::string& out, unsigned char byte);
}
