#include "playerController.h"
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;


// The DJ gets a speaker and a librarian. Nothing is playing yet. Volume is 50.
PlayerController::PlayerController(AudioBackend& a, TrackList& t)
    : audio(a), tracks(t), rng(std::random_device{}())
{
    current = 0;
    currentId = 0;
    paused = false;
    volumePercent = 50;
    trackName = "";
    errorText = "";
    pos = -1;
    anchorId = 0;
    queueName = "";
    shuffleOn = false;
    repeat = 0;
    autoplayOn = false;
    counted = false;
    playingFile = false;
}


// Tells the librarian to look in the folder. True if at least one song was found.
bool PlayerController::openFolder(const std::string& folder)
{
    tracks.scan(folder);

    // the scan forgets songs whose file is gone and numbers can move, so find the playing song again
    if (currentId != 0) {
        current = tracks.positionOfId(currentId);

        if (current == 0) {                 // its file is gone
            audio.pause();
            currentId = 0;
            paused = false;
            trackName = "";
        }
    }

    // songs that are gone leave the queue (new songs do not join it, the queue is what was clicked on)
    for (int i = (int)baseQueue.size() - 1; i >= 0; i--) {
        if (tracks.positionOfId(baseQueue[i]) == 0)
            dropFromQueue(baseQueue[i]);
    }

    for (int i = userQueue.count(); i >= 1; i--) {
        if (tracks.positionOfId(userQueue.get(i)) == 0)
            userQueue.remove(userQueue.get(i));
    }

    findPos();
    return tracks.getCount() > 0;
}


// A song id leaves the queue.
void PlayerController::dropFromQueue(int id)
{
    int place = -1;

    for (int i = 0; i < (int)order.size(); i++) {
        if (order[i] == id)
            place = i;
    }

    baseQueue.erase(std::remove(baseQueue.begin(), baseQueue.end(), id), baseQueue.end());
    order.erase(std::remove(order.begin(), order.end(), id), order.end());

    while (userQueue.remove(id)) {}

    if (place != -1 && place <= pos)
        pos--;                                  // the place of the last played song moves up with the list

    anchorId = (pos >= 0 && pos < (int)order.size()) ? order[pos] : 0;
}


// Where the queue is: the place of the song that came from it last (a "play next" song does not move it).
void PlayerController::findPos()
{
    pos = -1;

    if (anchorId == 0)
        return;

    for (int i = 0; i < (int)order.size(); i++) {
        if (order[i] == anchorId) {
            pos = i;
            return;
        }
    }
}


// Shuffles `order`. The song with firstId (the one that was clicked, or is playing) stays in front.
void PlayerController::shuffleOrder(int firstId)
{
    order = baseQueue;

    auto it = std::find(order.begin(), order.end(), firstId);
    bool hasFirst = (firstId != 0 && it != order.end());

    if (hasFirst)
        order.erase(it);

    std::shuffle(order.begin(), order.end(), rng);

    if (hasFirst)
        order.insert(order.begin(), firstId);
}


// Makes a new queue from a list of song numbers.
void PlayerController::setQueue(const std::vector<int>& songNumbers, const std::string& name, int startId)
{
    baseQueue.clear();

    for (int n : songNumbers) {
        int id = tracks.getId(n);

        if (id != 0)
            baseQueue.push_back(id);
    }

    queueName = name;

    if (shuffleOn)
        shuffleOrder(startId);
    else
        order = baseQueue;

    // the playing song is only known once it starts, so look for the clicked one
    pos = -1;
    anchorId = startId;

    for (int i = 0; i < (int)order.size(); i++) {
        if (order[i] == startId) {
            pos = i;
            break;
        }
    }
}


// Nothing was queued yet (the play button on a fresh start, or "next" before any song): use the whole library.
void PlayerController::ensureQueue()
{
    if (!order.empty())
        return;

    std::vector<int> all;

    for (int n = 1; n <= tracks.getCount(); n++)
        all.push_back(n);

    setQueue(all, "All Songs", 0);
}


