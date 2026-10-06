#pragma once
#include <string>

class TrackList {
private:
    std::string folder;   // the folder where the music is kept
    int count;            // how many songs we found

public:
    TrackList();

    // Look in the folder and write down every song. False if the folder does not exist.
    bool scan(const std::string& folderPath);

    // How many songs were found
    int getCount() const;

    // The address of song number n (the first song is number 1)
    std::string getTrack(int n) const;

    // The folder we scanned (for the Storage screen)
    std::string getFolder() const;
};
