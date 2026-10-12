#include "library.h"
#include "../audio/AudioInfo.h"
#include <algorithm>
#include <cctype>

static std::string lowerCase(std::string text)
{
    for (char& c : text)
        c = (char)std::tolower((unsigned char)c);

    return text;
}


Library::Library(PlayerController& p, TrackList& t)
    : player(p), tracks(t)
{
}


void Library::reload()
{
    songs.clear();

    for (int n = 1; n <= tracks.getCount(); n++) {
        SongMeta s;
        s.title = tracks.getTitle(n);
        s.artist = tracks.getArtist(n);
        s.album = tracks.getAlbum(n);
        s.titleLow = lowerCase(s.title);
        s.artistLow = lowerCase(s.artist);
        s.albumLow = lowerCase(s.album);
        s.added = tracks.getAdded(n);

        int id = tracks.getId(n);
        auto known = lengthOfId.find(id);

        if (known == lengthOfId.end())
            known = lengthOfId.emplace(id, AudioInfo::seconds(tracks.getTrack(n))).first;

        s.seconds = known->second;
        songs.push_back(s);
    }
}


int Library::size() const { return (int)songs.size(); }
bool Library::valid(int n) const { return n >= 1 && n <= (int)songs.size(); }


const SongMeta& Library::meta(int n) const
{
    static const SongMeta none;
    return valid(n) ? songs[n - 1] : none;
}


std::string Library::path(int n) const { return tracks.getTrack(n); }
std::string Library::folder() const { return tracks.getFolder(); }


void Library::setSearch(Page page, const std::string& text)
{
    pages[(int)page].search = lowerCase(text);

    if (page == Page::AllSongs)
        everywhere = "";                        // the page's own search box takes over from the top bar
}


void Library::setSort(Page page, int sort)
{
    pages[(int)page].sort = sort;
}


void Library::resetPage(Page page)
{
    pages[(int)page] = PageState();

    if (page == Page::AllSongs)
        everywhere = "";
}


void Library::setPlaylist(const std::string& name) { playlist = name; }


void Library::setArtist(const std::string& name)
{
    artist = name;
    artistKey = lowerCase(name);
}


void Library::setEverywhereSearch(const std::string& text)
{
    everywhere = lowerCase(text);
    pages[(int)Page::AllSongs].search = everywhere;
}


const std::string& Library::playlistName() const { return playlist; }
const std::string& Library::artistName() const { return artist; }
bool Library::searchingEverywhere() const { return !everywhere.empty(); }


std::string Library::queueName(Page page) const
{
    if (page == Page::AllSongs) return everywhere.empty() ? "All Songs" : "Search results";
    if (page == Page::Favorites) return "Favorites";
    if (page == Page::Artist) return artist.empty() ? "Artist" : artist;
    return playlist.empty() ? "Playlist" : playlist;
}


std::vector<int> Library::source(Page page) const
{
    std::vector<int> numbers;

    if (page == Page::AllSongs) {
        for (int n = 1; n <= size(); n++)
            numbers.push_back(n);
    }
    else if (page == Page::Favorites) {
        for (int row = 1; row <= player.favoriteCount(); row++) {
            int n = player.favoriteSongNumber(row);

            if (n > 0)
                numbers.push_back(n);
        }
    }
    else if (page == Page::Artist) {
        numbers = songsOfArtist(artist);
    }
    else if (!playlist.empty()) {
        for (int row = 1; row <= player.playlistSongCount(playlist); row++) {
            int n = player.playlistSongNumber(playlist, row);

            if (n > 0)
                numbers.push_back(n);
        }
    }

    return numbers;
}


