#pragma once
#include <string>
#include "../audio/audioBackend.h"
#include "trackList.h"

// THE DJ (all the rules live here)
//
// The screen only says "next!" or "volume up!".
// The DJ decides what that means, then tells the speaker what to do.
//
// The DJ does NOT make sound (the speaker does)
// The DJ does NOT find songs   (the librarian does)
// The DJ does NOT draw or read keys (the screen does)

class PlayerController {
private:
    AudioBackend& audio;    // the speaker (the & means "the real one, not a copy")
    TrackList& tracks;      // the librarian

    int current;            // song playing now (0 = nothing started yet)
    bool paused;            // true after we press pause
    int volumePercent;      // 0 to 100
    std::string trackName;  // name of the song playing now
    std::string errorText;  // what went wrong ("" = nothing)

    void playTrack(int n);  // private: only the DJ uses this inside

public:
    PlayerController(AudioBackend& a, TrackList& t);

    // Ask the librarian to look in the folder. True if songs were found.
    bool openFolder(const std::string& folder);

    // Song buttons
    void next();
    void previous();

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