// Plays order[place]. A song that does not open is skipped, moving in the direction of `step` (+1 or -1).
void PlayerController::playAt(int place, int step)
{
    int songs = (int)order.size();

    for (int tries = 0; tries < songs; tries++) {
        int id = order[place];
        int n = tracks.positionOfId(id);

        if (n != 0) {
            std::string path = tracks.getTrack(n);

            current = n;
            currentId = id;
            paused = false;
            playingFile = false;
            trackName = songTitle(n);

            if (audio.load(path)) {
                errorText = "";
                pos = place;
                anchorId = id;
                counted = false;
                audio.set_volume(volumePercent / 100.0f);
                audio.play();
                recent.add(id);      // recent remembers the id, not the path
                return;
            }

            errorText = audio.last_error();
        }

        place = (place + step + songs) % songs;
    }
}


// Plays song n. The queue becomes the whole library, like before.
void PlayerController::playSong(int n)
{
    if (n < 1 || n > tracks.getCount())
        return;

    std::vector<int> all;

    for (int i = 1; i <= tracks.getCount(); i++)
        all.push_back(i);

    playQueue(all, n - 1, "All Songs");
}


void PlayerController::playQueue(const std::vector<int>& songNumbers, int row, const std::string& name)
{
    if (row < 0 || row >= (int)songNumbers.size())
        return;

    int startId = tracks.getId(songNumbers[row]);

    if (startId == 0)
        return;

    setQueue(songNumbers, name, startId);

    if (pos >= 0)
        playAt(pos, 1);
}


// "Shuffle" on a list: shuffle goes on and the first song is picked at random.
void PlayerController::playQueueShuffled(const std::vector<int>& songNumbers, const std::string& name)
{
    if (songNumbers.empty())
        return;

    shuffleOn = true;
    int row = std::uniform_int_distribution<int>(0, (int)songNumbers.size() - 1)(rng);

    playQueue(songNumbers, row, name);
}


// Plays one song of the library right now, without moving the place in the queue
// (a song added with "Add to queue" plays "in between"). False if it will not open.
bool PlayerController::playId(int id)
{
    int n = tracks.positionOfId(id);

    if (n == 0)
        return false;

    current = n;
    currentId = id;
    paused = false;
    playingFile = false;
    trackName = songTitle(n);

    if (!audio.load(tracks.getTrack(n))) {
        errorText = audio.last_error();
        return false;
    }

    errorText = "";
    counted = false;
    audio.set_volume(volumePercent / 100.0f);
    audio.play();
    recent.add(id);
    return true;
}


// A loose file, like a recording. It is not a song of the library: no number, no favorites, no queue.
bool PlayerController::playFilePath(const std::string& path, const std::string& title)
{
    if (!audio.load(path)) {
        errorText = audio.last_error();
        return false;
    }

    current = 0;
    currentId = 0;
    paused = false;
    playingFile = true;
    trackName = title;
    errorText = "";
    counted = true;                 // recordings are not counted as plays
    audio.set_volume(volumePercent / 100.0f);
    audio.play();
    return true;
}


// The next song. First what was added with "Add to queue", then the queue itself.
// byItself: the song just ended (so repeat off may stop here). A click on "next" never stops.
void PlayerController::advance(bool byItself)
{
    while (userQueue.count() > 0) {
        if (playId(userQueue.take()))
            return;
    }

    ensureQueue();

    if (order.empty())
        return;

    int songs = (int)order.size();
    bool atEnd = (pos >= songs - 1);

    if (byItself && atEnd && repeat == 0) {
        if (autoplayOn && tracks.getCount() > 0) {          // keep going with the whole library
            std::vector<int> all;

            for (int n = 1; n <= tracks.getCount(); n++)
                all.push_back(n);

            int from = tracks.positionOfId(anchorId);
            setQueue(all, "All Songs", from != 0 ? anchorId : 0);
            playAt(((pos + 1) % (int)order.size() + (int)order.size()) % (int)order.size(), 1);
            return;
        }

        audio.pause();                                       // the queue is over
        audio.seek(0);
        paused = true;
        return;
    }

    playAt((pos + 1) % songs, 1);                            // pos -1 starts at the first song
}


void PlayerController::next()
{
    advance(false);
}


// If the song is more than 3 seconds in, start it again.
// If it just began, go to the song before.
void PlayerController::previous()
{
    ensureQueue();

    if (order.empty())
        return;

    if ((current != 0 || playingFile) && audio.position() > 3.0) {
        restart();
        return;
    }

    int songs = (int)order.size();

    if (currentId != anchorId && pos >= 0) {                 // a "play next" song plays: go back to the queue song
        playAt(pos, -1);
        return;
    }

    playAt(pos <= 0 ? songs - 1 : pos - 1, -1);              // nothing playing, or the first song: go to the last
}


