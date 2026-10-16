#include "playlistPicker.h"
#include <algorithm>
#include <cctype>

static std::string lowerCase(std::string text)
{
    for (char& c : text)
        c = (char)std::tolower((unsigned char)c);

    return text;
}


PlaylistPicker::PlaylistPicker(PlayerController& p)
    : player(p)
{
}


void PlaylistPicker::open(int number)
{
    openMany(number != 0 ? std::vector<int>{ number } : std::vector<int>());
}


void PlaylistPicker::openMany(const std::vector<int>& numbers)
{
    songNumbers = numbers;
    search = "";
    savedAtOpen.clear();

    // a playlist counts as "saved" when it holds every one of the songs
    for (int i = 1; i <= player.playlistCount() && !songNumbers.empty(); i++) {
        std::string name = player.playlistName(i);
        bool all = true;

        for (int n : songNumbers) {
            if (player.playlistRowOf(name, n) == 0) {
                all = false;
                break;
            }
        }

        if (all)
            savedAtOpen.push_back(name);
    }
}


void PlaylistPicker::setSearch(const std::string& text)
{
    search = lowerCase(text);
}


int PlaylistPicker::song() const
{
    return songNumbers.empty() ? 0 : songNumbers[0];
}


const std::vector<int>& PlaylistPicker::songList() const
{
    return songNumbers;
}


std::vector<PlaylistRow> PlaylistPicker::rows() const
{
    std::vector<PlaylistRow> result;
    shownNames.clear();

    for (int pass = 0; pass < 2; pass++) {
        for (int i = 1; i <= player.playlistCount(); i++) {
            std::string name = player.playlistName(i);
            bool saved = std::find(savedAtOpen.begin(), savedAtOpen.end(), name) != savedAtOpen.end();

            if (saved != (pass == 0))
                continue;

            if (!search.empty() && lowerCase(name).find(search) == std::string::npos)
                continue;

            bool holdsAll = !songNumbers.empty();

            for (int n : songNumbers) {
                if (player.playlistRowOf(name, n) == 0) {
                    holdsAll = false;
                    break;
                }
            }

            PlaylistRow row;
            row.name = name;
            row.count = player.playlistSongCount(name);
            row.saved = saved;
            row.checked = holdsAll;

            result.push_back(row);
            shownNames.push_back(name);
        }
    }

    return result;
}


std::string PlaylistPicker::toggle(int row)
{
    rows();                                     // makes shownNames match what the window shows

    if (songNumbers.empty() || row < 0 || row >= (int)shownNames.size())
        return "";

    std::string name = shownNames[row];
    bool holdsAll = true;

    for (int n : songNumbers) {
        if (player.playlistRowOf(name, n) == 0)
            holdsAll = false;
    }

    for (int n : songNumbers) {
        int inRow = player.playlistRowOf(name, n);

        if (holdsAll) {
            if (inRow != 0)
                player.removeFromPlaylistRow(name, inRow);
        }
        else if (inRow == 0) {
            player.addToPlaylist(name, n);
        }
    }

    return name;
}


void PlaylistPicker::markSaved(const std::string& name)
{
    savedAtOpen.push_back(name);
}
