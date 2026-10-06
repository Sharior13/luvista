#pragma once
#include <string>

// playlists.txt holds the playlist names.
// Each playlist has its own file "playlist-<name>.txt" holding song addresses.
class Playlists {
public:
    // The list of playlists
    bool create(const std::string& name);        // false if name is empty, bad, or already used
    bool exists(const std::string& name) const;
    int count() const;                           // how many playlists
    std::string getName(int n) const;            // name of playlist number n (first is 1)

    // The songs inside one playlist
    bool addSong(const std::string& name, const std::string& path);
    bool hasSong(const std::string& name, const std::string& path) const;
    int songCount(const std::string& name) const;
    std::string getSong(const std::string& name, int n) const;
};