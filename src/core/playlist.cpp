#include "playlist.h"
#include <fstream>
#include <string>

static const char* PLAYLISTS_FILE = "playlist-index.dat";


// The notebook that belongs to one playlist.
static std::string fileFor(const std::string& name)
{
    return "playlist-" + name + ".txt";
}


// A name is fine if it is not empty and has none of the letters
// that a file name cannot hold.
static bool nameIsOk(const std::string& name)
{
    if (name.empty())
        return false;

    const std::string bad = "\\/:*?\"<>|";

    for (char c : name) {
        if (bad.find(c) != std::string::npos)
            return false;
    }
    return true;
}


// Is there already a playlist with this name?
bool Playlists::exists(const std::string& name) const
{
    std::ifstream File(PLAYLISTS_FILE);
    std::string line;

    while (std::getline(File, line)) {
        if (line == name)
            return true;
    }
    return false;
}


// Makes a new empty playlist.
bool Playlists::create(const std::string& name)
{
    if (!nameIsOk(name) || exists(name))
        return false;

    std::ofstream list(PLAYLISTS_FILE, std::ios::app);   // add the name at the end
    list << name << "\n";

    std::ofstream songs(fileFor(name));                  // make its empty notebook
    return true;
}


// How many playlists there are.
int Playlists::count() const
{
    std::ifstream File(PLAYLISTS_FILE);
    std::string line;
    int lines = 0;

    while (std::getline(File, line))
        lines++;

    return lines;
}


// The name of playlist number n.
std::string Playlists::getName(int n) const
{
    std::ifstream File(PLAYLISTS_FILE);
    std::string line;
    int lineNumber = 0;

    while (std::getline(File, line)) {
        lineNumber++;

        if (lineNumber == n)
            return line;
    }
    return "";
}


// Is this song already in the playlist?
bool Playlists::hasSong(const std::string& name, const std::string& path) const
{
    std::ifstream File(fileFor(name));
    std::string line;

    while (std::getline(File, line)) {
        if (line == path)
            return true;
    }
    return false;
}


// Puts a song in the playlist (no copies).
bool Playlists::addSong(const std::string& name, const std::string& path)
{
    if (!exists(name) || hasSong(name, path))
        return false;

    std::ofstream File(fileFor(name), std::ios::app);
    File << path << "\n";
    return true;
}


// How many songs are in the playlist.
int Playlists::songCount(const std::string& name) const
{
    std::ifstream File(fileFor(name));
    std::string line;
    int lines = 0;

    while (std::getline(File, line))
        lines++;

    return lines;
}


// The address of song number n in the playlist.
std::string Playlists::getSong(const std::string& name, int n) const
{
    std::ifstream File(fileFor(name));
    std::string line;
    int lineNumber = 0;

    while (std::getline(File, line)) {
        lineNumber++;

        if (lineNumber == n)
            return line;
    }
    return "";
}

