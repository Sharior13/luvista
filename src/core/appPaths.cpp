#include "appPaths.h"
#include <cstdlib>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

static std::string activeName = "guest";


// Makes the folder if it is not there yet, then gives back its address.
static std::string makeFolder(const fs::path& p)
{
    std::error_code problem;
    fs::create_directories(p, problem);
    return p.string();
}


std::string AppPaths::root()
{
    std::string where = ".";          // if the system gives no address, use the current folder

#ifdef _WIN32
    char* base = nullptr;
    size_t size = 0;
    _dupenv_s(&base, &size, "LOCALAPPDATA");

    if (base != nullptr) {
        where = base;
        free(base);
    }
#else
    const char* base = std::getenv("XDG_DATA_HOME");
    const char* home = std::getenv("HOME");

    if (base != nullptr && *base != '\0')
        where = base;
    else if (home != nullptr && *home != '\0')
        where = std::string(home) + "/.local/share";
#endif

    return makeFolder(fs::path(where) / "Luvista");
}


std::string AppPaths::indexFile()
{
    return (fs::path(root()) / "id-index.dat").string();
}


std::string AppPaths::musicFoldersFile()
{
    return (fs::path(root()) / "music-folders.dat").string();
}


std::string AppPaths::removedFile()
{
    return (fs::path(root()) / "removed-songs.dat").string();
}


std::string AppPaths::launchFile()
{
    return (fs::path(root()) / "launch.dat").string();
}


std::string AppPaths::profilesFolder()
{
    return makeFolder(fs::path(root()) / "profiles");
}


std::string AppPaths::profileFolder(const std::string& profile)
{
    return makeFolder(fs::path(root()) / "profiles" / profile);
}


std::string AppPaths::activeProfile()
{
    return activeName;
}


void AppPaths::setActiveProfile(const std::string& profile)
{
    activeName = profile.empty() ? "guest" : profile;
}


std::string AppPaths::activeFolder()
{
    return profileFolder(activeName);
}


std::string AppPaths::recordingsFolder()
{
    return makeFolder(fs::path(activeFolder()) / "recordings");
}


std::string AppPaths::coversFolder()
{
    return makeFolder(fs::path(root()) / "covers");
}


std::string AppPaths::defaultMusicFolder()
{
    std::string home;

#ifdef _WIN32
    char* base = nullptr;
    size_t size = 0;
    _dupenv_s(&base, &size, "USERPROFILE");

    if (base != nullptr) {
        home = base;
        free(base);
    }
#else
    const char* base = std::getenv("HOME");

    if (base != nullptr)
        home = base;
#endif

    if (home.empty())
        return "";

    return (fs::path(home) / "Music").string();
}
