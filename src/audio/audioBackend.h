#pragma once
#include <string>

struct AudioBackend {
    virtual ~AudioBackend() = default;
    virtual bool load(const std::string& path) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void seek(double seconds) = 0;
    virtual void set_volume(float v) = 0;   // 0..1
    virtual double position() const = 0;    // seconds
    virtual double duration() const = 0;    // seconds
    virtual bool is_playing() const = 0;
};