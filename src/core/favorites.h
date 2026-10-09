#pragma once
#include <string>

// Favorite songs. The file holds song ids, one per line.
class Favorite {
public:
    bool contains(int id) const;
    void add(int id);
    void remove(int id);
    void clear();            // empty the whole list

    int count() const;
    int get(int n) const;       // the id on line n (0 if there is no line n)
};
