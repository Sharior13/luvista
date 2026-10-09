#include "Audioimport.h"

#include <cstdlib>
#include <fstream>
#include <filesystem> // C++17: only used to create the converted folder

namespace AudioImport
{
    // lower-case extension including the dot ("" = none)
    std::string extensionOf(const std::string& path)
    {
        std::string::size_type dot = path.find_last_of('.');
        std::string::size_type slash = path.find_last_of("/\\");
        if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
        {
            return "";
        }

        std::string ext = path.substr(dot);
        for (std::string::size_type i = 0; i < ext.size(); ++i)
        {
            if (ext[i] >= 'A' && ext[i] <= 'Z')
            {
                ext[i] = static_cast<char>(ext[i] - 'A' + 'a');
            }
        }
        return ext;
    }

    // aif/aiff must be converted first
    bool needsConversion(const std::string& path)
    {
        std::string ext = extensionOf(path);
        return ext == ".aif" || ext == ".aiff";
    }

    // every accepted format: mp3 wav flac m4a aif aiff
    bool isSupported(const std::string& path)
    {
        std::string ext = extensionOf(path);
        return ext == ".mp3" || ext == ".wav" || ext == ".flac" || ext == ".m4a" || needsConversion(path);
    }

    // file name without folder and extension
    static std::string fileNameWithoutExtension(const std::string& path)
    {
        std::string::size_type slash = path.find_last_of("/\\");
        std::string name = (slash == std::string::npos) ? path : path.substr(slash + 1);
        std::string::size_type dot = name.find_last_of('.');
        if (dot != std::string::npos)
        {
            name = name.substr(0, dot);
        }
        return name;
    }

    // convert to <convertedFolder>/<name>.flac with ffmpeg ("" = failed)
    std::string convertToFlac(const std::string& path, const std::string& convertedFolder)
    {
        std::filesystem::create_directories(convertedFolder);

        std::string folder = convertedFolder;
        if (!folder.empty() && folder.back() != '/' && folder.back() != '\\')
        {
            folder += '/';
        }
        std::string outPath = folder + fileNameWithoutExtension(path) + ".flac";

        // reuse the FLAC from an earlier run
        {
            std::ifstream existing(outPath.c_str(), std::ios::binary);
            if (existing.good())
            {
                return outPath;
            }
        }

        std::string command = "ffmpeg -y -loglevel error -i \"" + path + "\" \"" + outPath + "\"";
#ifdef _WIN32
        // cmd.exe drops the outer quotes, so wrap the whole command in one more pair.
        command = "\"" + command + "\" >nul 2>&1";
#else
        command += " >/dev/null 2>&1";
#endif

        if (std::system(command.c_str()) != 0)
        {
            return "";
        }

        std::ifstream check(outPath.c_str(), std::ios::binary);
        return check.good() ? outPath : "";
    }

    // the path to play, or "" when unsupported / conversion failed
    std::string preparePlayablePath(const std::string& path, const std::string& convertedFolder)
    {
        if (!isSupported(path))
        {
            return "";
        }
        if (needsConversion(path))
        {
            return convertToFlac(path, convertedFolder);
        }
        return path;
    }
}