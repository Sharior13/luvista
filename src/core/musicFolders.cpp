#include "musicFolders.h"
#include "appPaths.h"
#include "pathUtil.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

MusicFolders::MusicFolders(PlayerController& p)
    : player(p)
{
    defaultFolder = AppPaths::defaultMusicFolder();
}


std::vector<std::string> MusicFolders::all() const
{
    std::vector<std::string> list;

    if (!defaultFolder.empty())
        list.push_back(defaultFolder);

    list.insert(list.end(), added.begin(), added.end());
    return list;
}


void MusicFolders::load()
{
    added.clear();
    std::ifstream in(AppPaths::musicFoldersFile());
    std::string line;

    while (std::getline(in, line)) {
        bool known = line.empty() || PathUtil::same(line, defaultFolder);

        for (const std::string& other : added)
            known = known || PathUtil::same(line, other);

        if (!known)
            added.push_back(line);
    }
}


void MusicFolders::save() const
{
    std::ofstream out(AppPaths::musicFoldersFile(), std::ios::trunc);

    for (const std::string& folder : added)
        out << folder << "\n";
}


void MusicFolders::rescanAll()
{
    for (const std::string& folder : all())
        player.openFolder(folder);              // a folder that is not there is skipped, its songs stay in the library
}


int MusicFolders::count() const
{
    return (int)all().size();
}


std::string MusicFolders::path(int row) const
{
    std::vector<std::string> list = all();

    return row >= 0 && row < (int)list.size() ? list[row] : "";
}


bool MusicFolders::isDefault(int row) const
{
    return row == 0 && !defaultFolder.empty();
}


bool MusicFolders::add(const std::string& folder, std::string& error)
{
    std::error_code problem;

    if (folder.empty() || !fs::is_directory(folder, problem)) {
        error = "That folder cannot be found";
        return false;
    }

    for (const std::string& other : all()) {
        if (PathUtil::same(folder, other)) {
            error = "That folder is already in the list";
            return false;
        }
    }

    added.push_back(folder);
    save();
    player.openFolder(folder, true);            // picked on purpose: songs removed from it earlier come back
    error = "";
    return true;
}


bool MusicFolders::remove(int row, std::string& error)
{
    if (row < 0 || row >= count()) {
        error = "That folder is not in the list";
        return false;
    }

    if (isDefault(row)) {
        error = "The default folder cannot be removed";
        return false;
    }

    std::string folder = path(row);

    added.erase(added.begin() + (row - (defaultFolder.empty() ? 0 : 1)));
    save();
    player.forgetSongsInside(folder, all());
    error = "";
    return true;
}
