#pragma once
#include <string>
#include "audioBackend.h"
#include "miniaudio.h"

class MiniaudioBackend : public AudioBackend {
private:
    ma_engine engine;        // the machine that makes sound
    ma_sound sound;          // the song that is open right now
    bool engineReady;        // true if the machine started OK
    bool songLoaded;         // true if a song is open
    float volume;            // volume knob: 0.0 to 1.0
    std::string errorText;   // last problem ("" = no problem)

public:
    MiniaudioBackend();
    ~MiniaudioBackend();

    // No copying allowed: the object holds a live audio engine by value.
    // A copy would share the same engine memory, and two destructors
    // shutting it down = crash. (This turns the crash into a compile error.)
    MiniaudioBackend(const MiniaudioBackend&) = delete;
    MiniaudioBackend& operator=(const MiniaudioBackend&) = delete;

    bool load(const std::string& path) override;
    void play() override;
    void pause() override;
    void seek(double seconds) override;
    void set_volume(float v) override;

    double position() override;
    double duration() override;
    bool is_playing() override;
    std::string last_error() override;
};
