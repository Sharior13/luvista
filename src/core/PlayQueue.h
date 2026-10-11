#pragma once

// The play queue: songs waiting to play next, one after the other.
// It works like a line of people at a shop: the first one in is the first one out.
//
// The queue holds song IDs (the same numbers favorites and playlists use),
// so it stays right even when songs are taken out of the library later.
//
// It is a plain fixed-size array + a counter - no vector, no growing:
//   ids[0] is the song that plays next, ids[1] the one after that, ...
//   used says how many places of the array are filled.
class PlayQueue
{
public:
    PlayQueue();

    void add(int id);        // put a song at the END of the line
    int  take();             // take the FIRST song out and give it back (0 = the line is empty)
    void clear();            // everybody leaves: the line becomes empty
    int  count() const;      // how many songs are still waiting
    int  get(int n) const;   // the id on place n (1 = plays next, 2 = after that, ...)

    static constexpr int capacity = 500;   // the line never holds more than 500 songs

private:
    int ids[capacity];       // the waiting songs: ids[0] is next in line
    int used;                // how many places of the array are filled
};
