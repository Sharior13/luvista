#include "PlayQueue.h"

PlayQueue::PlayQueue()
{
    used = 0;
}


// Puts a song at the end of the line.
// Nothing happens when the id is 0 (no such song) or the line is already full.
void PlayQueue::add(int id)
{
    if (id == 0 || used >= capacity)
        return;

    ids[used] = id;
    used++;
}


// Gives back the first song and takes it out of the line.
// Everybody behind it moves one place up (ids[1] becomes ids[0], and so on).
// 0 means the line was empty.
int PlayQueue::take()
{
    if (used == 0)
        return 0;

    int first = ids[0];

    for (int i = 1; i < used; i++)
        ids[i - 1] = ids[i];

    used--;
    return first;
}


// The line becomes empty.
void PlayQueue::clear()
{
    used = 0;
}


// How many songs are still waiting.
int PlayQueue::count() const
{
    return used;
}


// The id on place n (1 = next). 0 when there is no such place.
int PlayQueue::get(int n) const
{
    if (n < 1 || n > used)
        return 0;

    return ids[n - 1];
}


// Takes one song out of the middle of the line. Everybody behind it moves one place up.
bool PlayQueue::remove(int id)
{
    for (int i = 0; i < used; i++) {
        if (ids[i] == id) {
            for (int j = i + 1; j < used; j++)
                ids[j - 1] = ids[j];

            used--;
            return true;
        }
    }
    return false;
}


bool PlayQueue::contains(int id) const
{
    for (int i = 0; i < used; i++) {
        if (ids[i] == id)
            return true;
    }
    return false;
}
