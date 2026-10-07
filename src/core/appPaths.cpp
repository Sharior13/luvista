#include "appPaths.h"
#include <cstdlib>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;


// Makes the folder if it is not there yet, then gives back its address.
static std::string makeFolder(const fs::path& p)
{
    std::error_code problem;
    fs::create_directories(p, problem);
    return p.string();
}


std::string AppPaths::root()
{
    char* base = nullptr;
    size_t size = 0;
    _dupenv_s(&base, &size, "LOCALAPPDATA");

    std::string where = ".";          // if Windows gives no address, use the current folder

    if (base != nullptr) {
        where = base;
        free(base);
    }

    return makeFolder(fs::path(where) / "Luvista");
}


std::string AppPaths::indexFile()
{
    return (fs::path(root()) / "id-index.dat").string();
}


std::string AppPaths::profileFolder(const std::string& profile)
{
    return makeFolder(fs::path(root()) / "profiles" / profile);
}