void PlayerController::setShuffle(bool on)
{
    if (on == shuffleOn)
        return;

    shuffleOn = on;

    if (baseQueue.empty())
        return;

    if (on)
        shuffleOrder(anchorId);          // the song the queue is at goes to the front
    else
        order = baseQueue;               // back to the order of the list

    findPos();
}


void PlayerController::toggleShuffle()
{
    setShuffle(!shuffleOn);
}


bool PlayerController::isShuffle() const
{
    return shuffleOn;
}


void PlayerController::setRepeatMode(int mode)
{
    repeat = (mode < 0 || mode > 2) ? 0 : mode;
}


int PlayerController::repeatMode() const
{
    return repeat;
}


void PlayerController::toggleRepeat()
{
    repeat = (repeat + 1) % 3;
}


void PlayerController::setAutoplay(bool on)
{
    autoplayOn = on;
}


bool PlayerController::isAutoplay() const
{
    return autoplayOn;
}


// The songs that play next: the "Add to queue" songs first, then what is left of the queue.
std::vector<int> PlayerController::upcoming(int most) const
{
    std::vector<int> numbers;

    for (int i = 1; i <= userQueue.count() && (int)numbers.size() < most; i++) {
        int n = tracks.positionOfId(userQueue.get(i));

        if (n != 0)
            numbers.push_back(n);
    }

    int songs = (int)order.size();

    for (int i = pos + 1; i < songs && (int)numbers.size() < most; i++) {
        int n = tracks.positionOfId(order[i]);

        if (n != 0)
            numbers.push_back(n);
    }

    if (repeat == 1) {                          // the queue starts again: those songs come after
        for (int i = 0; i <= pos && i < songs && (int)numbers.size() < most; i++) {
            int n = tracks.positionOfId(order[i]);

            if (n != 0)
                numbers.push_back(n);
        }
    }

    return numbers;
}


// A click on a row of the queue panel: that song plays now. A song that was added with "Add to queue"
// leaves the line; a song of the queue itself moves the queue to it.
void PlayerController::playUpcoming(int songNumber)
{
    int id = tracks.getId(songNumber);

    if (id == 0)
        return;

    if (userQueue.remove(id)) {
        playId(id);
        return;
    }

    for (int i = 0; i < (int)order.size(); i++) {
        if (order[i] == id) {
            playAt(i, 1);
            return;
        }
    }

    playId(id);
}


void PlayerController::addToQueue(int songNumber)
{
    int id = tracks.getId(songNumber);

    if (id != 0)
        userQueue.add(id);
}


void PlayerController::removeFromQueue(int songNumber)
{
    userQueue.remove(tracks.getId(songNumber));
}


bool PlayerController::isQueued(int songNumber) const
{
    return userQueue.contains(tracks.getId(songNumber));
}


std::string PlayerController::queueContext() const
{
    return queueName;
}


std::vector<int> PlayerController::mostPlayedSongNumbers(int most) const
{
    std::vector<int> numbers;

    for (int id : counts.top(most)) {
        int n = tracks.positionOfId(id);

        if (n != 0)
            numbers.push_back(n);
    }

    return numbers;
}


