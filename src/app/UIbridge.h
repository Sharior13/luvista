#pragma once
#include <string>
#include <vector>
#include "app.h"                          // made by Slint from ui/app.slint
#include "../core/playerController.h"
#include "../core/trackList.h"

// The wire between the screen (Slint) and the music player (PlayerController).
// Clicks go from the screen to the player. Titles, lists and times go back.
class UiBridge {
private:
    // One song, copied out of the librarian once, so search and sort are quick.
    struct Song {
        int number;                   // the song number (1, 2, 3 ...)
        std::string title;
        std::string artist;
        std::string album;
        std::string titleLow;         // the same words in small letters, for search and sort
        std::string artistLow;
        std::string albumLow;
        long long added;              // seconds since 1970
        double seconds;               // length of the song
    };

    // What one song page (All Songs, Favorites or a playlist) is showing right now.
    struct SongPage {
        std::string search;           // in small letters
        int sort;                     // 0 recently added, 1 title, 2 artist, 3 length
        std::vector<int> shown;       // song numbers, in the order of the rows
    };

    AppWindow& ui;
    PlayerController& player;
    TrackList& tracks;

    std::vector<Song> library;        // library[n] is song n. Slot 0 is empty because songs start at 1.
    SongPage pages[3];                // All Songs, Favorites, playlist

    std::vector<std::string> playlistNames;   // same order as the sidebar
    int openPlaylist;                         // the playlist the page shows, -1 for none
    std::vector<std::string> choiceNames;     // the rows of the "+" popup
    std::vector<std::string> savedAtOpen;     // playlists that had the song when the popup opened
    std::string pickerSearch;                 // the popup's search box, in small letters
    std::vector<int> recentNumbers;           // the songs behind the "Recently played" tab

    int lastCurrent;                  // the song the player bar is showing
    slint::Timer timer;

    // Gives the AppState / Session parts of the screen.
    decltype(auto) state() const { return ui.global<AppState>(); }
    decltype(auto) session() const { return ui.global<Session>(); }

    // Hooks the screen's callbacks up to the player.
    void connectPlayerBar();
    void connectSongPages();
    void connectPlaylists();
    void connectOtherScreens();

    void loadLibrary();
    SongInfo infoOf(int number) const;
    std::vector<int> pageSource(int page) const;
    void refreshPage(int page);
    void resetPage(int page);
    void playRow(int page, int row);

    void refreshPlaylists();
    bool inPlaylist(const std::string& name, int number) const;
    std::string newPlaylistName() const;
    void refreshChoices();
    void refreshHeartAndTick();
    void openPopup();
    void toggleInPlaylist(int row);
    void newPlaylistFromTrack();

    void refreshHome();
    void refreshQueue();
    void playCard(const std::string& title, const std::string& artist);

    void togglePlay();
    void setFavorite(bool on);
    void syncPlayerBar(bool force);
    void addFolder();
    void openSongsPage();

public:
    UiBridge(AppWindow& window, PlayerController& p, TrackList& t);

    void start();
};