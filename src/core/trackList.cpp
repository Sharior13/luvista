#include "trackList.h"
#include "appPaths.h"
#include <fstream>
#include <filesystem>
#include <cctype>
#include <cstdlib>
#include <system_error>

namespace fs = std::filesystem;


// Says yes if a file is a music file.
// It only looks at the ending of the name, like ".mp3".
static bool isMusic(const fs::path& p)
{
    std::string ending = p.extension().string();

    // make all letters small, so ".MP3" and ".mp3" are the same
    for (char& c : ending)
        c = (char)std::tolower((unsigned char)c);

    return ending == ".mp3" || ending == ".wav" || ending == ".m4a" ||
        ending == ".aac" || ending == ".aif" || ending == ".aiff";
}


// A line looks like:  5 <tab> C:/music/Adele - Hello.mp3 <tab> Hello <tab> ...
// This gives back one piece of the line. Piece 0 is the first, piece 1 is the second...
static std::string piece(const std::string& line, int number)
{
    int tabsSeen = 0;
    std::string result = "";

    for (char c : line) {
        if (c == '\t') {
            tabsSeen++;
        }
        else if (tabsSeen == number) {
            result += c;
        }
    }

    return result;
}


// Opens the notebook and gives back line number n. "" if there is no such line.
static std::string lineNumber(int n)
{
    std::ifstream notebook(AppPaths::indexFile());
    std::string line;
    int number = 0;

    while (std::getline(notebook, line)) {
        number++;

        if (number == n)
            return line;
    }

    return "";
}


// A file named "Adele - Hello" is cut at " - ": artist = "Adele", title = "Hello".
// If there is no " - " in the name, the artist is "Unknown artist" and the title is the whole name.
static void cutName(const std::string& fileName, std::string& artist, std::string& title)
{
    size_t place = fileName.find(" - ");

    if (place != std::string::npos && place > 0 && place + 3 < fileName.size()) {
        artist = fileName.substr(0, place);
        title = fileName.substr(place + 3);
    }
    else {
        artist = "Unknown artist";
        title = fileName;
    }
}


// Starts with zero songs, then counts what is already in the notebook from last time.
TrackList::TrackList()
{
    folder = "";
    count = 0;
    nextId = 1;
    countSongs();
}


// Counts the songs in the notebook. Also remembers the biggest id, so a new id is never reused.
void TrackList::countSongs()
{
    std::ifstream notebook(AppPaths::indexFile());
    std::string line;

    count = 0;

    while (std::getline(notebook, line)) {
        if (line == "")
            continue;

        count++;

        int id = std::atoi(piece(line, 0).c_str());

        if (id >= nextId)
            nextId = id + 1;
    }
}


// Looks for a path in the notebook. Gives the line number, or 0 if it is not there.
int TrackList::find(const std::string& path) const
{
    std::ifstream notebook(AppPaths::indexFile());
    std::string line;
    int number = 0;

    while (std::getline(notebook, line)) {
        number++;

        if (piece(line, 1) == path)
            return number;
    }

    return 0;
}


// Old songs keep their id. New songs get a new id.
bool TrackList::scan(const std::string& folderPath)
{
    folder = folderPath;

    std::error_code problem;

    if (!fs::is_directory(folder, problem))
        return false;          // the folder is not there

    // Step 1: keep only the songs whose file is still there
    std::string kept = "";

    {
        std::ifstream notebook(AppPaths::indexFile());
        std::string line;

        while (std::getline(notebook, line)) {
            std::error_code e;

            if (line != "" && fs::exists(piece(line, 1), e))
                kept += line + "\n";
        }
    }   // the notebook is closed here, so we can write to it now

    {
        std::ofstream notebook(AppPaths::indexFile());   // this erases the old notebook
        notebook << kept;
    }

    // Step 2: walk through the folder (and the folders inside it)
    fs::recursive_directory_iterator it(
        folder, fs::directory_options::skip_permission_denied, problem);

    for (; !problem && it != fs::recursive_directory_iterator(); it.increment(problem)) {

        if (it->is_regular_file(problem) && isMusic(it->path())) {
            std::string path = it->path().string();

            if (find(path) == 0) {                        // a new song
                std::string artist, title;
                cutName(it->path().stem().string(), artist, title);

                std::ofstream notebook(AppPaths::indexFile(), std::ios::app);   // add at the end
                notebook << nextId << '\t' << path << '\t' << title << '\t'
                    << artist << '\t' << "-" << '\n';

                nextId++;
            }
        }
    }

    countSongs();
    return true;
}


int TrackList::getCount() const
{
    return count;
}


std::string TrackList::getTrack(int n) const
{
    return piece(lineNumber(n), 1);
}


std::string TrackList::getTitle(int n) const
{
    return piece(lineNumber(n), 2);
}


std::string TrackList::getArtist(int n) const
{
    return piece(lineNumber(n), 3);
}


std::string TrackList::getAlbum(int n) const
{
    return piece(lineNumber(n), 4);
}


// "Date added" is not stored yet, so this gives 0 for every song.
// (Nothing in the console screens uses it. It only exists so the linker finds it.)
long long TrackList::getAdded(int n) const
{
    return 0;
}


int TrackList::getId(int n) const
{
    return std::atoi(piece(lineNumber(n), 0).c_str());
}


// Reads down the notebook until it finds the id. Gives the line number, or 0.
int TrackList::positionOfId(int id) const
{
    std::ifstream notebook(AppPaths::indexFile());
    std::string line;
    int number = 0;

    while (std::getline(notebook, line)) {
        number++;

        if (std::atoi(piece(line, 0).c_str()) == id)
            return number;
    }

    return 0;
}


// Makes every letter small, so "ADELE" and "adele" are the same.
static std::string lowerCase(std::string text)
{
    for (char& c : text)
        c = (char)std::tolower((unsigned char)c);

    return text;
}


bool TrackList::matches(int n, const std::string& query) const
{
    if (n < 1 || n > count)
        return false;

    if (query == "")
        return true;

    std::string wanted = lowerCase(query);

    return lowerCase(getTitle(n)).find(wanted) != std::string::npos ||
        lowerCase(getArtist(n)).find(wanted) != std::string::npos;
}


std::string TrackList::getFolder() const
{
    return folder;
}