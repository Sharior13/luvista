#pragma once
#include <string>

// Knows where Luvista keeps its files: C:\Users\<you>\AppData\Local\Luvista
class AppPaths {
public:
    static std::string root();                                    // the main folder
    static std::string indexFile();                               // the list of all songs
    static std::string profileFolder(const std::string& profile); // favorites, recent, playlists
};
