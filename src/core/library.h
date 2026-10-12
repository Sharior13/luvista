#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "playerController.h"
#include "trackList.h"

// Everything the song pages need to KNOW about the library, without any screen in it:
// the facts about each song, which songs a page holds, and how search and sort treat them.
// The user interface only shows what this class gives back.
enum class Page { AllSongs, Favorites, Playlist, Artist };

struct SongMeta {
    std::string title, artist, album;
    std::string titleLow, artistLow, albumLow;      // the same in small letters, for searching and sorting
    long long added = 0;                            // seconds since 1970
    double seconds = 0;                             // length
};

struct PageResult {
    std::vector<int> rows;      // song numbers, as the page shows them (search and sort applied)
    double seconds = 0;         // together
    int sourceCount = 0;        // songs the page holds before the search (the sidebar number)
};

class Library {
public:
    Library(PlayerController& player, TrackList& tracks);

    void reload();                              // after songs were added or removed. Lengths already read are not read again

    int size() const;
    bool valid(int songNumber) const;
    const SongMeta& meta(int songNumber) const;
    std::string path(int songNumber) const;

    // --- pages. Each page has its own search text and sort; opening a page starts them fresh.
    // sort: 0 = recently added, 1 = title, 2 = artist, 3 = length
    void setSearch(Page page, const std::string& text);
    void setSort(Page page, int sort);
    void resetPage(Page page);
    void setPlaylist(const std::string& name);  // which playlist the Playlist page shows
    void setArtist(const std::string& name);    // which artist the Artist page shows
    void setEverywhereSearch(const std::string& text);   // the search box in the top bar ("" ends it)

    PageResult build(Page page) const;
    std::string queueName(Page page) const;     // what the queue panel calls the page: "Favorites", a playlist name ...
    const std::string& playlistName() const;
    const std::string& artistName() const;
    bool searchingEverywhere() const;

    // --- the home page and the profile page
    std::vector<int> recentlyAdded(int most) const;         // newest first
    std::vector<int> recentlyPlayed() const;                // newest first
    std::vector<int> mostPlayed(int most) const;            // the most played first
    std::vector<char> favoriteFlags() const;                // [n] = 1 when song n is a favorite
    std::vector<std::string> topArtists(int most) const;    // the artists you listen to most
    std::vector<int> songsOfArtist(const std::string& name) const;
    int findSong(const std::string& title, const std::string& artist) const;   // 0 if there is none

    // --- the Storage line of the settings
    std::string folder() const;

private:
    PlayerController& player;
    TrackList& tracks;

    std::vector<SongMeta> songs;                // songs[0] is song number 1
    std::unordered_map<int, double> lengthOfId; // song id -> seconds, so reload() does not read every file again

    struct PageState {
        std::string search;
        int sort = 0;
    };
    PageState pages[4];
    std::string everywhere;                     // the top bar search, small letters
    std::string playlist;
    std::string artistKey;                      // small letters
    std::string artist;

    std::vector<int> source(Page page) const;   // the song numbers of a page before search and sort
};
