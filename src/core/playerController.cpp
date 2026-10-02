#include "playerController.h"
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;


PlayerController::PlayerController(AudioBackend& a, TrackList& t)
    : audio(a), tracks(t)
{
    current = 0;
    paused = false;
    volumePercent = 50;
    trackName = "";
    errorText = "";
}

// Scans the folder. True if at least one song was found.
bool PlayerController::openFolder(const string& folder)
{
    tracks.scan(folder);
    return tracks.getCount() > 0;
}

// Asks the list for song n, gives it to the speaker and presses play.
// If the song cannot open, we still move to it, so "next" is never stuck.
void PlayerController::playTrack(int n)
{
    string path = tracks.getTrack(n);

    current = n;
    paused = false;
    trackName = fs::path(path).stem().string();

    if (audio.load(path)) {
        errorText = "";
        audio.set_volume(volumePercent / 100.0f);
        audio.play();
    }
    else {
        errorText = audio.last_error();
    }
}

void PlayerController::next()
{
    int n = tracks.getCount();

    if (n == 0)
        return;

    playTrack(current % n + 1);
}

void PlayerController::previous()
{
    int n = tracks.getCount();

    if (n == 0)
        return;

    playTrack((current - 2 + n) % n + 1);
}

void PlayerController::pause()
{
    audio.pause();
    paused = true;
}

void PlayerController::resume()
{
    audio.play();
    paused = false;
}

// Back to the start of the song, and play it
void PlayerController::restart()
{
    audio.seek(0);
    audio.play();
    paused = false;
}

// Jumps ahead, but stops at the end of the song
void PlayerController::forward(double seconds)
{
    double pos = audio.position() + seconds;

    if (pos > audio.duration())
        pos = audio.duration();

    audio.seek(pos);
}

// Jumps back, but stops at the start of the song
void PlayerController::back(double seconds)
{
    double pos = audio.position() - seconds;

    if (pos < 0)
        pos = 0;

    audio.seek(pos);
}

void PlayerController::volumeUp()
{
    volumePercent += 10;

    if (volumePercent > 100)
        volumePercent = 100;

    audio.set_volume(volumePercent / 100.0f);
}

void PlayerController::volumeDown()
{
    volumePercent -= 10;

    if (volumePercent < 0)
        volumePercent = 0;

    audio.set_volume(volumePercent / 100.0f);
}

int PlayerController::volume() const        { return volumePercent; }
bool PlayerController::isPaused() const     { return paused; }
int PlayerController::currentIndex() const  { return current; }
int PlayerController::total() const         { return tracks.getCount(); }
double PlayerController::position() const   { return audio.position(); }
double PlayerController::duration() const   { return audio.duration(); }
string PlayerController::songName() const   { return trackName; }
string PlayerController::lastError() const  { return errorText; }
