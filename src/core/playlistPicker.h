#pragma once
#include <string>
#include <vector>
#include "playerController.h"

// The list of playlists in an "add this song to a playlist" window.
// The playlists that already held the song when the window opened come first, so a row does not
// jump away when you tick it. One picker works for one open window; the player bar's "+" popup and the
// "Add to playlist" window of a song row each have their own.
struct PlaylistRow {
    std::string name;
    int count = 0;          // songs in the playlist
    bool saved = false;     // it held the song when the window opened
    bool checked = false;   // it holds the song now
};

class PlaylistPicker {
public:
    explicit PlaylistPicker(PlayerController& player);

    void open(int songNumber);                  // the window opened for this song (0 = no song)
    void openMany(const std::vector<int>& songNumbers);   // ...for several songs at once (a ticked selection)
    void setSearch(const std::string& text);    // only playlists whose name has the text (any case)

    std::vector<PlaylistRow> rows() const;
    int song() const;                           // the first song, 0 if none
    const std::vector<int>& songList() const;   // all of them

    // Adds the song to the playlist in row `row`, or takes it out. With several songs: all of them are added
    // unless the playlist already holds all of them, then all are taken out. Gives the playlist's name ("" if nothing happened).
    std::string toggle(int row);

    // A playlist was just made for this song: it counts as one that had it from the start.
    void markSaved(const std::string& playlistName);

private:
    PlayerController& player;
    std::vector<int> songNumbers;
    std::string search;
    std::vector<std::string> savedAtOpen;
    mutable std::vector<std::string> shownNames;     // the playlist of each row, same order as rows()
};
