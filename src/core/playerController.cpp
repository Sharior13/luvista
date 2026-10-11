#include "playerController.h"
#include <cstdlib>      // srand / rand: the shuffle needs random numbers
#include <ctime>        // time(): so every run of the program shuffles differently
#include <filesystem>

namespace fs = std::filesystem;


// The DJ gets a speaker and a librarian. Nothing is playing yet. Volume is 50.
PlayerController::PlayerController(AudioBackend& a, TrackList& t)
    : audio(a), tracks(t)
{
    current = 0;
    paused = false;
    volumePercent = 50;
    trackName = "";
    errorText = "";

    shuffle = false;
    repeatMode = 0;
    orderSize = 0;
    orderAt = -1;

    std::srand((unsigned int)std::time(nullptr));   // seed the random numbers (runs once)
}


// Tells the librarian to look in the folder. True if at least one song was found.
bool PlayerController::openFolder(const std::string& folder)
{
    tracks.scan(folder);
    return tracks.getCount() > 0;
}

void PlayerController::playTrack(int n, int step)
{
    int songs = tracks.getCount();

    for (int tries = 0; tries < songs; tries++) {
        std::string path = tracks.getTrack(n);

        current = n;
        paused = false;
        trackName = songTitle(n);

        if (audio.load(path)) {
            errorText = "";
            audio.set_volume(volumePercent / 100.0f);
            audio.play();
            recent.add(tracks.getId(n));     // recent remembers the id, not the path
            orderAt = placeInOrder(n);       // the shuffle finger follows the song playing
            return;
        }

        errorText = audio.last_error();
        n = (n - 1 + step + songs) % songs + 1;
    }

    // every try failed: nothing is playing after all
    current = 0;
    trackName = "";
}


void PlayerController::playSong(int n)
{
    if (n < 1 || n > tracks.getCount())
        return;

    playTrack(n, 1);
}


// Plays a file that is NOT in the library (a recording from the recordings folder).
// current stays 0 so the player knows it is not a library song: no auto-next,
// no favorite, no queue - but pause / resume / seek / volume still work.
bool PlayerController::playFilePath(const std::string& path, const std::string& name)
{
    if (audio.load(path)) {
        errorText = "";
        audio.set_volume(volumePercent / 100.0f);
        audio.play();
        current = 0;
        paused = false;
        trackName = name;
        return true;
    }

    errorText = audio.last_error();
    return false;
}


void PlayerController::next()
{
    int songs = tracks.getCount();

    if (songs == 0)
        return;

    // 1) the queue wins: play the first waiting song
    int id = queue.take();

    while (id != 0) {
        int n = tracks.positionOfId(id);

        if (n != 0) {
            playTrack(n, 1);
            return;
        }

        id = queue.take();     // this song was removed from the library: skip it
    }

    // 2) shuffle: the next place in the shuffled order
    if (shuffle) {
        if (orderSize != songs)
            buildShuffleOrder();            // the library changed size since last time

        orderAt++;

        if (orderAt >= orderSize)
            orderAt = 0;                  // heard them all: the same order starts again

        playTrack(order[orderAt], 1);
        return;
    }

    // 3) normal: the song after this one
    playTrack(current % songs + 1, 1);
}

// If the song is more than 3 seconds in, start it again.
// If it just began, go to the song before.
void PlayerController::previous()
{
    int songs = tracks.getCount();

    if (songs == 0)
        return;

    if (current != 0 && audio.position() > 3.0) {
        restart();
        return;
    }

    // shuffle: one step back in the shuffled order (wraps to the end)
    if (shuffle) {
        if (orderSize != songs)
            buildShuffleOrder();            // the library changed size since last time

        orderAt--;

        if (orderAt < 0)
            orderAt = orderSize - 1;

        playTrack(order[orderAt], -1);
        return;
    }

    if (current == 0)
        playTrack(songs, -1);
    else
        playTrack((current - 2 + songs) % songs + 1, -1);
}


