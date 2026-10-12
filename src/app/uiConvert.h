#pragma once
#include <string>
#include <string_view>
#include "slint.h"

// Small translators between what the program knows and what the screen shows. No decisions in here.
namespace UiConvert
{
    slint::SharedString toSlint(const std::string& text);    // text that is not UTF-8 is repaired first, Slint would abort on it
    std::string toStd(const slint::SharedString& text);
    std::string lengthText(double seconds);                  // 3:22
    std::string ago(long long added);                        // "today", "yesterday", "3 days", "2 weeks"
    std::string countOf(long long n, const std::string& word);   // "1 song", "2 songs"
}
