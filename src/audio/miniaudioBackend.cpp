#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "miniaudioBackend.h"

using namespace std;

// Turns the speaker on. No song is open yet and the volume is half.
MiniaudioBackend::MiniaudioBackend()
{
    songLoaded = false;
    volume = 0.5f;
    errorText = "";
    engineReady = (ma_engine_init(NULL, &engine) == MA_SUCCESS);

    if (!engineReady)
        errorText = "Audio engine failed to start";
}

// Cleans up when the program ends: closes the song, turns the speaker off.
MiniaudioBackend::~MiniaudioBackend()
{
    if (songLoaded)
        ma_sound_uninit(&sound);

    if (engineReady)
        ma_engine_uninit(&engine);
}

// Opens a song file so it is ready to play.
// Says true if it worked, false if not (the reason is in last_error()).
bool MiniaudioBackend::load(const string& path)
{
    if (!engineReady) {
        errorText = "Audio engine failed to start";
        return false;
    }

    // close the old song first
    if (songLoaded) {
        ma_sound_uninit(&sound);
        songLoaded = false;
    }

    ma_result result = ma_sound_init_from_file(&engine, path.c_str(), 0, NULL, NULL, &sound);

    if (result != MA_SUCCESS) {
        errorText = string("Cannot open song: ") + ma_result_description(result);
        return false;
    }

    ma_sound_set_volume(&sound, volume * volume);
    songLoaded = true;
    errorText = "";
    return true;
}

// Presses the play button.
void MiniaudioBackend::play()
{
    if (songLoaded)
        ma_sound_start(&sound);
}

// Presses the pause button. The song waits at the same place.
void MiniaudioBackend::pause()
{
    if (songLoaded)
        ma_sound_stop(&sound);
}

// Jumps to a place in the song. seek(30) goes to second 30.
void MiniaudioBackend::seek(double seconds)
{
    if (!songLoaded)
        return;

    if (seconds < 0)
        seconds = 0;

    ma_uint32 sampleRate = ma_engine_get_sample_rate(&engine);
    ma_uint64 frame = (ma_uint64)(seconds * sampleRate);
    ma_sound_seek_to_pcm_frame(&sound, frame);
}

// Turns the volume knob (0 to 1).
// The number is squared so each step sounds like an even change.
void MiniaudioBackend::set_volume(float v)
{
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;

    volume = v;

    if (songLoaded)
        ma_sound_set_volume(&sound, volume * volume);
}

// How many seconds of the song we have heard.
double MiniaudioBackend::position() const
{
    if (!songLoaded)
        return 0.0;

    float seconds = 0.0f;
    ma_sound_get_cursor_in_seconds(const_cast<ma_sound*>(&sound), &seconds);
    return seconds;
}

// How long the whole song is, in seconds.
double MiniaudioBackend::duration() const
{
    if (!songLoaded)
        return 0.0;

    float seconds = 0.0f;
    ma_sound_get_length_in_seconds(const_cast<ma_sound*>(&sound), &seconds);
    return seconds;
}

// Yes if the song is playing right now.
bool MiniaudioBackend::is_playing() const
{
    if (!songLoaded)
        return false;

    return ma_sound_is_playing(&sound);
}

// The last problem, or "" if there was none.
string MiniaudioBackend::last_error() const
{
    return errorText;
}
