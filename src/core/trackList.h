#pragma once

#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>

// One song in the library.
struct Song {
    int id = 0;
    std::string path;
    std::string title;
    std::string artist;
    std::string album;
    long long added = 0;     // seconds since 1970
    double seconds = 0;      // length. 0 = not read yet, -1 = the file has no length we can read
};

// What walking a folder found. It is made without touching the TrackList (so it can run on another
// thread) and handed to TrackList::apply() afterwards.
struct ScanResult {
    bool ok = false;                        // the folder was there
    std::string folder;
    std::vector<Song> found;                // songs that are not in the library yet (id = 0 until apply)
    std::vector<std::string> missing;       // library songs whose file is gone
};

// Better title / artist / length for a song that is already in the library.
struct SongFix {
    int id = 0;
    std::string title, artist, album;
    double seconds = 0;
    bool names = false;                     // title, artist and album are new
};

// A song taken out of the library on purpose. It is not added again by a scan, only by restoreRemoved().
struct RemovedSong {
    std::string path;
    std::string title, artist, album;
    long long added = 0;
    double seconds = 0;
};

class TrackList {
public:
    // The library never holds more songs than this.
    static constexpr int capacity = 10000;

    TrackList();

    bool scan(const std::string& folderPath);       // discover() + apply() in one go, on the calling thread
    bool removeSong(int n, bool hide = true);       // takes the song out. hide: also remember that, so scans leave it out
    std::string getFolder() const;                  // the folder scanned last

    // --- scanning in the background: take the snapshot on the main thread, call discover() anywhere, apply() on the main thread
    struct Snapshot {
        std::vector<std::string> paths;             // every song in the library
        std::vector<std::string> artists;           // artists known from tags, small letters (helps to read "A - B" file names)
        std::vector<std::string> removed;           // paths of songs that were removed on purpose: a scan must not bring them back
    };
    Snapshot snapshot() const;
    // `cancel` (if given) is looked at often: set it to stop early. A stopped walk gives ok = false.
    static ScanResult discover(const std::string& folderPath, const Snapshot& known, const std::atomic<bool>* cancel = nullptr);
    int apply(const ScanResult& result);            // adds and removes songs, saves. Gives how many songs changed

    // --- songs that still need their length read, or whose names were made by older, weaker rules
    std::vector<Song> needsWork() const;
    // A stopped repair gives nothing back (and so changes nothing).
    static std::vector<SongFix> repair(const std::vector<Song>& work, const Snapshot& known, const std::atomic<bool>* cancel = nullptr);
    void applyFixes(const std::vector<SongFix>& fixes);

    // --- the songs that were removed on purpose
    const std::vector<RemovedSong>& removedSongs() const;
    bool restoreRemoved(int index);                 // puts removed song `index` (from 0) back. false if its file is gone (it is forgotten then)
    int restoreAllRemoved();                        // gives how many came back
    void restoreRemovedInside(const std::string& folder);   // forgets the removed songs inside this folder (the user added it on purpose): a scan finds them again

    int         getCount() const;
    std::string getTrack(int n) const;
    std::string getTitle(int n) const;
    std::string getArtist(int n) const;
    std::string getAlbum(int n) const;
    long long   getAdded(int n) const;
    double      getSeconds(int n) const;
    int         getId(int n) const;

    int  find(const std::string& path) const;
    int  positionOfId(int id) const;
    bool matches(int n, const std::string& query) const;

private:
    std::string folder;
    int nextId;
    int formatVersion;
    std::vector<Song> songs;                        // all songs, in memory. songs[0] is song number 1
    std::unordered_map<std::string, int> pathIndex; // plain address -> song number
    std::unordered_map<int, int> idIndex;           // id -> song number
    std::vector<RemovedSong> removedList;
    std::vector<std::string> removedKeys;           // plain addresses of removedList, for quick lookups

    void loadFile();                // read the notebook file into songs
    void saveFile();                // write songs into the notebook file
    void rebuildIndexes();          // after songs were added, removed or moved
    void loadRemoved();
    void saveRemoved();
    bool isRemoved(const std::string& path) const;
};
