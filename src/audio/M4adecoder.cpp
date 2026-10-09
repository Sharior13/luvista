#include "M4adecoder.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

// Visual Studio links these automatically. (MinGW: add mfplat mfreadwrite mfuuid ole32 to CMake.)
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")
#endif

namespace M4a
{
    // true if the path ends in .m4a (any letter case)
    bool isM4a(const std::string& path)
    {
        if (path.size() < 4)
        {
            return false;
        }

        std::string ending = path.substr(path.size() - 4);
        for (std::string::size_type i = 0; i < ending.size(); ++i)
        {
            if (ending[i] >= 'A' && ending[i] <= 'Z')
            {
                ending[i] = static_cast<char>(ending[i] - 'A' + 'a');
            }
        }
        return ending == ".m4a";
    }

#ifdef _WIN32

    // HRESULT -> "0x........" text for error messages
    static std::string hresultText(HRESULT hr)
    {
        char text[32];
        std::snprintf(text, sizeof(text), "0x%08lX", static_cast<unsigned long>(hr));
        return text;
    }

    // decode the whole file into RAM (COM and MF are already started by decode())
    static bool decodeFile(const std::string& path, short** pcm, unsigned long long* frames,
        unsigned int* channels, unsigned int* sampleRate, std::string& error)
    {
        // the path as wide text
        int wideLength = MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, NULL, 0);
        if (wideLength <= 0)
        {
            error = "bad file name";
            return false;
        }
        std::wstring widePath;
        widePath.resize(static_cast<std::wstring::size_type>(wideLength));
        MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, &widePath[0], wideLength);

        IMFSourceReader* reader = NULL;
        unsigned char* data = NULL;
        size_t used = 0;
        size_t capacity = 0;
        UINT32 channelCount = 0;
        UINT32 rate = 0;
        bool ok = false;

        do
        {
            HRESULT hr = MFCreateSourceReaderFromURL(widePath.c_str(), NULL, &reader);
            if (FAILED(hr))
            {
                error = "Windows could not open the file (" + hresultText(hr) + ")";
                break;
            }

            // ask for plain 16-bit PCM (MF adds the AAC decoder itself)
            IMFMediaType* wanted = NULL;
            hr = MFCreateMediaType(&wanted);
            if (FAILED(hr))
            {
                error = "out of memory (" + hresultText(hr) + ")";
                break;
            }
            wanted->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
            wanted->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
            wanted->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
            hr = reader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), NULL, wanted);
            wanted->Release();
            if (FAILED(hr))
            {
                error = "no decoder for this audio (" + hresultText(hr) + ")";
                break;
            }

            // what did we actually get?
            IMFMediaType* actual = NULL;
            hr = reader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &actual);
            if (FAILED(hr))
            {
                error = "could not read the audio format (" + hresultText(hr) + ")";
                break;
            }
            UINT32 bits = 0;
            actual->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channelCount);
            actual->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &rate);
            actual->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bits);
            actual->Release();
            if (channelCount == 0 || rate == 0 || bits != 16)
            {
                error = "unexpected audio format";
                break;
            }

            // read the whole song into one growing block of memory
            bool failed = false;
            while (true)
            {
                DWORD flags = 0;
                IMFSample* sample = NULL;
                hr = reader->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM),
                    0, NULL, &flags, NULL, &sample);
                if (FAILED(hr) || (flags & MF_SOURCE_READERF_ERROR))
                {
                    if (sample != NULL)
                    {
                        sample->Release();
                    }
                    error = "error while decoding (" + hresultText(hr) + ")";
                    failed = true;
                    break;
                }

                if (sample != NULL)
                {
                    IMFMediaBuffer* buffer = NULL;
                    if (SUCCEEDED(sample->ConvertToContiguousBuffer(&buffer)))
                    {
                        BYTE* bytes = NULL;
                        DWORD length = 0;
                        if (SUCCEEDED(buffer->Lock(&bytes, NULL, &length)))
                        {
                            if (used + length > capacity)
                            {
                                size_t newCapacity = (capacity == 0) ? (1u << 20) : capacity;
                                while (newCapacity < used + length)
                                {
                                    newCapacity *= 2;
                                }
                                unsigned char* bigger = static_cast<unsigned char*>(std::realloc(data, newCapacity));
                                if (bigger == NULL)
                                {
                                    buffer->Unlock();
                                    buffer->Release();
                                    sample->Release();
                                    error = "not enough memory for this song";
                                    failed = true;
                                    break;
                                }
                                data = bigger;
                                capacity = newCapacity;
                            }
                            std::memcpy(data + used, bytes, length);
                            used += length;
                            buffer->Unlock();
                        }
                        buffer->Release();
                    }
                    sample->Release();
                }

                if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
                {
                    break;
                }
            }

            if (failed)
            {
                break;
            }

            ok = true;
        } while (false);

        if (reader != NULL)
        {
            reader->Release();
        }

        if (!ok)
        {
            std::free(data);
            return false;
        }

        unsigned long long frameCount = used / (static_cast<size_t>(channelCount) * 2);
        if (frameCount == 0)
        {
            std::free(data);
            error = "no audio found in the file";
            return false;
        }

        *pcm = reinterpret_cast<short*>(data);
        *frames = frameCount;
        *channels = channelCount;
        *sampleRate = rate;
        return true;
    }

    // decode: start/stop MF around decodeFile
    bool decode(const std::string& path, short** pcm, unsigned long long* frames,
        unsigned int* channels, unsigned int* sampleRate, std::string& error)
    {
        *pcm = NULL;
        *frames = 0;
        *channels = 0;
        *sampleRate = 0;
        error = "";

        // COM may already run here in another mode; we only undo what we started ourselves
        HRESULT comResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        bool weStartedCom = SUCCEEDED(comResult);

        HRESULT hr = MFStartup(MF_VERSION);
        if (FAILED(hr))
        {
            error = "Media Foundation is not available (" + hresultText(hr) + ")";
            if (weStartedCom)
            {
                CoUninitialize();
            }
            return false;
        }

        bool ok = decodeFile(path, pcm, frames, channels, sampleRate, error);

        MFShutdown();
        if (weStartedCom)
        {
            CoUninitialize();
        }
        return ok;
    }

#else

    // not Windows: m4a is not implemented
    bool decode(const std::string& /*path*/, short** pcm, unsigned long long* frames,
        unsigned int* channels, unsigned int* sampleRate, std::string& error)
    {
        *pcm = NULL;
        *frames = 0;
        *channels = 0;
        *sampleRate = 0;
        error = "m4a playback is only built for Windows";
        return false;
    }

#endif
}