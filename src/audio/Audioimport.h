#pragma once

#include <string>

// Import helpers for the accepted formats:
//   mp3 wav flac  -> played directly by miniaudio
//   m4a           -> played directly too (decoded by Windows, see M4adecoder.h)
//   aif aiff      -> converted ONCE to FLAC with ffmpeg, then played
namespace AudioImport
{
    // lower-case extension including the dot, e.g. ".m4a" ("" if none)
    std::string extensionOf(const std::string& path);

    // every format this app accepts: mp3 wav flac m4a aif aiff
    bool isSupported(const std::string& path);

    // formats that must be converted first: aif aiff
    bool needsConversion(const std::string& path);

    // converts to <convertedFolder>/<name>.flac (ffmpeg must be on PATH or next to the app);
    // reuses the FLAC if it exists. Returns the FLAC path, or "" on failure.
    std::string convertToFlac(const std::string& path, const std::string& convertedFolder);

    // one call for the scanner: the path to play (mp3/wav/flac/m4a unchanged, aif/aiff as
    // converted FLAC), or "" when unsupported / conversion failed.
    std::string preparePlayablePath(const std::string& path, const std::string& convertedFolder);
}
