#pragma once
#include <string>
#include "audioBackend.h"
#include "miniaudio.h"

// THE REAL SPEAKER
//
// This is the ONLY class that uses the miniaudio library.
// "override" means: "I am keeping one of the promises from AudioBackend".

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
