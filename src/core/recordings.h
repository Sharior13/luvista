#pragma once
#include <string>
#include <vector>

// The finished recordings of the profile that is logged in: the .mp3 files inside its recordings folder
// (AppPaths::recordingsFolder()). They are not part of the song library, so they have their own list.
class Recordings {
public:
    void refresh();                              // reads the folder again. Call after a recording was saved or deleted

    int count() const;                           // all recordings
    std::string path(int row) const;             // the file (row 1 is the newest)
    std::string title(int row) const;            // the file name without ".mp3"
    long long added(int row) const;              // when the file was written, seconds since 1970
    double seconds(int row) const;               // how long it is

    bool remove(int row);                        // deletes the file from the computer, then refreshes

    // The rows to show, as row numbers: only the recordings whose title holds `search` (any case),
    // sorted. sort: 0 = newest first, 1 = title A-Z, 2 = longest first.
    std::vector<int> rows(const std::string& search, int sort) const;
    double totalSeconds(const std::vector<int>& shown) const;

private:
    struct Entry {
        std::string path;
        std::string title;
        long long added;
        double seconds;
    };
    std::vector<Entry> entries;
};
