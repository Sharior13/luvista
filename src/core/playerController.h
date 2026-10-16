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
#include "PlayQueue.h"

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
    int pos;                        // place in `order` of the last song that came from the queue, -1 = none
    int anchorId;                   // the id of that song: where the queue carries on after a "play next" song
    std::string queueName;          // "All Songs", "Favorites", a playlist name ...
    bool shuffleOn;
    int repeat;                     // 0 = off, 1 = the queue starts again at its end, 2 = the song plays again
    bool autoplayOn;                // off + queue over: keep going with the whole library
    bool counted;                   // this play was already added to the play counts
    bool playingFile;               // a loose file (a recording) plays, not a song of the library
    PlayQueue userQueue;            // "Add to queue": these songs play before the rest of the queue
    std::mt19937 rng;

    void playAt(int place, int step);                                   // plays order[place]. Skips songs that will not open
    bool playId(int id);                                                // plays a song without touching the place in the queue
    void advance(bool byItself);                                        // next song. byItself = the song just ended
    void setQueue(const std::vector<int>& songNumbers, const std::string& name, int startId);
    void shuffleOrder(int firstId);                                     // shuffles `order`, with firstId (if any) kept at the front
    void ensureQueue();                                                 // no queue yet: use the whole library
    void findPos();                                                     // works out `pos` again from currentId
    void dropFromQueue(int id);

public:
    PlayerController(AudioBackend& a, TrackList& t);

    bool openFolder(const std::string& folder, bool bringBackRemoved = false);   // bringBackRemoved: the user picked this folder on purpose
    void libraryChanged();              // after songs were added or removed behind the player's back (a scan was applied)
    void forgetSongsInside(const std::string& folder, const std::vector<std::string>& keepFolders);
    void bringBackRemovedInside(const std::string& folder);   // songs removed from this folder earlier may be found again by the next scan

    void next();
    void previous();
    void playSong(int n);               // plays song n, and the queue becomes the whole library

    // Play a list. `songNumbers` is the list as the screen shows it, `row` the one that was clicked
    // (0 = first), `name` is shown above the queue ("Favorites"). The rest of the list becomes the queue.
    void playQueue(const std::vector<int>& songNumbers, int row, const std::string& name);
    void playQueueShuffled(const std::vector<int>& songNumbers, const std::string& name);   // shuffle on, random first song
    bool playFilePath(const std::string& path, const std::string& title);   // plays a loose file (a recording). false if it will not open

    void setShuffle(bool on);           // reorders the queue, the playing song stays where it is
    bool isShuffle() const;
    void toggleShuffle();
    void setRepeatMode(int mode);       // 0 = off, 1 = all (the queue starts again), 2 = one (the song plays again)
    int repeatMode() const;
    void toggleRepeat();                // off -> all -> one -> off
    void setAutoplay(bool on);          // when the queue is over (repeat off): go on with the whole library
    bool isAutoplay() const;

    // What plays next: the songs added with "Add to queue" first, then the rest of the queue. At most `most` song numbers.
    std::vector<int> upcoming(int most) const;
    void playUpcoming(int songNumber);  // plays a song of upcoming() right now
    void addToQueue(int songNumber);    // "Add to queue"
    void removeFromQueue(int songNumber);
    bool isQueued(int songNumber) const;
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
    bool isFavoriteSong(int songNumber) const;
    void toggleFavoriteSong(int songNumber);   // the heart on a row
    int favoriteCount() const;
    std::string favorite(int n) const;
    std::string favoriteTitle(int n) const;

    int recentCount() const;
    std::string recentSong(int n) const;
    std::string recentTitle(int n) const;

    bool createPlaylist(const std::string& name);
    bool renamePlaylist(const std::string& oldName, const std::string& newName);
    std::string freePlaylistName(const std::string& base) const;
    std::vector<std::string> playlistsWithSong(int songNumber) const;   // the playlists that hold this song
    int playlistRowOf(const std::string& name, int songNumber) const;   // its row in the playlist (first is 1), 0 if it is not in it
    int playCount(int songNumber) const;
    int playlistCount() const;
    std::string playlistName(int n) const;
    bool addCurrentToPlaylist(const std::string& name);
    bool addToPlaylist(const std::string& name, int songNumber);   // NEW: the "+" on a row
    bool removeCurrentFromPlaylist(const std::string& name);       // takes the playing song out of a playlist
    bool removeFromPlaylistRow(const std::string& name, int row);  // takes song row n out of a playlist
    bool deletePlaylist(const std::string& name);                  // deletes a whole playlist
    void clearFavorites();                                         // empties the favorites list
    bool removeSongFromLibrary(int songNumber, bool hide = true);                    // forgets a song (the file stays on disk)
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

    std::string nextSongTitle() const;          // "Next in queue" card
    long long songAdded(int n) const;           // "Date added" column
    bool hasSong() const;                       // something is loaded: a song or a recording
};