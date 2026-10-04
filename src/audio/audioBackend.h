#pragma once
#include <string>

// THE JOB DESCRIPTION OF A SPEAKER
//
// This file has NO real code. It only lists what any speaker must be able to do.
// "= 0" means: "I promise this function exists, but I will not write it here."
// The real speaker (MiniaudioBackend) is the one that writes the code.
//
// Why do this? So the rest of the program can say "speaker, play!"
// without knowing which music library is inside the speaker.

class AudioBackend {
public:
    // (needed by C++ so the real speaker is cleaned up properly)
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
