#pragma once
#include <string>

// playlists.dat holds the playlist names.
// Each playlist has its own file "playlist-<name>.dat" holding song ids.
class Playlists {
public:
    // The list of playlists
    bool create(const std::string& name);        // false if name is empty, bad, or already used
    bool remove(const std::string& name);        // deletes the playlist and its song file
    bool exists(const std::string& name) const;
    int count() const;                           // how many playlists
    std::string getName(int n) const;            // name of playlist number n (first is 1)

    // The songs inside one playlist
    bool addSong(const std::string& name, int id);
    bool removeSong(const std::string& name, int id);   // takes one song out of the playlist
    void removeSongEverywhere(int id);                  // takes a song out of every playlist
    bool hasSong(const std::string& name, int id) const;
    int songCount(const std::string& name) const;
    int getSong(const std::string& name, int n) const;   // song id (0 if no line n)
};
