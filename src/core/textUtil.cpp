#include "textUtil.h"
#include <vector>

namespace {

// Windows-1252 differs from Latin-1 in 0x80-0x9F: those places hold curly quotes, dashes and so on. 0 = no character there.
const unsigned short cp1252High[32] = {
    0x20AC, 0, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0, 0x017D, 0,
    0, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0, 0x017E, 0x0178
};

void addCodePoint(std::string& out, unsigned int c)
{
    if (c < 0x80) {
        out += (char)c;
    }
    else if (c < 0x800) {
        out += (char)(0xC0 | (c >> 6));
        out += (char)(0x80 | (c & 0x3F));
    }
    else if (c < 0x10000) {
        out += (char)(0xE0 | (c >> 12));
        out += (char)(0x80 | ((c >> 6) & 0x3F));
        out += (char)(0x80 | (c & 0x3F));
    }
    else {
        out += (char)(0xF0 | (c >> 18));
        out += (char)(0x80 | ((c >> 12) & 0x3F));
        out += (char)(0x80 | ((c >> 6) & 0x3F));
        out += (char)(0x80 | (c & 0x3F));
    }
}

// the code points of a UTF-8 string. false if it is not valid UTF-8
bool decode(const std::string& s, std::vector<unsigned int>& out)
{
    const unsigned char* p = (const unsigned char*)s.data();
    size_t n = s.size();
    size_t i = 0;

    while (i < n) {
        unsigned char c = p[i];

        if (c < 0x80) {
            out.push_back(c);
            i++;
            continue;
        }

        int len;
        unsigned int cp;

        if (c >= 0xC2 && c <= 0xDF) { len = 2; cp = c & 0x1F; }
        else if (c >= 0xE0 && c <= 0xEF) { len = 3; cp = c & 0x0F; }
        else if (c >= 0xF0 && c <= 0xF4) { len = 4; cp = c & 0x07; }
        else
            return false;

        if (i + len > n)
            return false;

        for (int k = 1; k < len; k++) {
            if ((p[i + k] & 0xC0) != 0x80)
                return false;

            cp = (cp << 6) | (p[i + k] & 0x3F);
        }

        if ((len == 3 && (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF))) ||
            (len == 4 && (cp < 0x10000 || cp > 0x10FFFF)))
            return false;

        out.push_back(cp);
        i += len;
    }

    return true;
}

}   // namespace


bool TextUtil::isUtf8(const std::string& s)
{
    std::vector<unsigned int> unused;
    return decode(s, unused);
}


std::string TextUtil::lower(std::string s)
{
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z')
            c = (char)(c - 'A' + 'a');
    }

    return s;
}


std::string TextUtil::trim(const std::string& s)
{
    size_t a = s.find_first_not_of(" \t\r\n");

    if (a == std::string::npos)
        return "";

    return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}


std::string TextUtil::cleanTag(const std::string& s)
{
    std::string out;
    bool space = false;

    for (char c : s) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            space = true;
            continue;
        }

        if (space && !out.empty())
            out += ' ';

        space = false;
        out += c;
    }

    return out;
}


void TextUtil::appendCp1252(std::string& out, unsigned char byte)
{
    if (byte >= 0x80 && byte <= 0x9F) {
        unsigned short cp = cp1252High[byte - 0x80];

        if (cp == 0) {
            addCodePoint(out, 0xFFFD);          // a hole in Windows-1252
            return;
        }

        addCodePoint(out, cp);
        return;
    }

    addCodePoint(out, byte);
}


std::string TextUtil::repairMojibake(const std::string& s)
{
    std::vector<unsigned int> points;

    if (!decode(s, points))
        return s;

    std::string bytes;
    bool high = false;

    for (unsigned int cp : points) {
        if (cp > 0xFF)
            return s;                           // a real character beyond Latin-1: this text is fine as it is

        if (cp >= 0x80)
            high = true;

        bytes += (char)cp;
    }

    if (!high || !isUtf8(bytes))
        return s;

    return bytes;                               // "Ã©" -> the one letter it was meant to be
}


bool TextUtil::looksGarbled(const std::string& s)
{
    if (s.empty())
        return false;

    std::vector<unsigned int> points;

    if (!decode(s, points))
        return true;

    int questions = 0;
    int mojibake = 0;

    for (size_t i = 0; i < points.size(); i++) {
        unsigned int c = points[i];

        if (c == 0xFFFD || (c < 0x20 && c != '\t') || (c >= 0x7F && c <= 0x9F))
            return true;

        if (c == '?')
            questions++;

        // "Ã" or "Â" followed by a symbol in 0x80-0xBF is what UTF-8 looks like when it is read as Latin-1
        if ((c == 0xC3 || c == 0xC2) && i + 1 < points.size() && points[i + 1] >= 0x80 && points[i + 1] <= 0xBF)
            mojibake++;
    }

    if (mojibake > 0)
        return true;

    // "????" where a letter could not be converted
    return questions >= 2 && questions * 3 >= (int)points.size();
}
