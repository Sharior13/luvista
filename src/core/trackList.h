#pragma once

#include <string>
#include <vector>

// One song in the library.
struct Song {
    int id = 0;
    std::string path;
    std::string title;
    std::string artist;
    std::string album;
    long long added = 0;     // seconds since 1970
};

class TrackList {
public:
    // The library never holds more songs than this.
    // The songs live in a vector (on the heap): a plain array this big would
    // not fit on the 1 MB stack when a TrackList is a local variable.
    static constexpr int capacity = 10000;

    TrackList();

    bool scan(const std::string& folderPath);
    bool removeSong(int n);

    int         getCount() const;
    std::string getTrack(int n) const;
    std::string getTitle(int n) const;
    std::string getArtist(int n) const;
    std::string getAlbum(int n) const;
    long long   getAdded(int n) const;
    int         getId(int n) const;

    int  find(const std::string& path) const;
    int  positionOfId(int id) const;
    bool matches(int n, const std::string& query) const;

private:
    std::string folder;
    int nextId;
    int songCount;          // how many places of the array are filled
    std::vector<Song> songs;   // all songs are kept here, in memory (size is always `capacity`)

    void loadFile();            // read the notebook file into songs
    void saveFile();            // write songs into the notebook file
};