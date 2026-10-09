#include "Recorder.h"

#include <cstdint>
#include <cstdlib>
#include <fstream>

extern "C"
{
#include "layer3.h" // the shine MP3 encoder (external/shine is on the include path)
}

// recording format
static const int kSampleRate = 44100;
static const int kBitrateKbps = 128;

static bool fileExists(const std::string& path)
{
    std::ifstream test(path.c_str(), std::ios::binary);
    return test.good();
}

Recorder::Recorder()
    : m_device(), m_file(nullptr), m_recording(false), m_path(), m_partPath(), m_error()
{
}

Recorder::~Recorder()
{
    stop();
}

// capture callback: appends incoming frames to the temp file
void Recorder::dataCallback(ma_device* device, void* /*output*/, const void* input, ma_uint32 frameCount)
{
    Recorder* self = static_cast<Recorder*>(device->pUserData);
    if (self != nullptr && self->m_file != nullptr && input != nullptr)
    {
        // mono, 16-bit: one frame = one sample
        std::fwrite(input, sizeof(ma_int16), frameCount, self->m_file);
    }
}

// start: open the temp file and the microphone
bool Recorder::start(const std::string& outPath)
{
    if (m_recording)
    {
        return false;
    }

    m_error = "";
    std::string partPath = outPath + ".part"; // scanners ignore .part files

    m_file = std::fopen(partPath.c_str(), "wb");
    if (m_file == nullptr)
    {
        m_error = "Cannot create the file (is the folder writable?)";
        return false;
    }

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.format = ma_format_s16;
    deviceConfig.capture.channels = 1;
    deviceConfig.sampleRate = kSampleRate;
    deviceConfig.dataCallback = dataCallback;
    deviceConfig.pUserData = this;

    if (ma_device_init(nullptr, &deviceConfig, &m_device) != MA_SUCCESS)
    {
        m_error = "No microphone found";
        std::fclose(m_file);
        m_file = nullptr;
        std::remove(partPath.c_str());
        return false;
    }

    if (ma_device_start(&m_device) != MA_SUCCESS)
    {
        m_error = "Could not start the microphone";
        ma_device_uninit(&m_device);
        std::fclose(m_file);
        m_file = nullptr;
        std::remove(partPath.c_str());
        return false;
    }

    m_path = outPath;
    m_partPath = partPath;
    m_recording = true;
    return true;
}

// stop: stop the mic, encode the temp file into the MP3, delete the temp file
std::string Recorder::stop()
{
    if (!m_recording)
    {
        return "";
    }

    m_recording = false;
    ma_device_uninit(&m_device);  // stops the callback first
    std::fclose(m_file);          // then closes the temporary file
    m_file = nullptr;

    bool ok = encodeRawToMp3(m_partPath, m_path, kSampleRate, kBitrateKbps);
    std::remove(m_partPath.c_str());

    if (!ok)
    {
        m_error = "Could not make the MP3 (nothing was recorded?)";
        std::remove(m_path.c_str());
        return "";
    }
    return m_path;
}

// raw 16-bit mono file -> MP3 with shine
bool Recorder::encodeRawToMp3(const std::string& rawPath, const std::string& mp3Path,
    int sampleRate, int bitrateKbps)
{
    shine_config_t config;
    shine_set_config_mpeg_defaults(&config.mpeg);
    config.wave.channels = PCM_MONO;
    config.wave.samplerate = sampleRate;
    config.mpeg.mode = MONO;
    config.mpeg.bitr = bitrateKbps;

    if (shine_check_config(config.wave.samplerate, config.mpeg.bitr) < 0)
    {
        return false;
    }

    std::FILE* in = std::fopen(rawPath.c_str(), "rb");
    if (in == nullptr)
    {
        return false;
    }

    shine_t shine = shine_initialise(&config);
    if (shine == nullptr)
    {
        std::fclose(in);
        return false;
    }

    std::FILE* out = std::fopen(mp3Path.c_str(), "wb");
    if (out == nullptr)
    {
        shine_close(shine);
        std::fclose(in);
        return false;
    }

    const int samplesPerPass = shine_samples_per_pass(shine); // 1152 for 44.1 kHz
    std::int16_t buffer[SHINE_MAX_SAMPLES];
    bool ok = true;
    bool gotAnySound = false;

    while (ok)
    {
        std::size_t got = std::fread(buffer, sizeof(std::int16_t), static_cast<std::size_t>(samplesPerPass), in);
        if (got == 0)
        {
            break;
        }
        gotAnySound = true;

        for (std::size_t i = got; i < static_cast<std::size_t>(samplesPerPass); ++i)
        {
            buffer[i] = 0; // pad the last piece with silence
        }

        int written = 0;
        unsigned char* data = shine_encode_buffer_interleaved(shine, buffer, &written);
        if (data != nullptr && written > 0)
        {
            if (std::fwrite(data, 1, static_cast<std::size_t>(written), out) != static_cast<std::size_t>(written))
            {
                ok = false;
            }
        }

        if (got < static_cast<std::size_t>(samplesPerPass))
        {
            break;
        }
    }

    if (ok)
    {
        int written = 0;
        unsigned char* data = shine_flush(shine, &written);
        if (data != nullptr && written > 0)
        {
            if (std::fwrite(data, 1, static_cast<std::size_t>(written), out) != static_cast<std::size_t>(written))
            {
                ok = false;
            }
        }
    }

    shine_close(shine);
    std::fclose(in);
    if (std::fclose(out) != 0)
    {
        ok = false;
    }

    if (!(ok && gotAnySound))
    {
        std::remove(mp3Path.c_str());
        return false;
    }
    return true;
}

// recording?
bool Recorder::isRecording() const
{
    return m_recording;
}

// the file being recorded / saved last
const std::string& Recorder::currentPath() const
{
    return m_path;
}

// why the last start()/stop() failed ("" = none)
const std::string& Recorder::lastError() const
{
    return m_error;
}

// first free rec_N.mp3 in the folder
std::string Recorder::nextRecordingPath(const std::string& folder)
{
    std::string base = folder;
    if (!base.empty() && base.back() != '/' && base.back() != '\\')
    {
        base += '/';
    }

    for (int n = 1; n < 100000; ++n)
    {
        std::string candidate = base + "rec_" + std::to_string(n) + ".mp3";
        if (!fileExists(candidate))
        {
            return candidate;
        }
    }
    return base + "rec_new.mp3";
}