#pragma once
#include <string>

// Comparing addresses of files and folders the way Windows does: letters in any case, "/" and "\" the same.
namespace PathUtil {

    std::string plain(const std::string& path);                        // small letters, "/" slashes, no "/" at the end
    bool isInside(const std::string& path, const std::string& folder); // is the file inside the folder (at any depth)?
    bool same(const std::string& a, const std::string& b);             // do both mean the same folder?

}
