#include "pathUtil.h"
#include <cctype>

std::string PathUtil::plain(const std::string& path)
{
    std::string text = path;

    for (char& c : text)
        c = c == '\\' ? '/' : (char)std::tolower((unsigned char)c);

    while (text.size() > 1 && text.back() == '/')
        text.pop_back();

    return text;
}


bool PathUtil::isInside(const std::string& path, const std::string& folder)
{
    std::string inner = plain(path);
    std::string outer = plain(folder);

    if (outer.empty())
        return false;

    return inner.size() > outer.size() && inner.compare(0, outer.size(), outer) == 0 && inner[outer.size()] == '/';
}


bool PathUtil::same(const std::string& a, const std::string& b)
{
    return plain(a) == plain(b);
}
