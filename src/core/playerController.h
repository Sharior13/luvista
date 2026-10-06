#pragma once
#include <string>
#include "../audio/audioBackend.h"
#include "trackList.h"
#include "favorites.h"
#include "recentList.h"
#include "playlist.h"            

class PlayerController {
private:
    AudioBackend& audio;    // the speaker (the & means "the real one, not a copy")
    TrackList& tracks;      // the librarian
    Favorite favorites;     // the favorites notebook
    RecentList recent;      // the recently played notebook
    Playlists playlists;        

    void playTrack(int n);  // private: only the DJ uses this inside

public:
    PlayerController(AudioBackend& a, TrackList& t);

    // Ask the librarian to look in the folder. True if songs were found.
    bool openFolder(const std::string& folder);

    // Song buttons
    void next();
    void previous();
    void playSong(int n);     // play song number n (for clicking a song in a list)

    // Play buttons
    void pause();
    void resume();
    void restart();
    void forward(double seconds = 10);
    void back(double seconds = 10);
    void update();            // call again and again: starts the next song when one ends

    // Volume buttons
    void volumeUp();
    void volumeDown();

    // Favorites
    void toggleFavorite();
    bool isFavorite() const;
    int favoriteCount() const;
    std::string favorite(int n) const;        // full address
    std::string favoriteTitle(int n) const;   // just the song name

    // Recent
    int recentCount() const;
    std::string recentSong(int n) const;      // full address
    std::string recentTitle(int n) const;     // just the song name

    // Playlists            
    bool createPlaylist(const std::string& name);
    int playlistCount() const;
    std::string playlistName(int n) const;
    bool addCurrentToPlaylist(const std::string& name);
    int playlistSongCount(const std::string& name) const;
    std::string playlistSongTitle(const std::string& name, int n) const;
    int current;            // song playing now (0 = nothing started yet)
    bool paused;            // true after we press pause
    int volumePercent;      // 0 to 100
    std::string trackName;  // name of the song playing now
    std::string errorText;  // what went wrong ("" = nothing)

    // Questions the screen can ask ("const" = only looks, changes nothing)
    int volume() const;
    bool isPaused() const;
    int currentIndex() const;
    int total() const;
    double position() const;
    double duration() const;
    std::string songName() const;
    std::string lastError() const;
    std::string songTitle(int n) const;
};
