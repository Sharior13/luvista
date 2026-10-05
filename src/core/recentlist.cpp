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

void RecentList::add(const std::string& path)
{
    if (contains(path))
        return;

    std::ofstream File(RECENT_FILE, std::ios::app);
    File << path << "\n";
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