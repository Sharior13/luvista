// This line makes miniaudio include all its code in THIS file.
// It must appear in exactly one .cpp file in the whole project.
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "miniaudioBackend.h"



// Runs when the speaker is created.
// Turns the sound machine on. No song is open yet, volume is half.
MiniaudioBackend::MiniaudioBackend()
{
    songLoaded = false;
    volume = 0.5f;
    errorText = "";

    engineReady = (ma_engine_init(NULL, &engine) == MA_SUCCESS);

    if (!engineReady)
        errorText = "Audio engine failed to start";
}


// Runs when the program ends.
// Closes the song and turns the machine off, like cleaning up the kitchen.
MiniaudioBackend::~MiniaudioBackend()
{
    if (songLoaded)
        ma_sound_uninit(&sound);

    if (engineReady)
        ma_engine_uninit(&engine);
}


// Opens a song file so it is ready to play.
// Gives back true if it worked, false if it did not.
bool MiniaudioBackend::load(const std::string& path)
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

    // try to open the new song
    ma_result result = ma_sound_init_from_file(&engine, path.c_str(), 0, NULL, NULL, &sound);

    if (result != MA_SUCCESS) {
        errorText = std::string("Cannot open song: ") + ma_result_description(result);
        return false;
    }

    // the volume is squared so each step sounds like an even change
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


// Jumps to a second in the song. seek(30) goes to second 30.
void MiniaudioBackend::seek(double seconds)
{
    if (!songLoaded)
        return;

    if (seconds < 0)
        seconds = 0;

    ma_uint32 fileRate = 0;
    ma_sound_get_data_format(&sound, NULL, NULL, &fileRate, NULL, 0);

    if (fileRate == 0)
        return;

    ma_uint64 frame = (ma_uint64)(seconds * fileRate);
    ma_sound_seek_to_pcm_frame(&sound, frame);
}


void MiniaudioBackend::set_volume(float v)
{
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;

    volume = v;

    if (songLoaded)
        ma_sound_set_volume(&sound, volume * volume);
}


// How many seconds of the song we have heard so far.
double MiniaudioBackend::position()
{
    if (!songLoaded)
        return 0.0;

    float seconds = 0.0f;
    ma_sound_get_cursor_in_seconds(&sound, &seconds);
    return seconds;
}


// How long the whole song is, in seconds.
double MiniaudioBackend::duration()
{
    if (!songLoaded)
        return 0.0;

    float seconds = 0.0f;
    ma_sound_get_length_in_seconds(&sound, &seconds);
    return seconds;
}


// Yes if the song is playing right now.
bool MiniaudioBackend::is_playing()
{
    if (!songLoaded)
        return false;

    return ma_sound_is_playing(&sound);
}


// The last problem, or "" if there was none.
std::string MiniaudioBackend::last_error()
{
    return errorText;
}
