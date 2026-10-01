#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "audioBackend.h"
#include <iostream>

using namespace std;

AudioBackend::AudioBackend()
{
    songLoaded = false;
    volume = 0.5f;
    songName = "";
    engineReady = (ma_engine_init(NULL, &engine) == MA_SUCCESS);
}

AudioBackend::~AudioBackend()
{
    if (songLoaded)
        ma_sound_uninit(&sound);

    if (engineReady)
        ma_engine_uninit(&engine);
}

bool AudioBackend::load(const string& path)
{
    if (!engineReady) {
        cout << "Audio engine failed to start" << endl;
        return false;
    }

    // free the old song first
    if (songLoaded) {
        ma_sound_uninit(&sound);
        songLoaded = false;
    }

    ma_result result = ma_sound_init_from_file(&engine, path.c_str(), 0, NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
        cout << "miniaudio error: " << ma_result_description(result) << endl;
        return false;
    }

    // get the song name from the path: remove folders and ".mp3"
    songName = path;

    size_t slash = songName.find_last_of("/\\");
    if (slash != string::npos)
        songName = songName.substr(slash + 1);

    size_t dot = songName.find_last_of('.');
    if (dot != string::npos)
        songName = songName.substr(0, dot);

    ma_sound_set_volume(&sound, volume);
    songLoaded = true;
    return true;
}

void AudioBackend::play()
{
    if (songLoaded)
        ma_sound_start(&sound);
}

void AudioBackend::pause()
{
    // stop keeps the position, so play() continues from the same spot
    if (songLoaded)
        ma_sound_stop(&sound);
}

void AudioBackend::restart() {
    // restart the audio
    if (!songLoaded)
        return;

    ma_sound_seek_to_pcm_frame(&sound, 0);
    ma_sound_start(&sound);
}
void AudioBackend::seek(double seconds)
{
    if (!songLoaded)
        return;

    if (seconds < 0)
        seconds = 0;

    ma_uint32 sampleRate = ma_engine_get_sample_rate(&engine);
    ma_uint64 frame = (ma_uint64)(seconds * sampleRate);
    ma_sound_seek_to_pcm_frame(&sound, frame);
}

void AudioBackend::set_volume(float v)
{
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;

    volume = v;
    if (songLoaded)
        ma_sound_set_volume(&sound, volume);
}

string AudioBackend::SongName() const
{
    return songName;
}

double AudioBackend::position() const
{
    if (!songLoaded)
        return 0.0;

    float seconds = 0.0f;
    ma_sound_get_cursor_in_seconds(const_cast<ma_sound*>(&sound), &seconds);
    return seconds;
}

double AudioBackend::duration() const
{
    if (!songLoaded)
        return 0.0;

    float seconds = 0.0f;
    ma_sound_get_length_in_seconds(const_cast<ma_sound*>(&sound), &seconds);
    return seconds;
}

bool AudioBackend::is_playing() const
{
    if (!songLoaded)
        return false;

    return ma_sound_is_playing(&sound);
}