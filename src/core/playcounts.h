#pragma once
#include <vector>

// How many times each song was played (for "Most played" on the home page).
// The file holds one line per song: id, a tab, the count. It belongs to the profile that is logged in.
class PlayCounts {
public:
    void add(int id);                       // one more play for this song
    int get(int id) const;                  // how many plays (0 if never)
    void remove(int id);                    // forget a song
    std::vector<int> top(int most) const;   // song ids, the most played first (never-played songs are left out)
};
