#pragma once

#include <atomic>
#include <chrono>
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

    // start: begin recording to outPath (use nextRecordingPath()). false = failure (see lastError()).
    bool start(const std::string& outPath);

    // stop: encode the temp file into the final MP3 and delete it. Returns the saved path,
    // or "" if nothing was recording / encoding failed (see lastError()).
    std::string stop();

    bool isRecording() const;

    // how loud the last piece of sound was, 0..1 (the recorder window's level meter)
    float level() const;

    // stop and throw the recording away (no MP3 is made, the temp file is deleted)
    void discard();

    // why the last start()/stop() failed ("" = no problem)
    const std::string& lastError() const;

    // seconds since start() (0 when nothing is recording)
    double seconds() const;

    // first free name in the folder (rec_1.mp3, rec_2.mp3, ...). Creates the folder if missing.
    static std::string nextPathIn(const std::string& folder);

    // the path for a recording the user named: <folder>/<name>.mp3.
    // Letters a file name cannot hold are dropped. An empty name gives nextPathIn(folder).
    // "" + error filled in when the name is taken.
    static std::string pathForName(const std::string& folder, const std::string& name, std::string& error);

    // raw 16-bit mono file -> MP3. Used by stop(); public so it can be tested alone.
    static bool encodeRawToMp3(const std::string& rawPath, const std::string& mp3Path,
        int sampleRate, int bitrateKbps);

private:
    // capture callback: appends the incoming frames to the temp file
    static void dataCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount);

    ma_device m_device;
    std::FILE* m_file;       // temp raw file while recording
    std::atomic<bool> m_recording;
    std::atomic<float> m_level;
    std::string m_path;      // final path (.mp3)
    std::string m_partPath;  // temp file
    std::string m_error;
    std::chrono::steady_clock::time_point m_startedAt;
};