// Stops the music and forgets what was queued. The favorites, playlists and so on stay, they belong to the profile.
void PlayerController::stop()
{
    audio.pause();

    current = 0;
    currentId = 0;
    paused = false;
    trackName = "";
    errorText = "";
    baseQueue.clear();
    order.clear();
    userQueue.clear();
    pos = -1;
    anchorId = 0;
    queueName = "";
    counted = false;
    playingFile = false;
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
void PlayerController::togglePlay()
{
    if (!hasSong())
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


// Checks "is the song finished?". If yes, plays it again (repeat) or the next one.
// Also counts a play once the song has been heard for 30 seconds (or half of a short song).
void PlayerController::update()
{
    if (!hasSong() || paused)
        return;                  // nothing is playing, or we paused

    double length = audio.duration();

    if (!counted && length > 0 && audio.position() >= std::min(30.0, length / 2)) {
        counts.add(currentId);
        counted = true;
    }

    // a song with length 0 failed to open, so we skip this check for it
    bool finished = length > 0 && audio.position() >= length - 0.1;

    if (!finished)
        return;

    if (playingFile) {           // a recording ended: wait at the start
        audio.pause();
        audio.seek(0);
        paused = true;
    }
    else if (repeat == 2) {
        restart();
        counted = false;         // it plays again, so it can count again
    }
    else {
        advance(true);
    }
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
    toggleFavoriteSong(current);
}


// Is the song playing now a favorite?
bool PlayerController::isFavorite() const
{
    return isFavoriteSong(current);
}


bool PlayerController::isFavoriteSong(int songNumber) const
{
    int id = tracks.getId(songNumber);

    return id != 0 && favorites.contains(id);
}


// The heart on any row.
void PlayerController::toggleFavoriteSong(int songNumber)
{
    int id = tracks.getId(songNumber);

    if (id == 0)
        return;

    if (favorites.contains(id))
        favorites.remove(id);
    else
        favorites.add(id);
}


int PlayerController::favoriteCount() const
{
    return favorites.count();
}

// The file address of favorite number n (the list holds ids, we turn it into a path).
std::string PlayerController::favorite(int n) const
{
    return tracks.getTrack(tracks.positionOfId(favorites.get(n)));
}

std::string PlayerController::favoriteTitle(int n) const
{
    return tracks.getTitle(tracks.positionOfId(favorites.get(n)));
}


int PlayerController::recentCount() const
{
    return recent.count();
}

std::string PlayerController::recentSong(int n) const
{
    return tracks.getTrack(tracks.positionOfId(recent.get(n)));
}

std::string PlayerController::recentTitle(int n) const
{
    return tracks.getTitle(tracks.positionOfId(recent.get(n)));
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


// The name of the song that comes after the one playing now.
std::string PlayerController::nextSongTitle() const
{
    std::vector<int> next = upcoming(1);

    if (!next.empty())
        return tracks.getTitle(next[0]);

    if (order.empty())
        return "";

    return tracks.getTitle(tracks.positionOfId(order[0]));       // the queue starts again
}


// When song number n was found (seconds since 1970).
long long PlayerController::songAdded(int n) const
{
    return tracks.getAdded(n);
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

bool PlayerController::hasSong() const
{
    return !trackName.empty();
}


// Playlists
bool PlayerController::createPlaylist(const std::string& name)
{
    return playlists.create(name);
}

bool PlayerController::renamePlaylist(const std::string& oldName, const std::string& newName)
{
    return playlists.rename(oldName, newName);
}

std::string PlayerController::freePlaylistName(const std::string& base) const
{
    return playlists.freeName(base);
}

std::vector<std::string> PlayerController::playlistsWithSong(int songNumber) const
{
    std::vector<std::string> names;
    int id = tracks.getId(songNumber);

    if (id == 0)
        return names;

    for (int i = 1; i <= playlists.count(); i++) {
        if (playlists.hasSong(playlists.getName(i), id))
            names.push_back(playlists.getName(i));
    }

    return names;
}

int PlayerController::playlistRowOf(const std::string& name, int songNumber) const
{
    int id = tracks.getId(songNumber);

    if (id == 0)
        return 0;

    for (int row = 1; row <= playlists.songCount(name); row++) {
        if (playlists.getSong(name, row) == id)
            return row;
    }

    return 0;
}

int PlayerController::playCount(int songNumber) const
{
    return counts.get(tracks.getId(songNumber));
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
        currentId = 0;
        paused = false;
        trackName = "";
    }
    else if (current > songNumber) {
        current--;              // the songs after the removed one move up one number
    }

    if (id != 0) {
        favorites.remove(id);
        recent.remove(id);
        counts.remove(id);
        playlists.removeSongEverywhere(id);
        dropFromQueue(id);
    }

    return tracks.removeSong(songNumber);
}

// Deletes a whole playlist (its name and its songs).
bool PlayerController::deletePlaylist(const std::string& name)
{
    return playlists.remove(name);
}

// The picture chosen for a playlist ("" if none).
std::string PlayerController::playlistArt(const std::string& name) const
{
    return playlists.art(name);
}

bool PlayerController::setPlaylistArt(const std::string& name, const std::string& imagePath)
{
    return playlists.setArt(name, imagePath);
}

int PlayerController::playlistSongCount(const std::string& name) const
{
    return playlists.songCount(name);
}

std::string PlayerController::playlistSongTitle(const std::string& name, int n) const
{
    return tracks.getTitle(tracks.positionOfId(playlists.getSong(name, n)));
}