PageResult Library::build(Page page) const
{
    const PageState& state = pages[(int)page];
    PageResult result;
    std::vector<int> all = source(page);
    result.sourceCount = (int)all.size();

    // while the top bar is searching, the songs of a playlist whose name has the text are hits too
    std::vector<char> inMatchingPlaylist(songs.size() + 1, 0);

    if (page == Page::AllSongs && !everywhere.empty()) {
        for (int i = 1; i <= player.playlistCount(); i++) {
            std::string name = player.playlistName(i);

            if (lowerCase(name).find(everywhere) == std::string::npos)
                continue;

            for (int row = 1; row <= player.playlistSongCount(name); row++) {
                int n = player.playlistSongNumber(name, row);

                if (valid(n))
                    inMatchingPlaylist[n] = 1;
            }
        }
    }

    // search: the text is in the title, the artist or the album
    for (int n : all) {
        const SongMeta& s = songs[n - 1];

        if (state.search.empty() || inMatchingPlaylist[n] ||
            s.titleLow.find(state.search) != std::string::npos ||
            s.artistLow.find(state.search) != std::string::npos ||
            s.albumLow.find(state.search) != std::string::npos)
            result.rows.push_back(n);
    }

    // sort: "recently added" is the list turned around, the others sort by words or by length
    if (state.sort == 0) {
        std::reverse(result.rows.begin(), result.rows.end());
    }
    else {
        std::stable_sort(result.rows.begin(), result.rows.end(), [&](int a, int b) {
            if (state.sort == 1) return songs[a - 1].titleLow < songs[b - 1].titleLow;
            if (state.sort == 2) return songs[a - 1].artistLow < songs[b - 1].artistLow;
            return songs[a - 1].seconds < songs[b - 1].seconds;
            });
    }

    for (int n : result.rows)
        result.seconds += songs[n - 1].seconds;

    return result;
}


std::vector<int> Library::songsOfArtist(const std::string& name) const
{
    std::string key = lowerCase(name);
    std::vector<int> numbers;

    for (int n = 1; n <= size(); n++) {
        if (songs[n - 1].artistLow == key)
            numbers.push_back(n);
    }

    return numbers;
}


std::vector<int> Library::recentlyAdded(int most) const
{
    std::vector<int> numbers;

    for (int n = size(); n >= 1 && (int)numbers.size() < most; n--)
        numbers.push_back(n);

    return numbers;
}


std::vector<int> Library::mostPlayed(int most) const
{
    return player.mostPlayedSongNumbers(most);
}


// Index n is 1 when song n is a favorite. One pass over the favorites, so a page can mark many rows cheaply.
std::vector<char> Library::favoriteFlags() const
{
    std::vector<char> flags(songs.size() + 1, 0);

    for (int row = 1; row <= player.favoriteCount(); row++) {
        int n = player.favoriteSongNumber(row);

        if (valid(n))
            flags[n] = 1;
    }

    return flags;
}


std::vector<int> Library::recentlyPlayed() const
{
    std::vector<int> numbers;

    for (int row = 1; row <= player.recentCount(); row++) {
        int n = player.recentSongNumber(row);

        if (valid(n))
            numbers.push_back(n);
    }

    return numbers;
}


// The artists with the most plays. Songs in the library, plays and the recently played list all count.
std::vector<std::string> Library::topArtists(int most) const
{
    std::vector<std::string> names;
    std::vector<long long> score;

    auto add = [&](int n, long long points) {
        const SongMeta& s = songs[n - 1];

        if (s.artistLow.empty() || s.artistLow == "unknown" || s.artistLow == "unknown artist")
            return;

        for (size_t i = 0; i < names.size(); i++) {
            if (lowerCase(names[i]) == s.artistLow) {
                score[i] += points;
                return;
            }
        }

        names.push_back(s.artist);
        score.push_back(points);
    };

    for (int n = 1; n <= size(); n++)
        add(n, 1 + 1000LL * player.playCount(n));

    for (int n : recentlyPlayed())
        add(n, 1000);

    std::vector<int> order(names.size());

    for (size_t i = 0; i < order.size(); i++)
        order[i] = (int)i;

    std::stable_sort(order.begin(), order.end(), [&](int a, int b) { return score[a] > score[b]; });

    std::vector<std::string> top;

    for (size_t i = 0; i < order.size() && (int)i < most; i++)
        top.push_back(names[order[i]]);

    return top;
}


int Library::findSong(const std::string& title, const std::string& artist) const
{
    for (int n = 1; n <= size(); n++) {
        if (songs[n - 1].title == title && songs[n - 1].artist == artist)
            return n;
    }

    return 0;
}
