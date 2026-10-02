#pragma once
#include <string>
#include "audioBackend.h"
#include "miniaudio.h"

// MiniaudioBackend is the real speaker.
// It is the ONLY class that talks to the miniaudio library.
class MiniaudioBackend : public AudioBackend {
private:
    ma_engine engine;        // the machine that makes sound
    ma_sound sound;          // the song that is open now
    bool engineReady;        // true if the machine started fine
    bool songLoaded;         // true if a song is open
    float volume;            // the knob: 0.0 to 1.0
    std::string errorText;   // what went wrong last time ("" = nothing)

public:
    MiniaudioBackend();
    ~MiniaudioBackend();

    // a speaker cannot be copied
    MiniaudioBackend(const MiniaudioBackend&) = delete;
    MiniaudioBackend& operator=(const MiniaudioBackend&) = delete;

    bool load(const std::string& path) override;
    void play() override;
    void pause() override;
    void seek(double seconds) override;
    void set_volume(float v) override;

    double position() const override;
    double duration() const override;
    bool is_playing() const override;
    std::string last_error() const override;
};
