#include "recentList.h"
#include <fstream>
#include <string>

static const char* RECENT_FILE = "recent-index.dat";

bool RecentList::contains(const std::string& path) const
{
    std::ifstream File(RECENT_FILE);
    std::string line;

    while (std::getline(File, line))
    {
        if (line == path)
            return true;
    }

    return false;
}

// Puts the song at the TOP of the list (newest first).
// If the song was already there, the old copy is dropped.
// The list never grows past 10 songs.
void RecentList::add(const std::string& path)
{
    std::string newList = path + "\n";   // newest song goes first
    int kept = 1;

    std::ifstream in(RECENT_FILE);
    std::string line;

    while (kept < 10 && std::getline(in, line))
    {
        if (line != path)                // skip the old copy of this song
        {
            newList += line + "\n";
            kept++;
        }
    }
    in.close();                          // finish reading before writing

    std::ofstream out(RECENT_FILE);      // this erases the old file
    out << newList;
}

int RecentList::count() const
{
    std::ifstream File(RECENT_FILE);
    std::string line;
    int lines = 0;

    while (std::getline(File, line))
        lines++;

    return lines;
}

std::string RecentList::get(int n) const
{
    std::ifstream File(RECENT_FILE);
    std::string line;
    int lineNumber = 0;

    while (std::getline(File, line))
    {
        lineNumber++;

        if (lineNumber == n)
            return line;
    }

    return "";
}
