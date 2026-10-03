#pragma once
#include <string>
#include "../audio/audioBackend.h"
#include "trackList.h"

// PlayerController is like a DJ.
// It owns all the music-player rules: which song is next, how loud it is,
// how far we may jump forward... The screen only asks it to do things.
// It knows the AudioBackend CONTRACT, never miniaudio itself.
class PlayerController {
private:
    AudioBackend& audio;    // the speaker (any speaker that keeps the contract)
    TrackList& tracks;      // the list of songs

    int current;            // song playing now (0 = none yet)
    bool paused;            // true if we pressed pause
    int volumePercent;      // 0 to 100
    std::string trackName;  // name of the current song
    std::string errorText;  // "" if the last action worked

    void playTrack(int n);

public:
    PlayerController(AudioBackend& a, TrackList& t);

    // Looks for songs in a folder. Says true if at least one song was found.
    bool openFolder(const std::string& folder);

    // Song buttons
    void next();            // next song (after the last one, goes to the first)
    void previous();        // song before (before the first, goes to the last)

    // Play buttons
    void pause();
    void resume();
    void restart();
    void forward(double seconds = 10);   // jump ahead, never past the end
    void back(double seconds = 10);      // jump back, never before the start

    // Volume buttons (steps of 10, between 0 and 100)
    void volumeUp();
    void volumeDown();

    // Questions the screen can ask
    int volume() const;
    bool isPaused() const;
    int currentIndex() const;
    int total() const;
    double position() const;
    double duration() const;
    std::string songName() const;
    std::string lastError() const;
};
