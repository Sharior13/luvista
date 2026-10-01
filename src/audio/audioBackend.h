#pragma once
#include <string>
#include "miniaudio.h"

using namespace std;

class AudioBackend {
private:
    ma_engine engine;     // the audio system
    ma_sound sound;       // the loaded song
    bool engineReady;     // true if the engine started OK
    bool songLoaded;      // true if a song is loaded
    float volume;         // 0.0 to 1.0
    string songName;      // name of the song (from the file name)

public:
    AudioBackend();
    ~AudioBackend();

    bool load(const string& path);
    void play();
    void pause();
    void restart();
    void seek(double seconds);
    void set_volume(float v);

    string SongName() const;      // name of the current song
    double position() const;
    double duration() const;
    bool is_playing() const;
};