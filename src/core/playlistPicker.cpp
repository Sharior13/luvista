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
    songNumber = number;
    search = "";
    savedAtOpen.clear();

    if (number != 0)
        savedAtOpen = player.playlistsWithSong(number);
}


void PlaylistPicker::setSearch(const std::string& text)
{
    search = lowerCase(text);
}


int PlaylistPicker::song() const
{
    return songNumber;
}


std::vector<PlaylistRow> PlaylistPicker::rows() const
{
    std::vector<PlaylistRow> result;
    std::vector<std::string> holding = songNumber != 0 ? player.playlistsWithSong(songNumber) : std::vector<std::string>();
    shownNames.clear();

    for (int pass = 0; pass < 2; pass++) {
        for (int i = 1; i <= player.playlistCount(); i++) {
            std::string name = player.playlistName(i);
            bool saved = std::find(savedAtOpen.begin(), savedAtOpen.end(), name) != savedAtOpen.end();

            if (saved != (pass == 0))
                continue;

            if (!search.empty() && lowerCase(name).find(search) == std::string::npos)
                continue;

            PlaylistRow row;
            row.name = name;
            row.count = player.playlistSongCount(name);
            row.saved = saved;
            row.checked = std::find(holding.begin(), holding.end(), name) != holding.end();

            result.push_back(row);
            shownNames.push_back(name);
        }
    }

    return result;
}


std::string PlaylistPicker::toggle(int row)
{
    rows();                                     // makes shownNames match what the window shows

    if (songNumber == 0 || row < 0 || row >= (int)shownNames.size())
        return "";

    std::string name = shownNames[row];
    int inRow = player.playlistRowOf(name, songNumber);

    if (inRow != 0)
        player.removeFromPlaylistRow(name, inRow);
    else
        player.addToPlaylist(name, songNumber);

    return name;
}


void PlaylistPicker::markSaved(const std::string& name)
{
    savedAtOpen.push_back(name);
}
