#include "playerController.h"
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
            return;
        }

        errorText = audio.last_error();
        n = (n - 1 + step + songs) % songs + 1;
    }
}


void PlayerController::playSong(int n)
{
    if (n < 1 || n > tracks.getCount())
        return;

    playTrack(n, 1);
}


void PlayerController::next()
{
    int songs = tracks.getCount();

    if (songs == 0)
        return;

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
void PlayerController::togglePlay()
{
    if (current == 0)
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


// Checks "is the song finished?". If yes, plays the next one.
void PlayerController::update()
{
    if (current == 0 || paused)
        return;                  // nothing is playing, or we paused

    // a song with length 0 failed to open, so we skip this check for it
    bool finished = audio.duration() > 0 &&
        audio.position() >= audio.duration() - 0.1;

    if (finished)
        next();
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
    int songs = tracks.getCount();

    if (songs == 0)
        return "";

    return tracks.getTitle(current % songs + 1);
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

int PlayerController::playlistSongCount(const std::string& name) const
{
    return playlists.songCount(name);
}

std::string PlayerController::playlistSongTitle(const std::string& name, int n) const
{
    return tracks.getTitle(tracks.positionOfId(playlists.getSong(name, n)));
}