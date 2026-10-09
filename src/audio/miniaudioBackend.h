#pragma once
#include <string>
#include "audioBackend.h"
#include "miniaudio.h"

// Playback on miniaudio: mp3/wav/flac open directly, m4a is decoded into memory first.
class MiniaudioBackend : public AudioBackend {
private:
    ma_engine engine;        // the sound engine
    ma_sound sound;          // the open song
    bool engineReady;        // engine started OK
    bool songLoaded;         // a song is open
    float volume;            // 0.0 - 1.0
    std::string errorText;   // last problem ("" = none)

    // m4a: whole song decoded into this memory buffer (miniaudio cannot read m4a itself)
    ma_audio_buffer audioBuffer;
    ma_int16* m4aData;       // memory backing audioBuffer (NULL = unused)
    bool bufferLoaded;
    double length;           // total seconds, measured once while the song opens

    void closeSong();

public:
    MiniaudioBackend();
    ~MiniaudioBackend();

    // Not copyable: a copy would share the live engine and both destructors would
    // shut it down = crash. Deleted here to turn that crash into a compile error.
    MiniaudioBackend(const MiniaudioBackend&) = delete;
    MiniaudioBackend& operator=(const MiniaudioBackend&) = delete;

    bool load(const std::string& path) override;   // open a song
    void play() override;                          // play
    void pause() override;                         // pause
    void seek(double seconds) override;            // seek
    void set_volume(float v) override;             // volume

    double position() override;                    // seconds played
    double duration() override;                    // total seconds
    bool is_playing() override;                    // playing?
    std::string last_error() override;             // last problem ("" = none)
};
