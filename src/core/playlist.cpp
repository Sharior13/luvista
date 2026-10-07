#include "playlist.h"
#include "appPaths.h"
#include <fstream>
#include <string>
#include <cstdlib>

// The notebook with all playlist names
static std::string namesFile() {
    return AppPaths::profileFolder("default") + "/playlists.dat";
}

// Generate filename
static std::string fileFor(const std::string& name) {
    return AppPaths::profileFolder("default") + "/playlist-" + name + ".dat";
}

// Validate name
static bool nameIsOk(const std::string& name) {
    if (name.empty())
        return false;

    const std::string bad = "\\/:*?\"<>|";

    for (char c : name) {
        if (bad.find(c) != std::string::npos)
            return false;
    }
    return true;
}

// Check playlist
bool Playlists::exists(const std::string& name) const {
    std::ifstream File(namesFile());
    std::string line;

    while (std::getline(File, line)) {
        if (line == name)
            return true;
    }
    return false;
}

// Create playlist
bool Playlists::create(const std::string& name) {
    if (!nameIsOk(name) || exists(name))
        return false;

    std::ofstream list(namesFile(), std::ios::app);   // add the name at the end
    list << name << "\n";

    std::ofstream songs(fileFor(name));               // make its empty notebook
    return true;
}

// Count playlists
int Playlists::count() const {
    std::ifstream File(namesFile());
    std::string line;
    int lines = 0;

    while (std::getline(File, line))
        lines++;

    return lines;
}

// Fetch playlist
std::string Playlists::getName(int n) const {
    std::ifstream File(namesFile());
    std::string line;
    int lineNumber = 0;

    while (std::getline(File, line)) {
        lineNumber++;

        if (lineNumber == n)
            return line;
    }
    return "";
}

// Check song
bool Playlists::hasSong(const std::string& name, int id) const {
    std::ifstream File(fileFor(name));
    std::string line;

    while (std::getline(File, line)) {
        if (std::atoi(line.c_str()) == id)
            return true;
    }
    return false;
}

// Append song
bool Playlists::addSong(const std::string& name, int id) {
    if (id == 0 || !exists(name) || hasSong(name, id))
        return false;

    std::ofstream File(fileFor(name), std::ios::app);
    File << id << "\n";
    return true;
}

// Count songs
int Playlists::songCount(const std::string& name) const {
    std::ifstream File(fileFor(name));
    std::string line;
    int lines = 0;

    while (std::getline(File, line))
        lines++;

    return lines;
}

// Get song
int Playlists::getSong(const std::string& name, int n) const {
    std::ifstream File(fileFor(name));
    std::string line;
    int lineNumber = 0;

    while (std::getline(File, line)) {
        lineNumber++;

        if (lineNumber == n)
            return std::atoi(line.c_str());
    }
    return 0;
}
