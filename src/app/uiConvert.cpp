#include "uiConvert.h"
#include <cstdio>
#include <ctime>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

// ---- text -------------------------------------------------------------------------------------

namespace {

//true when s is valid UTF-8. Slint aborts the whole program (not just the one string) on anything else.
bool isUtf8(const std::string& s)
{
    const unsigned char* p = (const unsigned char*)s.data();
    size_t n = s.size();
    size_t i = 0;

    while (i < n) {
        unsigned char c = p[i];

        if (c < 0x80) {
            i++;
            continue;
        }

        int len;
        unsigned int cp;

        if (c >= 0xC2 && c <= 0xDF) { len = 2; cp = c & 0x1F; }
        else if (c >= 0xE0 && c <= 0xEF) { len = 3; cp = c & 0x0F; }
        else if (c >= 0xF0 && c <= 0xF4) { len = 4; cp = c & 0x07; }
        else
            return false;                                   //a stray byte, e.g. a Windows-1252 letter like e-acute or a curly apostrophe

        if (i + len > n)
            return false;                                   //the character is cut off

        for (int k = 1; k < len; k++) {
            if ((p[i + k] & 0xC0) != 0x80)
                return false;

            cp = (cp << 6) | (p[i + k] & 0x3F);
        }

        if (len == 3 && (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)))
            return false;                                   //too long a form, or a lone surrogate

        if (len == 4 && (cp < 0x10000 || cp > 0x10FFFF))
            return false;

        i += len;
    }

    return true;
}

}   // namespace


//a normal string as a slint string. Text that is not valid UTF-8 (file names come from windows
//in the ANSI code page) is converted first, so one odd song name can never crash the program.
slint::SharedString UiConvert::toSlint(const std::string& text)
{
    if (isUtf8(text))
        return slint::SharedString(std::string_view(text));

    std::string fixed;

#ifdef _WIN32
    int wideLength = MultiByteToWideChar(CP_ACP, 0, text.data(), (int)text.size(), nullptr, 0);

    if (wideLength > 0) {
        std::wstring wide((size_t)wideLength, L'\0');
        MultiByteToWideChar(CP_ACP, 0, text.data(), (int)text.size(), &wide[0], wideLength);

        int utf8Length = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLength, nullptr, 0, nullptr, nullptr);

        if (utf8Length > 0) {
            fixed.assign((size_t)utf8Length, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLength, &fixed[0], utf8Length, nullptr, nullptr);
        }
    }
#endif

    if (fixed.empty() || !isUtf8(fixed)) {                  //last resort: read every byte as a Latin-1 letter
        fixed.clear();

        for (unsigned char c : text) {
            if (c < 0x80) {
                fixed += (char)c;
            }
            else {
                fixed += (char)(0xC0 | (c >> 6));
                fixed += (char)(0x80 | (c & 0x3F));
            }
        }
    }

    return slint::SharedString(std::string_view(fixed));
}


std::string UiConvert::toStd(const slint::SharedString& text)
{
    return std::string(std::string_view(text));
}


// ---- numbers and dates --------------------------------------------------------------------------

//3:22
std::string UiConvert::lengthText(double seconds)
{
    int total = (int)(seconds + 0.5);
    char text[16];

    std::snprintf(text, sizeof text, "%d:%02d", total / 60, total % 60);
    return text;
}

//"1 week", "2 weeks"
std::string UiConvert::countOf(long long n, const std::string& word)
{
    if (n == 1)
        return "1 " + word;

    return std::to_string(n) + " " + word + "s";
}

//how long ago a song was added: "today", "yesterday", "3 days", "2 weeks"
std::string UiConvert::ago(long long added)
{
    long long days = ((long long)std::time(nullptr) - added) / 86400;

    if (added <= 0 || days <= 0) return "today";
    if (days == 1) return "yesterday";
    if (days < 7) return countOf(days, "day");
    if (days < 30) return countOf(days / 7, "week");
    if (days < 365) return countOf(days / 30, "month");
    return countOf(days / 365, "year");
}
