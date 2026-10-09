#pragma once
#include <string>

class TrackList {
private:
    std::string folder;     // the folder we scanned last
    int count;              // how many songs are in the notebook
    int nextId;             // the id the next new song gets

    void countSongs();                          // counts the lines in the notebook
    int find(const std::string& path) const;    // line number of a path, 0 if not there

public:
    TrackList();

    // Adds the new songs in this folder. Forgets songs whose file is gone.
    // False if the folder does not exist.
    bool scan(const std::string& folderPath);

    // Takes song number n out of the notebook (the file on disk stays).
    bool removeSong(int n);

    int getCount() const;

    // Song number n (the first song is number 1)
    std::string getTrack(int n) const;      // the file address
    std::string getTitle(int n) const;      // the name
    std::string getArtist(int n) const;     // the artist
    std::string getAlbum(int n) const;      // the album
    int getId(int n) const;                 // the id (0 if no such song)
    long long getAdded(int n) const;        // when it was found

    // From an id to a song number: 1, 2, 3 ... or 0 if there is no such song
    int positionOfId(int id) const;

    // Does song number n match the search words? Looks at the title, artist and album,
    // ignores big/small letters. An empty search matches every song.
    bool matches(int n, const std::string& query) const;

    std::string getFolder() const;
};