// Pause button.
void PlayerController::pause()
{
    audio.pause();
    paused = true;
}


// Resume button.
void PlayerController::resume()
{
    audio.play();
    paused = false;
}


// One play/pause button: pauses if playing, plays if paused.
// Works for library songs AND for recordings (something must be loaded).
void PlayerController::togglePlay()
{
    if (trackName == "")
        return;

    if (paused)
        resume();
    else
        pause();
}


// Go back to the start of the song and play it.
void PlayerController::restart()
{
    audio.seek(0);
    audio.play();
    paused = false;
}


// Jump ahead, but never past the end of the song.
void PlayerController::forward(double seconds)
{
    double newPlace = audio.position() + seconds;

    if (newPlace > audio.duration())
        newPlace = audio.duration();

    audio.seek(newPlace);
}


// Jump back, but never before the start of the song.
void PlayerController::back(double seconds)
{
    double newPlace = audio.position() - seconds;

    if (newPlace < 0)
        newPlace = 0;

    audio.seek(newPlace);
}


// Jump to any second (the seek bar). Stays inside the song.
void PlayerController::seekTo(double seconds)
{
    if (seconds < 0)
        seconds = 0;

    if (seconds > audio.duration())
        seconds = audio.duration();

    audio.seek(seconds);
}


// Checks "is the song finished?". If yes, carries on according to the repeat mode.
void PlayerController::update()
{
    if (current == 0 || paused)
        return;                  // nothing is playing, or we paused

    // a song with length 0 failed to open, so we skip this check for it
    bool finished = audio.duration() > 0 &&
        audio.position() >= audio.duration() - 0.1;

    if (!finished)
        return;

    // the queue always wins, whatever the repeat mode says
    if (queue.count() > 0) {
        next();
        return;
    }

    if (repeatMode == 2) {       // repeat ONE: the same song again
        restart();
        return;
    }

    if (repeatMode == 0 && atEndOfLibrary()) {   // repeat OFF: the library ends here
        audio.pause();
        audio.seek(0);           // so pressing Resume plays it from the top
        paused = true;
        return;
    }

    next();                      // repeat ALL (or still songs to go): carry on
}


// Volume up by 10, but never above 100.
void PlayerController::volumeUp()
{
    setVolume(volumePercent + 10);
}


// Volume down by 10, but never below 0.
void PlayerController::volumeDown()
{
    setVolume(volumePercent - 10);
}


// Sets the volume to any number from 0 to 100 (the volume slider).
void PlayerController::setVolume(int percent)
{
    if (percent > 100)
        percent = 100;

    if (percent < 0)
        percent = 0;

    volumePercent = percent;
    audio.set_volume(volumePercent / 100.0f);
}


// Heart button: adds the song to favorites, or removes it if it is already there.
void PlayerController::toggleFavorite()
{
    if (current == 0)
        return;

    int id = tracks.getId(current);

    if (favorites.contains(id))
        favorites.remove(id);
    else
        favorites.add(id);
}


// Is the song playing now a favorite?
bool PlayerController::isFavorite() const
{
    if (current == 0)
        return false;

    return favorites.contains(tracks.getId(current));
}


int PlayerController::favoriteCount() const
{
    return favorites.count();
}

int PlayerController::recentCount() const
{
    return recent.count();
}


// The song name without the folder and without ".mp3".
std::string PlayerController::songTitle(int n) const
{
    return tracks.getTitle(n);
}


std::string PlayerController::songArtist(int n) const
{
    return tracks.getArtist(n);
}


std::string PlayerController::songAlbum(int n) const
{
    return tracks.getAlbum(n);
}


int PlayerController::favoriteSongNumber(int row) const
{
    return tracks.positionOfId(favorites.get(row));
}


int PlayerController::recentSongNumber(int row) const
{
    return tracks.positionOfId(recent.get(row));
}


int PlayerController::playlistSongNumber(const std::string& name, int row) const
{
    return tracks.positionOfId(playlists.getSong(name, row));
}


