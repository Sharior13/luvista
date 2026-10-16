#include "library.h"
#include "../audio/AudioInfo.h"
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <unordered_set>

static std::string lowerCase(std::string text)
{
    for (char& c : text)
        c = (char)std::tolower((unsigned char)c);

    return text;
}


int Library::defaultSort(Page page)
{
    // the pages whose order is the point of the page start in that order, the others by title
    if (page == Page::Recent || page == Page::MostPlayed || page == Page::RecentlyAdded)
        return 0;

    return 1;
}


Library::Library(PlayerController& p, TrackList& t)
    : player(p), tracks(t)
{
    for (int i = 0; i < (int)Page::Count; i++)
        pages[i].sort = defaultSort((Page)i);
}


void Library::reload()
{
    songs.clear();
    songs.reserve(tracks.getCount());

    for (int n = 1; n <= tracks.getCount(); n++) {
        SongMeta s;
        s.title = tracks.getTitle(n);
        s.artist = tracks.getArtist(n);
        s.album = tracks.getAlbum(n);
        s.titleLow = lowerCase(s.title);
        s.artistLow = lowerCase(s.artist);
        s.albumLow = lowerCase(s.album);
        s.added = tracks.getAdded(n);
        s.seconds = tracks.getSeconds(n);           // read once, kept in the index. No file is opened here
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
    pages[(int)page].sort = defaultSort(page);

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


void Library::setAlbum(const std::string& name)
{
    album = name;
    albumKey = lowerCase(name);
}


const std::string& Library::playlistName() const { return playlist; }
const std::string& Library::artistName() const { return artist; }
const std::string& Library::albumName() const { return album; }
bool Library::searchingEverywhere() const { return !everywhere.empty(); }


std::string Library::queueName(Page page) const
{
    if (page == Page::AllSongs) return everywhere.empty() ? "All Songs" : "Search results";
    if (page == Page::Favorites) return "Favorites";
    if (page == Page::Artist) return artist.empty() ? "Artist" : artist;
    if (page == Page::Album) return album.empty() ? "Album" : album;
    if (page == Page::Recent) return "Recent activity";
    if (page == Page::MostPlayed) return "Most played";
    if (page == Page::RecentlyAdded) return "Recently added";
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
    else if (page == Page::Album) {
        numbers = songsOfAlbum(album);
    }
    else if (page == Page::Recent) {
        numbers = recentlyPlayed();
    }
    else if (page == Page::MostPlayed) {
        numbers = mostPlayed(size());
    }
    else if (page == Page::RecentlyAdded) {
        for (int n = 1; n <= size(); n++)
            numbers.push_back(n);
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

    // sort 0 is the order of the page itself: newest song first for the song lists, the newest favorite or playlist
    // entry first, and the order they come in for "recently played" and "most played"
    if (state.sort == 0) {
        if (page == Page::Favorites || page == Page::Playlist) {
            std::reverse(result.rows.begin(), result.rows.end());
        }
        else if (page != Page::Recent && page != Page::MostPlayed) {
            std::stable_sort(result.rows.begin(), result.rows.end(), [&](int a, int b) {
                if (songs[a - 1].added != songs[b - 1].added)
                    return songs[a - 1].added > songs[b - 1].added;
                return a < b;
                });
        }
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


std::vector<int> Library::songsOfAlbum(const std::string& name) const
{
    std::string key = lowerCase(name);
    std::vector<int> numbers;

    for (int n = 1; n <= size(); n++) {
        if (songs[n - 1].albumLow == key)
            numbers.push_back(n);
    }

    return numbers;
}


static bool hasNoArtist(const std::string& artistLow)
{
    return artistLow.empty() || artistLow == "unknown" || artistLow == "unknown artist";
}


std::vector<ArtistEntry> Library::artists(const std::string& search, int sort) const
{
    std::string wanted = lowerCase(search);
    std::unordered_map<std::string, size_t> at;
    std::vector<ArtistEntry> list;

    for (int n = 1; n <= size(); n++) {
        const SongMeta& s = songs[n - 1];

        if (hasNoArtist(s.artistLow))
            continue;

        auto found = at.find(s.artistLow);

        if (found == at.end()) {
            found = at.emplace(s.artistLow, list.size()).first;
            ArtistEntry entry;
            entry.name = s.artist;
            entry.firstSong = n;
            list.push_back(entry);
        }

        list[found->second].songs++;
    }

    if (!wanted.empty()) {
        list.erase(std::remove_if(list.begin(), list.end(), [&](const ArtistEntry& e) {
            return lowerCase(e.name).find(wanted) == std::string::npos;
            }), list.end());
    }

    std::stable_sort(list.begin(), list.end(), [&](const ArtistEntry& a, const ArtistEntry& b) {
        if (sort == 1 && a.songs != b.songs)
            return a.songs > b.songs;
        return lowerCase(a.name) < lowerCase(b.name);
        });

    return list;
}


int Library::artistCount() const
{
    std::unordered_set<std::string> names;

    for (const SongMeta& s : songs) {
        if (!hasNoArtist(s.artistLow))
            names.insert(s.artistLow);
    }

    return (int)names.size();
}


static bool hasNoAlbum(const std::string& albumLow)
{
    return albumLow.empty() || albumLow == "unknown album" || albumLow == "-";
}


std::vector<AlbumEntry> Library::albums(const std::string& search, int sort) const
{
    std::string wanted = lowerCase(search);
    std::unordered_map<std::string, size_t> at;         // album name (small letters) -> place in `list`
    std::vector<AlbumEntry> list;
    std::vector<std::string> artistKeys;                 // the artist all songs of the album share so far, "\n" = they differ

    for (int n = 1; n <= size(); n++) {
        const SongMeta& s = songs[n - 1];

        if (hasNoAlbum(s.albumLow))
            continue;

        auto found = at.find(s.albumLow);

        if (found == at.end()) {
            found = at.emplace(s.albumLow, list.size()).first;

            AlbumEntry entry;
            entry.name = s.album;
            entry.artist = s.artist;
            entry.firstSong = n;
            list.push_back(entry);
            artistKeys.push_back(s.artistLow);
        }

        AlbumEntry& entry = list[found->second];
        entry.songs++;
        entry.seconds += s.seconds;
        entry.added = std::max(entry.added, s.added);

        if (artistKeys[found->second] != s.artistLow && artistKeys[found->second] != "\n") {
            artistKeys[found->second] = "\n";
            entry.artist = "Various artists";
        }
    }

    if (!wanted.empty()) {
        list.erase(std::remove_if(list.begin(), list.end(), [&](const AlbumEntry& e) {
            return lowerCase(e.name).find(wanted) == std::string::npos &&
                lowerCase(e.artist).find(wanted) == std::string::npos;
            }), list.end());
    }

    std::stable_sort(list.begin(), list.end(), [&](const AlbumEntry& a, const AlbumEntry& b) {
        if (sort == 1) return lowerCase(a.artist) < lowerCase(b.artist);
        if (sort == 2) return a.added > b.added;
        return lowerCase(a.name) < lowerCase(b.name);
        });

    return list;
}


int Library::albumCount() const
{
    std::unordered_set<std::string> names;

    for (const SongMeta& s : songs) {
        if (!hasNoAlbum(s.albumLow))
            names.insert(s.albumLow);
    }

    return (int)names.size();
}


std::string Library::albumArtist(const std::string& name) const
{
    std::string key = lowerCase(name);
    std::string who;
    bool first = true;

    for (const SongMeta& s : songs) {
        if (s.albumLow != key)
            continue;

        if (first) {
            who = s.artist;
            first = false;
        }
        else if (lowerCase(who) != s.artistLow) {
            return "Various artists";
        }
    }

    return who;
}


std::vector<int> Library::recentlyAdded(int most) const
{
    std::vector<int> numbers;

    for (int n = 1; n <= size(); n++)
        numbers.push_back(n);

    // newest first by the day it was added, a song found later wins a tie
    std::stable_sort(numbers.begin(), numbers.end(), [&](int a, int b) {
        if (songs[a - 1].added != songs[b - 1].added)
            return songs[a - 1].added > songs[b - 1].added;
        return a < b;
        });

    if ((int)numbers.size() > most)
        numbers.resize(most);

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
    std::unordered_map<std::string, size_t> at;         // artist (small letters) -> place in names / score

    auto add = [&](int n, long long points) {
        const SongMeta& s = songs[n - 1];

        if (s.artistLow.empty() || s.artistLow == "unknown" || s.artistLow == "unknown artist")
            return;

        auto found = at.find(s.artistLow);

        if (found != at.end()) {
            score[found->second] += points;
            return;
        }

        at.emplace(s.artistLow, names.size());
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
