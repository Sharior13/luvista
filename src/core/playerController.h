#pragma once
#include <string>
#include "../audio/audioBackend.h"
#include "trackList.h"
#include "favorites.h"
#include "recentList.h"
#include "playlist.h"

class PlayerController {
private:
    AudioBackend& audio;
    TrackList& tracks;
    Favorite favorites;
    RecentList recent;
    Playlists playlists;

    int current;
    bool paused;
    int volumePercent;
    std::string trackName;
    std::string errorText;

    void playTrack(int n, int step);

public:
    PlayerController(AudioBackend& a, TrackList& t);

    bool openFolder(const std::string& folder);

    void next();
    void previous();
    void playSong(int n);

    void pause();
    void resume();
    void togglePlay();                  // NEW: one button for pause and resume
    void restart();
    void forward(double seconds = 10);
    void back(double seconds = 10);
    void seekTo(double seconds);        // NEW: for the seek bar
    void update();

    void volumeUp();
    void volumeDown();
    void setVolume(int percent);        // NEW: for the volume slider

    void toggleFavorite();
    bool isFavorite() const;
    int favoriteCount() const;
    std::string favorite(int n) const;
    std::string favoriteTitle(int n) const;

    int recentCount() const;
    std::string recentSong(int n) const;
    std::string recentTitle(int n) const;

    bool createPlaylist(const std::string& name);
    int playlistCount() const;
    std::string playlistName(int n) const;
    bool addCurrentToPlaylist(const std::string& name);
    bool addToPlaylist(const std::string& name, int songNumber);   // NEW: the "+" on a row
    int playlistSongCount(const std::string& name) const;
    std::string playlistSongTitle(const std::string& name, int n) const;

    // Search. Each one gives the position of the nth match (first match is 1),
    // or 0 when there are fewer than n matches. An empty query matches everything.
    int findSong(const std::string& query, int nth) const;        // a song number (all songs)
    int findFavorite(const std::string& query, int nth) const;    // a row in favorites
    int findRecent(const std::string& query, int nth) const;      // a row in recently played
    int findInPlaylist(const std::string& name, const std::string& query, int nth) const;   // a row in a playlist

    int volume() const;
    bool isPaused() const;
    int currentIndex() const;
    int total() const;
    double position() const;
    double duration() const;
    std::string songName() const;
    std::string lastError() const;
    std::string songTitle(int n) const;
    std::string nextSongTitle() const;          // NEW: "Next in queue" card
    long long songAdded(int n) const;           // NEW: "Date added" column
};