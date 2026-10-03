#pragma once
#include <string>

// TrackList is like a librarian.
// It looks in a folder, finds all the songs, and remembers them.
class TrackList {
private:
    std::string folder;   // the folder where the songs are kept
    int count;            // how many songs we found

public:
    TrackList();

    // Looks inside a folder and writes down every song it finds.
    // Says false if the folder does not exist.
    bool scan(const std::string& folderPath);

    // How many songs were found
    int getCount() const;

    // The address of song number n (1 is the first song)
    std::string getTrack(int n) const;
};
