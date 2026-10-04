#pragma once
#include <string>

class AudioBackend {
public:
    virtual ~AudioBackend() {}

    virtual bool load(const std::string& path) = 0;   // open a song file
    virtual void play() = 0;                          // play / continue
    virtual void pause() = 0;                         // stop, but remember the place
    virtual void seek(double seconds) = 0;            // jump to a second in the song
    virtual void set_volume(float v) = 0;             // 0.0 = silent, 1.0 = loudest

    virtual double position() = 0;                    // seconds we have heard so far
    virtual double duration() = 0;                    // how long the song is, in seconds
    virtual bool is_playing() = 0;                    // is it playing right now?
    virtual std::string last_error() = 0;             // what went wrong ("" = nothing)
};
