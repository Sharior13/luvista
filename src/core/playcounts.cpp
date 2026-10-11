#include "playcounts.h"
#include "appPaths.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <string>
#include <utility>

// Where the play counts notebook lives
static std::string countsFile()
{
    return AppPaths::activeFolder() + "/playcounts.dat";
}


// Reads the whole notebook: a list of (id, count)
static std::vector<std::pair<int, int>> readAll()
{
    std::vector<std::pair<int, int>> all;
    std::ifstream in(countsFile());
    std::string line;

    while (std::getline(in, line)) {
        size_t tab = line.find('\t');

        if (tab == std::string::npos)
            continue;

        int id = std::atoi(line.substr(0, tab).c_str());
        int count = std::atoi(line.substr(tab + 1).c_str());

        if (id > 0 && count > 0)
            all.push_back({ id, count });
    }

    return all;
}


static void writeAll(const std::vector<std::pair<int, int>>& all)
{
    std::ofstream out(countsFile());     // this erases the old notebook

    for (const auto& entry : all)
        out << entry.first << '\t' << entry.second << '\n';
}


void PlayCounts::add(int id)
{
    if (id <= 0)
        return;

    std::vector<std::pair<int, int>> all = readAll();
    bool found = false;

    for (auto& entry : all) {
        if (entry.first == id) {
            entry.second++;
            found = true;
        }
    }

    if (!found)
        all.push_back({ id, 1 });

    writeAll(all);
}


int PlayCounts::get(int id) const
{
    for (const auto& entry : readAll()) {
        if (entry.first == id)
            return entry.second;
    }

    return 0;
}


void PlayCounts::remove(int id)
{
    std::vector<std::pair<int, int>> all = readAll();
    std::vector<std::pair<int, int>> kept;

    for (const auto& entry : all) {
        if (entry.first != id)
            kept.push_back(entry);
    }

    if (kept.size() != all.size())
        writeAll(kept);
}


std::vector<int> PlayCounts::top(int most) const
{
    std::vector<std::pair<int, int>> all = readAll();

    // biggest count first. Equal counts keep the order they were first played in
    std::stable_sort(all.begin(), all.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
        });

    std::vector<int> ids;

    for (const auto& entry : all) {
        if ((int)ids.size() >= most)
            break;

        ids.push_back(entry.first);
    }

    return ids;
}
