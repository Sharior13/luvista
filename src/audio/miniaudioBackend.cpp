// miniaudio's implementation: must be defined in exactly ONE .cpp of the project
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "miniaudioBackend.h"
#include "M4adecoder.h"
#include <cstdlib>



// create: start the engine (no song open, volume 0.5)
MiniaudioBackend::MiniaudioBackend()
{
    songLoaded = false;
    bufferLoaded = false;
    m4aData = NULL;
    volume = 0.5f;
    errorText = "";
    length = 0.0;

    engineReady = (ma_engine_init(NULL, &engine) == MA_SUCCESS);

    if (!engineReady)
        errorText = "Audio engine failed to start";
}


// destroy: close the song, shut the engine down
MiniaudioBackend::~MiniaudioBackend()
{
    closeSong();

    if (engineReady)
        ma_engine_uninit(&engine);
}


// close the open song and free its memory
void MiniaudioBackend::closeSong()
{
    length = 0.0;

    if (songLoaded) {
        ma_sound_uninit(&sound);
        songLoaded = false;
    }

    if (bufferLoaded) {
        ma_audio_buffer_uninit(&audioBuffer);
        bufferLoaded = false;
    }

    if (m4aData != NULL) {
        std::free(m4aData);
        m4aData = NULL;
    }
}


// load: open a song so it is ready to play; true = ok, false = see last_error()
bool MiniaudioBackend::load(const std::string& path)
{
    if (!engineReady) {
        errorText = "Audio engine failed to start";
        return false;
    }

    // close the old song first
    closeSong();

    ma_result result;

    if (M4a::isM4a(path)) {
        // m4a: decode with Windows into memory first, then play from there
        short* pcm = NULL;
        unsigned long long frames = 0;
        unsigned int channels = 0;
        unsigned int rate = 0;
        std::string why;

        if (!M4a::decode(path, &pcm, &frames, &channels, &rate, why)) {
            errorText = "Cannot open m4a: " + why;
            return false;
        }

        ma_audio_buffer_config bufferConfig =
            ma_audio_buffer_config_init(ma_format_s16, channels, frames, pcm, NULL);
        bufferConfig.sampleRate = rate;

        result = ma_audio_buffer_init(&bufferConfig, &audioBuffer);
        if (result != MA_SUCCESS) {
            std::free(pcm);
            errorText = std::string("Cannot open m4a: ") + ma_result_description(result);
            return false;
        }

        m4aData = pcm;
        bufferLoaded = true;

        result = ma_sound_init_from_data_source(&engine, &audioBuffer, 0, NULL, &sound);
        if (result != MA_SUCCESS) {
            closeSong();
            errorText = std::string("Cannot open m4a: ") + ma_result_description(result);
            return false;
        }
    }
    else {
        result = ma_sound_init_from_file(&engine, path.c_str(), 0, NULL, NULL, &sound);
    }

    if (result != MA_SUCCESS) {
        errorText = std::string("Cannot open song: ") + ma_result_description(result);
        return false;
    }

    // squared so each volume step sounds even
    ma_sound_set_volume(&sound, volume * volume);

    // Measure the length NOW, while the sound is still stopped. Asking miniaudio
    // for the length while the song is playing crashes on files without a Xing
    // header (the shine recordings): that call scans and seeks the file while the
    // audio thread is decoding it. Doing it once here keeps duration() a plain read.
    float seconds = 0.0f;
    ma_sound_get_length_in_seconds(&sound, &seconds);
    length = (double)seconds;

    songLoaded = true;
    errorText = "";
    return true;
}


// play
void MiniaudioBackend::play()
{
    if (songLoaded)
        ma_sound_start(&sound);
}


// pause
void MiniaudioBackend::pause()
{
    if (songLoaded)
        ma_sound_stop(&sound);
}


// seek (jump to a second)
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


// volume (0..1; the squared value is what is applied)
void MiniaudioBackend::set_volume(float v)
{
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;

    volume = v;

    if (songLoaded)
        ma_sound_set_volume(&sound, volume * volume);
}


// seconds played so far
double MiniaudioBackend::position()
{
    if (!songLoaded)
        return 0.0;

    float seconds = 0.0f;
    ma_sound_get_cursor_in_seconds(&sound, &seconds);
    return seconds;
}


// The total seconds, from the value measured in load(). Never asks miniaudio
// while the song is playing - that is the call that crashed on rec_1.mp3.
double MiniaudioBackend::duration()
{
    if (!songLoaded)
        return 0.0;

    return length;
}


// playing?
bool MiniaudioBackend::is_playing()
{
    if (!songLoaded)
        return false;

    return ma_sound_is_playing(&sound);
}


// last problem ("" = none)
std::string MiniaudioBackend::last_error()
{
    return errorText;
}