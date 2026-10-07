#pragma once

#include <string>

// Recently played songs (newest first). The file holds song ids.
class RecentList
{
public:
    void add(int id);
    bool contains(int id) const;
    int count() const;
    int get(int n) const;       // the id on line n (0 if there is no line n)
};
