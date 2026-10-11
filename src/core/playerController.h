#pragma once
#include <random>
#include <string>
#include <vector>
#include "../audio/audioBackend.h"
#include "trackList.h"
#include "favorites.h"
#include "recentlist.h"
#include "playlist.h"
#include "playcounts.h"

class PlayerController {
private:
    AudioBackend& audio;
    TrackList& tracks;
    Favorite favorites;
    RecentList recent;
    Playlists playlists;
    PlayCounts counts;

    int current;                    // song number of the playing song, 0 = none
    int currentId;                  // its id. Numbers can move when the library changes, ids never do
    bool paused;
    int volumePercent;
    std::string trackName;
    std::string errorText;

    // The queue. It holds song ids, so it survives the library changing under it.
    std::vector<int> baseQueue;     // the songs in the order of the list they came from
    std::vector<int> order;         // the same songs in the order they will play (shuffled if shuffle is on)
    int pos;                        // place of the playing song in `order`, -1 = none
    std::string queueName;          // "All Songs", "Favorites", a playlist name ...
    bool shuffleOn;
    bool repeatOne;
    bool counted;                   // this play was already added to the play counts
    std::mt19937 rng;

    void playAt(int place, int step);                                   // plays order[place]. Skips songs that will not open
    void setQueue(const std::vector<int>& songNumbers, const std::string& name, int startId);
    void shuffleOrder(int firstId);                                     // shuffles `order`, with firstId (if any) kept at the front
    void ensureQueue();                                                 // no queue yet: use the whole library
    void findPos();                                                     // works out `pos` again from currentId
    void dropFromQueue(int id);

public:
    PlayerController(AudioBackend& a, TrackList& t);

    bool openFolder(const std::string& folder);

    void next();
    void previous();
    void playSong(int n);               // plays song n, and the queue becomes the whole library

    // Play a list. `songNumbers` is the list as the screen shows it, `row` the one that was clicked
    // (0 = first), `name` is shown above the queue ("Favorites"). The rest of the list becomes the queue.
    void playQueue(const std::vector<int>& songNumbers, int row, const std::string& name);
    void playQueueShuffled(const std::vector<int>& songNumbers, const std::string& name);   // shuffle on, random first song
    void playQueueRow(int row);         // a row of the queue panel (0 = first)

    void setShuffle(bool on);           // reorders the queue, the playing song stays where it is
    bool isShuffle() const;
    void setRepeat(bool on);            // true = the song plays again when it ends (it does not loop on "next")
    bool isRepeat() const;

    int queueCount() const;             // songs in the queue, in play order, the playing one included
    int queueSongNumber(int row) const; // song number of a queue row (0 if that song is gone)
    int queuePosition() const;          // row of the playing song, -1 if nothing is playing
    std::string queueContext() const;   // where the queue came from, "" if there is no queue

    std::vector<int> mostPlayedSongNumbers(int most) const;   // the most played songs, top first

    void stop();                        // stops the music and forgets the queue (logging out)

    void pause();
    void resume();
    void togglePlay();                  
    void restart();
    void forward(double seconds = 10);
    void back(double seconds = 10);
    void seekTo(double seconds);        
    void update();

    void volumeUp();
    void volumeDown();
    void setVolume(int percent);       

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
    bool removeCurrentFromPlaylist(const std::string& name);       // takes the playing song out of a playlist
    bool removeFromPlaylistRow(const std::string& name, int row);  // takes song row n out of a playlist
    bool deletePlaylist(const std::string& name);                  // deletes a whole playlist
    void clearFavorites();                                         // empties the favorites list
    bool removeSongFromLibrary(int songNumber);                    // forgets a song (the file stays on disk)
    std::string playlistArt(const std::string& name) const;                    // the picture file chosen for a playlist, "" if none
    bool setPlaylistArt(const std::string& name, const std::string& imagePath); // copies a png/jpg in as the playlist's picture
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

    std::string nextSongTitle() const;          // NEW: "Next in queue" card
    long long songAdded(int n) const;           // NEW: "Date added" column
};