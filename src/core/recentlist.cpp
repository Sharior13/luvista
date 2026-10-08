#include "recentList.h"
#include "appPaths.h"
#include <fstream>
#include <string>
#include <cstdlib>

// Where the recent notebook lives
static std::string recentFile()
{
    return AppPaths::profileFolder("default") + "/recent.dat";
}

bool RecentList::contains(int id) const
{
    std::ifstream File(recentFile());
    std::string line;

    while (std::getline(File, line))
    {
        if (std::atoi(line.c_str()) == id)
            return true;
    }

    return false;
}

// Puts the song at the TOP of the list (newest first).
// If the song was already there, the old copy is dropped.
// The list never grows past 10 songs.
void RecentList::add(int id)
{
    std::string newList = std::to_string(id) + "\n";   // newest song goes first
    int kept = 1;

    std::ifstream in(recentFile());
    std::string line;

    while (kept < 10 && std::getline(in, line))
    {
        if (std::atoi(line.c_str()) != id)             // skip the old copy of this song
        {
            newList += line + "\n";
            kept++;
        }
    }
    in.close();                          // finish reading before writing

    std::ofstream out(recentFile());     // this erases the old file
    out << newList;
}

// Takes one song out of the list (the other songs stay).
void RecentList::remove(int id)
{
    std::ifstream in(recentFile());
    std::string kept = "";
    std::string line;

    while (std::getline(in, line)) {
        if (std::atoi(line.c_str()) != id)
            kept += line + "\n";   // keep every song except this one
    }
    in.close();

    std::ofstream out(recentFile());   // this erases the old file
    out << kept;
}

int RecentList::count() const
{
    std::ifstream File(recentFile());
    std::string line;
    int lines = 0;

    while (std::getline(File, line))
        lines++;

    return lines;
}

int RecentList::get(int n) const
{
    std::ifstream File(recentFile());
    std::string line;
    int lineNumber = 0;

    while (std::getline(File, line))
    {
        lineNumber++;

        if (lineNumber == n)
            return std::atoi(line.c_str());
    }

    return 0;
}