// The song that plays after this one: the queue first, then the shuffled
// order, then the plain "next song in the list" answer.
std::string PlayerController::nextSongTitle() const
{
    int songs = tracks.getCount();

    if (songs == 0)
        return "";

    for (int row = 1; row <= queue.count(); row++) {
        int n = tracks.positionOfId(queue.get(row));

        if (n != 0)                       // the first queued song that still exists
            return tracks.getTitle(n);
    }

    if (shuffle && orderSize > 0) {
        int after = orderAt + 1;

        if (after >= orderSize)
            after = 0;                    // the round starts again

        return tracks.getTitle(order[after]);
    }

    return tracks.getTitle(current % songs + 1);
}


// When song number n was added (seconds since 1970). For the "Date added" column.
long long PlayerController::songAdded(int n) const
{
    return tracks.getAdded(n);
}


// Builds a new random way through the library: order[] holds 1..N, shuffled once.
// The trick (Fisher-Yates): walk backwards and swap each place with a random
// place that is still to come. Every song lands exactly once somewhere.
void PlayerController::buildShuffleOrder()
{
    int songs = tracks.getCount();

    if (songs > TrackList::capacity)
        songs = TrackList::capacity;     // never fill more than the array holds

    orderSize = songs;

    for (int i = 0; i < songs; i++)
        order[i] = i + 1;                // start in normal order: 1, 2, 3, ...

    for (int i = songs - 1; i > 0; i--) {
        int j = std::rand() % (i + 1);   // a random place from 0 to i

        int temp = order[i];
        order[i] = order[j];
        order[j] = temp;
    }
}


// Where song number n sits in the shuffled order. -1 = it is not in there.
int PlayerController::placeInOrder(int n) const
{
    for (int i = 0; i < orderSize; i++) {
        if (order[i] == n)
            return i;
    }

    return -1;
}


// Puts a song at the end of the queue: it plays after the ones already waiting.
bool PlayerController::addToQueue(int songNumber)
{
    if (songNumber < 1 || songNumber > tracks.getCount())
        return false;

    int before = queue.count();
    queue.add(tracks.getId(songNumber));
    return queue.count() > before;       // false when the queue is full
}


// The queue becomes empty.
void PlayerController::clearQueue()
{
    queue.clear();
}


int PlayerController::queueCount() const
{
    return queue.count();
}


// Queue row -> song number in the library (0 when the song was removed).
int PlayerController::queueSongNumber(int row) const
{
    return tracks.positionOfId(queue.get(row));
}


std::string PlayerController::queueTitle(int row) const
{
    return tracks.getTitle(tracks.positionOfId(queue.get(row)));
}


// Turns shuffle on / off.
void PlayerController::toggleShuffle()
{
    shuffle = !shuffle;

    if (shuffle) {
        buildShuffleOrder();
        orderAt = placeInOrder(current);   // start the shuffle after the song playing now
    }
}


bool PlayerController::isShuffleOn() const
{
    return shuffle;
}


// Turns repeat to the next mode: OFF -> ALL -> ONE -> OFF.
void PlayerController::toggleRepeat()
{
    repeatMode = (repeatMode + 1) % 3;
}


int PlayerController::repeat() const
{
    return repeatMode;
}


// Says yes when the next next() would wrap around: in shuffle mode that is the
// last place of the round, otherwise the last song of the library.
bool PlayerController::atEndOfLibrary() const
{
    int songs = tracks.getCount();

    if (songs == 0)
        return true;

    if (shuffle && orderSize > 0)
        return orderAt == orderSize - 1;

    return current == songs;
}


// Search
int PlayerController::findSong(const std::string& query, int nth) const
{
    int songs = tracks.getCount();
    int seen = 0;

    for (int i = 1; i <= songs; i++) {
        if (tracks.matches(i, query)) {
            seen++;

            if (seen == nth)
                return i;
        }
    }

    return 0;
}


