#include "playlist.h"
#include "appPaths.h"
#include <fstream>
#include <string>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

// The notebook with all playlist names
static std::string namesFile() {
    return AppPaths::activeFolder() + "/playlists.dat";
}

// Generate filename
static std::string fileFor(const std::string& name) {
    return AppPaths::activeFolder() + "/playlist-" + name + ".dat";
}

// The picture file of a playlist
static fs::path artFor(const std::string& name, const std::string& ending) {
    return fs::path(AppPaths::activeFolder()) / ("playlist-" + name + ".art" + ending);
}

// Spaces off both ends, so "  Gym " and "Gym" are the same playlist
static std::string trimmed(const std::string& name) {
    size_t a = name.find_first_not_of(" \t");
    if (a == std::string::npos)
        return "";
    return name.substr(a, name.find_last_not_of(" \t") - a + 1);
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
bool Playlists::create(const std::string& rawName) {
    std::string name = trimmed(rawName);

    if (!nameIsOk(name) || exists(name))
        return false;

    std::ofstream list(namesFile(), std::ios::app);   // add the name at the end
    list << name << "\n";

    std::ofstream songs(fileFor(name));               // make its empty notebook
    return true;
}

// Gives a playlist a new name: the name in the list changes and its song file and picture move to the new name.
bool Playlists::rename(const std::string& oldName, const std::string& rawNewName) {
    std::string newName = trimmed(rawNewName);

    if (oldName == newName)
        return exists(oldName);

    if (!exists(oldName) || !nameIsOk(newName) || exists(newName))
        return false;

    std::error_code problem;
    fs::rename(fileFor(oldName), fileFor(newName), problem);
    if (problem)
        return false;

    for (const char* ending : { ".png", ".jpg" }) {
        std::error_code ignored;
        if (fs::is_regular_file(artFor(oldName, ending), ignored))
            fs::rename(artFor(oldName, ending), artFor(newName, ending), ignored);
    }

    std::ifstream in(namesFile());
    std::string kept = "";
    std::string line;

    while (std::getline(in, line))
        kept += (line == oldName ? newName : line) + "\n";   // same place in the list, new name
    in.close();

    std::ofstream out(namesFile());
    out << kept;
    return true;
}

// A name nobody uses yet: "New playlist", then "New playlist 2", "New playlist 3", ...
std::string Playlists::freeName(const std::string& base) const {
    if (!exists(base))
        return base;

    for (int i = 2; ; i++) {
        std::string candidate = base + " " + std::to_string(i);

        if (!exists(candidate))
            return candidate;
    }
}

// Deletes the playlist: takes its name out of the list and erases its song file.
bool Playlists::remove(const std::string& name) {
    if (!exists(name))
        return false;

    std::ifstream in(namesFile());
    std::string kept = "";
    std::string line;

    while (std::getline(in, line)) {
        if (line != name)
            kept += line + "\n";   // keep every name except this one
    }
    in.close();

    std::ofstream out(namesFile());   // this erases the old list
    out << kept;

    std::error_code problem;
    fs::remove(fileFor(name), problem);   // erase the song notebook too
    fs::remove(artFor(name, ".png"), problem);
    fs::remove(artFor(name, ".jpg"), problem);

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

// Takes one song out of the playlist (the other songs stay).
bool Playlists::removeSong(const std::string& name, int id) {
    if (!exists(name) || !hasSong(name, id))
        return false;

    std::ifstream in(fileFor(name));
    std::string kept = "";
    std::string line;

    while (std::getline(in, line)) {
        if (std::atoi(line.c_str()) != id)
            kept += line + "\n";   // keep every song except this one
    }
    in.close();

    std::ofstream out(fileFor(name));   // this erases the old list
    out << kept;

    return true;
}

// Takes a song out of every playlist that has it.
void Playlists::removeSongEverywhere(int id) {
    if (id == 0)
        return;

    int total = count();

    for (int i = 1; i <= total; i++)
        removeSong(getName(i), id);
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


// The picture chosen for the playlist
std::string Playlists::art(const std::string& name) const {
    std::error_code problem;

    for (const char* ending : { ".png", ".jpg" }) {
        fs::path p = artFor(name, ending);

        if (fs::is_regular_file(p, problem))
            return p.string();
    }
    return "";
}

// Copies a picture in. Only png and jpg, because that is what the screen can show.
bool Playlists::setArt(const std::string& name, const std::string& imagePath) {
    std::error_code problem;

    if (!exists(name) || !fs::is_regular_file(imagePath, problem))
        return false;

    std::string ending = fs::path(imagePath).extension().string();

    for (char& c : ending)
        c = (char)std::tolower((unsigned char)c);

    if (ending == ".jpeg")
        ending = ".jpg";

    if (ending != ".png" && ending != ".jpg")
        return false;

    fs::remove(artFor(name, ".png"), problem);       // the old picture of the other kind would win
    fs::remove(artFor(name, ".jpg"), problem);

    fs::copy_file(imagePath, artFor(name, ending), fs::copy_options::overwrite_existing, problem);
    return !problem;
}
