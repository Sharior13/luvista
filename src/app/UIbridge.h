#pragma once
#include <map>
#include <string>
#include <vector>
#include "app.h"                          // made by Slint from ui/app.slint
#include "../audio/Recorder.h"
#include "../core/lyrics.h"
#include "../core/playerController.h"
#include "../core/profiles.h"
#include "../core/trackList.h"

// The wire between the screen (Slint) and the music player (PlayerController).
// Clicks go from the screen to the player. Titles, lists and times go back.
class UiBridge {
private:
    // One song, copied out of the librarian once, so search and sort are quick.
    struct Song {
        int number = 0;               // the song number (1, 2, 3 ...)
        int id = 0;                   // never changes, even when song numbers move
        std::string path;
        std::string title;
        std::string artist;
        std::string album;
        std::string titleLow;         // the same words in small letters, for search and sort
        std::string artistLow;
        std::string albumLow;
        long long added = 0;          // seconds since 1970
        double seconds = 0;           // length of the song
        bool recording = false;       // a microphone recording (of any profile). These are kept out of All Songs
        bool mine = false;            // a recording of the profile that is logged in
    };

    // What one song page (All Songs, Favorites, a playlist or Recordings) is showing right now.
    struct SongPage {
        std::string search;           // in small letters
        int sort;                     // 0 recently added, 1 title, 2 artist, 3 length
        std::vector<int> shown;       // song numbers, in the order of the rows
    };

    AppWindow& ui;
    PlayerController& player;
    TrackList& tracks;

    std::vector<Song> library;        // library[n] is song n. Slot 0 is empty because songs start at 1.
    SongPage pages[4];                // All Songs, Favorites, playlist, Recordings

    std::vector<std::string> playlistNames;   // same order as the sidebar
    int openPlaylist;                         // the playlist the page shows, -1 for none
    std::vector<std::string> choiceNames;     // the rows of the "+" popup
    std::vector<std::string> savedAtOpen;     // playlists that had the song when the popup opened
    std::string pickerSearch;                 // the popup's search box, in small letters
    std::vector<int> recentNumbers;           // the songs behind the "Recently played" tab
    std::string globalSearch;                 // what the top bar is searching for, in small letters. "" = no search going on

    // pictures. Reading a picture is slow, so each one is read once and kept
    std::map<int, std::string> coverPaths;            // song id -> picture file ("" = the song has none)
    std::map<std::string, slint::Image> images;       // picture file -> the picture, ready for the screen
    std::map<int, double> lengths;                    // song id -> seconds, so a file is not opened again on every login

    // who is logged in
    Profiles profiles;
    std::vector<std::string> profileNames;    // same order as the cards on the login screen

    Recorder recorder;                        // the microphone
    Lyrics lyrics;                            // the lyrics of the playing song
    int lastLyric;                            // the lyric line the screen shows now

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
    void connectSession();

    void loadLibrary();
    bool valid(int number) const;
    double lengthOf(const Song& s);
    slint::Image imageFile(const std::string& path);
    slint::Image coverOf(int number);
    slint::Image playlistImage(const std::string& name);
    SongInfo infoOf(int number);
    TrackInfo cardOf(int number);
    std::vector<int> pageSource(int page) const;
    std::string pageName(int page) const;
    void refreshPage(int page);
    void refreshAllPages();
    void resetPage(int page);
    void playRow(int page, int row);
    void searchEverywhere(const std::string& text);

    void refreshPlaylists();
    bool inPlaylist(const std::string& name, int number) const;
    std::string newPlaylistName() const;
    void refreshChoices();
    void refreshHeartAndTick();
    void openPopup();
    void toggleInPlaylist(int row);
    void newPlaylistFromTrack();
    void editPlaylistArt(int index);

    void refreshHome();
    void refreshQueue();
    void refreshLyrics(int current);
    void playCard(const std::string& title, const std::string& artist);

    void togglePlay();
    void setFavorite(bool on);
    void syncPlayerBar(bool force);
    void addFolder();
    void openSongsPage();

    void toggleRecording();
    void finishRecording();

    void refreshProfileList();
    void enterProfile(const std::string& name, bool guest);
    void leaveProfile();

public:
    UiBridge(AppWindow& window, PlayerController& p, TrackList& t);

    void start();
};