int PlayerController::findFavorite(const std::string& query, int nth) const
{
    int rows = favorites.count();
    int seen = 0;

    for (int i = 1; i <= rows; i++) {
        if (tracks.matches(tracks.positionOfId(favorites.get(i)), query)) {
            seen++;

            if (seen == nth)
                return i;
        }
    }

    return 0;
}


int PlayerController::findRecent(const std::string& query, int nth) const
{
    int rows = recent.count();
    int seen = 0;

    for (int i = 1; i <= rows; i++) {
        if (tracks.matches(tracks.positionOfId(recent.get(i)), query)) {
            seen++;

            if (seen == nth)
                return i;
        }
    }

    return 0;
}


int PlayerController::findInPlaylist(const std::string& name, const std::string& query, int nth) const
{
    int rows = playlists.songCount(name);
    int seen = 0;

    for (int i = 1; i <= rows; i++) {
        if (tracks.matches(tracks.positionOfId(playlists.getSong(name, i)), query)) {
            seen++;

            if (seen == nth)
                return i;
        }
    }

    return 0;
}


// Small "questions" the screen can ask. Each just gives back something.
int PlayerController::volume() const
{
    return volumePercent;
}

bool PlayerController::isPaused() const
{
    return paused;
}

int PlayerController::currentIndex() const
{
    return current;
}

int PlayerController::total() const
{
    return tracks.getCount();
}

double PlayerController::position() const
{
    return audio.position();
}

double PlayerController::duration() const
{
    return audio.duration();
}

std::string PlayerController::songName() const
{
    return trackName;
}

std::string PlayerController::lastError() const
{
    return errorText;
}

// Playlists
bool PlayerController::createPlaylist(const std::string& name)
{
    return playlists.create(name);
}

int PlayerController::playlistCount() const
{
    return playlists.count();
}

std::string PlayerController::playlistName(int n) const
{
    return playlists.getName(n);
}

// Puts the song that is playing now into the playlist.
bool PlayerController::addCurrentToPlaylist(const std::string& name)
{
    if (current == 0)
        return false;

    return playlists.addSong(name, tracks.getId(current));
}

// Puts song number n (any row of the table) into the playlist.
bool PlayerController::addToPlaylist(const std::string& name, int songNumber)
{
    return playlists.addSong(name, tracks.getId(songNumber));
}

// Takes the song that is playing now out of the playlist.
bool PlayerController::removeCurrentFromPlaylist(const std::string& name)
{
    if (current == 0)
        return false;

    return playlists.removeSong(name, tracks.getId(current));
}

// Takes song row n (1, 2, 3 ...) out of the playlist. The song stays in the library.
bool PlayerController::removeFromPlaylistRow(const std::string& name, int row)
{
    if (row < 1)
        return false;

    return playlists.removeSong(name, playlists.getSong(name, row));
}

// Empties the whole favorites list.
void PlayerController::clearFavorites()
{
    favorites.clear();
}

// Forgets a song everywhere: the library list, favorites, recents and every playlist.
// The music file itself stays on disk and can be found again by scanning the folder.
bool PlayerController::removeSongFromLibrary(int songNumber)
{
    if (songNumber < 1 || songNumber > tracks.getCount())
        return false;

    int id = tracks.getId(songNumber);

    // stop the song first if it is the one playing
    if (current == songNumber) {
        audio.pause();
        current = 0;
        paused = false;
        trackName = "";
    }
    else if (current > songNumber) {
        current--;              // the songs after the removed one move up one number
    }

    if (id != 0) {
        favorites.remove(id);
        recent.remove(id);
        playlists.removeSongEverywhere(id);
    }

    return tracks.removeSong(songNumber);
}

// Deletes a whole playlist (its name and its songs).
bool PlayerController::deletePlaylist(const std::string& name)
{
    return playlists.remove(name);
}

int PlayerController::playlistSongCount(const std::string& name) const
{
    return playlists.songCount(name);
}

std::string PlayerController::playlistSongTitle(const std::string& name, int n) const
{
    return tracks.getTitle(tracks.positionOfId(playlists.getSong(name, n)));
}