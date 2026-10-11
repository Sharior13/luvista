#pragma once
#include <string>
#include "../audio/audioBackend.h"
#include "trackList.h"
#include "favorites.h"
#include "recentList.h"
#include "playlist.h"
#include "PlayQueue.h"

class PlayerController {
private:
    AudioBackend& audio;
    TrackList& tracks;
    Favorite favorites;
    RecentList recent;
    Playlists playlists;
    PlayQueue queue;             // the songs waiting to play next

    int current;
    bool paused;
    int volumePercent;
    std::string trackName;
    std::string errorText;

    // Shuffle: one fixed, random way through the library.
    // order[] holds the song numbers 1..N in shuffled order,
    // orderAt is our place in that order (orderAt = -1 means "not started yet").
    bool shuffle;
    int repeatMode;                  // 0 = off, 1 = all, 2 = one
    int order[TrackList::capacity];
    int orderSize;               // how many songs are in that order
    int orderAt;                 // where we are in that order

    void playTrack(int n, int step);
    void buildShuffleOrder();    // makes a new random way through the library
    int  placeInOrder(int n) const;   // where song n sits in that order (-1 = not there)
    bool atEndOfLibrary() const;      // the next next() would wrap around

public:
    PlayerController(AudioBackend& a, TrackList& t);

    bool openFolder(const std::string& folder);

    void next();
    void previous();
    void playSong(int n);
    bool playFilePath(const std::string& path, const std::string& name);   // a file outside the library (a recording)

    void pause();
    void resume();
    void togglePlay();                  
    void restart();
    void forward(double seconds = 10);
    void back(double seconds = 10);
    void seekTo(double seconds);        // the seek bar: jump to any second
    void update();

    void volumeUp();
    void volumeDown();
    void setVolume(int percent);       

    // Queue: the line of songs waiting to play next (the queue wins over shuffle).
    bool addToQueue(int songNumber);      // put a song at the end of the line
    void clearQueue();                    // the line becomes empty
    int  queueCount() const;              // how many songs are still waiting
    int  queueSongNumber(int row) const;  // song number of queue row (0 = gone from library)
    std::string queueTitle(int row) const;

    // Shuffle: on = walk through all songs in a random order, each one once.
    void toggleShuffle();
    bool isShuffleOn() const;

    // Repeat: 0 = off (stops at the end), 1 = all (wraps around), 2 = one (same song again).
    void toggleRepeat();
    int  repeat() const;

    void toggleFavorite();
    bool isFavorite() const;
    int favoriteCount() const;

    int recentCount() const;

    bool createPlaylist(const std::string& name);
    int playlistCount() const;
    std::string playlistName(int n) const;
    bool addCurrentToPlaylist(const std::string& name);
    bool addToPlaylist(const std::string& name, int songNumber);   // the "+" on a row
    bool removeCurrentFromPlaylist(const std::string& name);       // takes the playing song out of a playlist
    bool removeFromPlaylistRow(const std::string& name, int row);  // takes song row n out of a playlist
    bool deletePlaylist(const std::string& name);                  // deletes a whole playlist
    void clearFavorites();                                         // empties the favorites list
    bool removeSongFromLibrary(int songNumber);                    // forgets a song (the file stays on disk)
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
    std::string songArtist(int n) const;        // the artist of song number n
    std::string songAlbum(int n) const;         // the album of song number n

    // From a row of a list to a song number (0 if there is no such row).
    // Use with songTitle / songArtist / songAlbum.
    int favoriteSongNumber(int row) const;
    int recentSongNumber(int row) const;
    int playlistSongNumber(const std::string& name, int row) const;

    std::string nextSongTitle() const;          // "Next:" line: queue first, then shuffle, then the list
    long long songAdded(int n) const;           // "Date added" column
};
