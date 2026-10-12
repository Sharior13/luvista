#pragma once
#include <string>
#include <vector>
#include "app.h"                          // made by Slint from ui/app.slint
#include "imageCache.h"
#include "../audio/Recorder.h"
#include "../core/library.h"
#include "../core/lyrics.h"
#include "../core/playerController.h"
#include "../core/playlistPicker.h"
#include "../core/recordings.h"
#include "../core/settings.h"
#include "../core/trackList.h"
#include "../core/userSession.h"

// The wire between the screen (Slint) and the program.
//
// Clicks go from the screen to the core classes, and what they answer goes back to the screen.
// This class makes NO decisions of its own: searching, sorting, queues, accounts, file names and
// the like all live in src/core (and src/audio). Here a number becomes a SongInfo, a list becomes a
// model, a click becomes a call.
//
// The code is split over several files, one per part of the screen:
//   UIbridge.cpp            start, the clock, helpers every part uses
//   UIbridge_player.cpp     the player bar, the queue panel, the lyrics
//   UIbridge_pages.cpp      home, All Songs, Favorites, artists, the "..." menus
//   UIbridge_playlists.cpp  the playlists and the "add to playlist" windows
//   UIbridge_recorder.cpp   the Start Recording window and the Recordings page
//   UIbridge_session.cpp    login, profiles, account and settings
class UiBridge {
public:
    UiBridge(AppWindow& window, PlayerController& player, TrackList& tracks, Library& library,
             UserSession& session, Settings& settings, Recordings& recordings, Recorder& recorder);

    void start();                         // fills the screen, hooks up the buttons and starts the clock. Call once before ui.run()

private:
    // The song pages. The first four are Library pages, RECORDINGS has its own list of files.
    enum PageId { ALL_SONGS, FAVORITES, PLAYLIST, ARTIST, RECORDINGS, PAGE_COUNT };

    static constexpr int HOME_ROWS = 12;          // cards in a home row
    static constexpr int QUEUE_ROWS = 50;         // songs shown after the playing one
    static constexpr int ARTIST_TILES = 8;        // the Top Artists row of the profile page

    AppWindow& ui;
    PlayerController& player;
    TrackList& tracks;
    Library& library;
    UserSession& session;
    Settings& settings;
    Recordings& recordings;
    Recorder& recorder;

    ImageCache images;
    Lyrics lyrics;
    PlaylistPicker popup;                 // the "+" popup of the player bar (about the playing song)
    PlaylistPicker pickWindow;            // the "Add to playlist" window of a song row
    slint::Timer timer;

    // What each page shows right now, in the order of its rows: song numbers, or recording rows for RECORDINGS.
    // The screen only sends a row number back, this is how a row becomes a song again.
    std::vector<int> shown[PAGE_COUNT];
    std::string recordingSearch;          // the Recordings page search text and sort (the Library keeps those of the other pages)
    int recordingSort = 0;
    std::vector<std::string> playlistNames;       // sidebar order
    int openPlaylist = -1;                // index into playlistNames
    std::vector<int> queueNumbers;        // row of the queue panel -> song number
    std::vector<int> recentNumbers;       // row of the "recently played" tab -> song number
    std::vector<std::string> profileCards;        // card of the login screen -> profile name

    int lastCurrent = -1;
    std::string lastName;
    int lastLyric = -1;

    decltype(auto) state() const { return ui.global<AppState>(); }
    decltype(auto) sessionState() const { return ui.global<Session>(); }

    static Page pageOf(PageId id);
    static int pageIdOf(const slint::SharedString& listName);          // "songs" -> ALL_SONGS ... -1 if unknown

    // ---- UIbridge.cpp
    void refreshAll();                    // every song page, the home page and the sidebar numbers
    void reloadLibrary();                 // songs came or went: read the library again, then refreshAll
    SongInfo infoOf(int songNumber, const std::vector<char>& favorites);
    SongInfo infoOf(int songNumber);
    TrackInfo cardOf(int songNumber);

    // ---- UIbridge_player.cpp
    void connectPlayerBar();
    void togglePlay();
    void setFavorite(bool on);
    void syncPlayerBar(bool force);       // copies what the player knows into the player bar. Cheap, so it runs often
    void refreshQueue();
    void refreshLyrics(int songNumber);
    void refreshHeartAndTick();

    // ---- UIbridge_pages.cpp
    void connectSongPages();
    void connectSongMenus();
    void refreshPage(PageId page);
    void resetPage(PageId page);
    void playRow(PageId page, int row);
    void songMenuAction(PageId page, int row, const std::string& action);
    void deleteRow(PageId page, int row);
    void searchEverywhere(const std::string& text);
    void refreshHome();
    void refreshTopArtists();
    void refreshStorage();
    void addFolder();
    void openSongsPage();

    // ---- UIbridge_playlists.cpp
    void connectPlaylists();
    void refreshPlaylists();
    void refreshPopupChoices();
    void refreshPickChoices();
    bool createPlaylistNamed(const std::string& name, const std::string& with);
    void editPlaylistArt(int index);
    void playlistChanged(const std::string& name);   // a playlist's songs changed: whatever shows it is made again

    // ---- UIbridge_recorder.cpp
    void connectRecorder();
    void beginRecording(const std::string& name);
    void stopRecording();
    void refreshRecordings();
    void tickRecorder();                  // the clock and the level meter of the open recorder window

    // ---- UIbridge_session.cpp
    void connectSession();
    void connectAccount();
    void connectSettings();
    void refreshCards();
    void enterProfile();                  // somebody logged in (or started guest mode): show their things
    void leaveProfile();
    void showProfile();                   // the name, letter, picture and email of the logged in profile
};
