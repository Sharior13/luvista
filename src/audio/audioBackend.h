#pragma once
#include <string>

// AudioBackend is only a CONTRACT (a list of promises).
// It says what any speaker must be able to do, but not HOW.
// The real speaker (MiniaudioBackend) fills in the "how".
// Nothing in this file knows that miniaudio exists.
class AudioBackend {
public:
    virtual ~AudioBackend() {}

    virtual bool load(const std::string& path) = 0;   // open a song file
    virtual void play() = 0;                          // start / continue
    virtual void pause() = 0;                         // stop, keep the place
    virtual void seek(double seconds) = 0;            // jump to a second
    virtual void set_volume(float v) = 0;             // 0.0 silent ... 1.0 full

    virtual double position() const = 0;              // seconds heard so far
    virtual double duration() const = 0;              // song length in seconds
    virtual bool is_playing() const = 0;              // playing right now?
    virtual std::string last_error() const = 0;       // "" if everything is fine
};