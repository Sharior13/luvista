#pragma once

#include <atomic>
#include <cstdio>
#include <string>

#include "miniaudio.h"

// Records the microphone to MP3 (mono, 44.1 kHz, 128 kbps) with the built-in shine
// encoder: no ffmpeg, no WAV. While recording the raw sound goes to a temp file
// (name.mp3.part); stop() encodes it and deletes the temp file.
//
// Do NOT define MINIAUDIO_IMPLEMENTATION here; it already lives in miniaudioBackend.cpp.
class Recorder
{
public:
    Recorder();
    ~Recorder();

    Recorder(const Recorder&) = delete;
    Recorder& operator=(const Recorder&) = delete;

    // start: begin recording to outPath (e.g. "music/rec_1.mp3"). false = failure (see lastError()).
    bool start(const std::string& outPath);

    // stop: encode the temp file into the final MP3 and delete it. Returns the saved path,
    // or "" if nothing was recording / encoding failed (see lastError()).
    std::string stop();

    bool isRecording() const;

    // the file being recorded / the file saved last
    const std::string& currentPath() const;

    // why the last start()/stop() failed ("" = no problem)
    const std::string& lastError() const;

    // first free name: folder/rec_1.mp3, folder/rec_2.mp3, ...
    static std::string nextRecordingPath(const std::string& folder);

    // raw 16-bit mono file -> MP3. Used by stop(); public so it can be tested alone.
    static bool encodeRawToMp3(const std::string& rawPath, const std::string& mp3Path,
        int sampleRate, int bitrateKbps);

private:
    // capture callback: appends the incoming frames to the temp file
    static void dataCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount);

    ma_device m_device;
    std::FILE* m_file;       // temp raw file while recording
    std::atomic<bool> m_recording;
    std::string m_path;      // final path (.mp3)
    std::string m_partPath;  // temp file
    std::string m_error;
};
