#include "playerController.h"
#include <filesystem>

using namespace std;
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
bool PlayerController::openFolder(const string& folder)
{
    tracks.scan(folder);
    return tracks.getCount() > 0;
}


// Plays song number n:
//   1. get its address from the librarian
//   2. give the address to the speaker
//   3. press play
// If the song cannot be opened, we still move to it, so "next" never gets stuck.
void PlayerController::playTrack(int n)
{
    string path = tracks.getTrack(n);

    current = n;
    paused = false;
    trackName = songTitle(n);

    if (audio.load(path)) {
        errorText = "";
        audio.set_volume(volumePercent / 100.0f);
        audio.play();

        recent.add(path);
    }
    else {
        errorText = audio.last_error();
    }
}


// Next song. After the last song it goes back to song 1.
void PlayerController::next()
{
    int songs = tracks.getCount();

    if (songs == 0)
        return;

    playTrack(current % songs + 1);
}


// Song before. Before song 1 it jumps to the last song.
void PlayerController::previous()
{
    int songs = tracks.getCount();

    if (songs == 0)
        return;

    playTrack((current - 2 + songs) % songs + 1);
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
    volumePercent += 10;

    if (volumePercent > 100)
        volumePercent = 100;

    audio.set_volume(volumePercent / 100.0f);
}


// Volume down by 10, but never below 0.
void PlayerController::volumeDown()
{
    volumePercent -= 10;

    if (volumePercent < 0)
        volumePercent = 0;

    audio.set_volume(volumePercent / 100.0f);
}

void PlayerController::toggleFavorite()
{
    if (current == 0)
        return;

    std::string path = tracks.getTrack(current);

    if (favorites.contains(path))
        favorites.remove(path);
    else
        favorites.add(path);
}

bool PlayerController::isFavorite() const
{
    if (current == 0)
        return false;

    return favorites.contains(tracks.getTrack(current));
}

string PlayerController::songTitle(int n) const
{
    return fs::path(tracks.getTrack(n)).stem().string();
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

int PlayerController::favoriteCount() const
{
    return favorites.count();
}

std::string PlayerController::favorite(int n) const
{
    return favorites.get(n);
}

string PlayerController::songName() const
{
    return trackName;
}

string PlayerController::lastError() const
{
    return errorText;
}

int PlayerController::recentCount() const
{
    return recent.count();
}

std::string PlayerController::recentSong(int n) const
{
    return recent.get(n);
}