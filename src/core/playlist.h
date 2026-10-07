#pragma once
#include <string>

// playlists.dat holds the playlist names.
// Each playlist has its own file "playlist-<name>.dat" holding song ids.
class Playlists {
public:
    // The list of playlists
    bool create(const std::string& name);        // false if name is empty, bad, or already used
    bool exists(const std::string& name) const;
    int count() const;                           // how many playlists
    std::string getName(int n) const;            // name of playlist number n (first is 1)

    // The songs inside one playlist
    bool addSong(const std::string& name, int id);
    bool hasSong(const std::string& name, int id) const;
    int songCount(const std::string& name) const;
    int getSong(const std::string& name, int n) const;   // song id (0 if no line n)
};
