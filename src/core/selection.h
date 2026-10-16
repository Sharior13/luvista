#pragma once
#include <vector>

// Which rows of a song list are ticked ("select several songs"). Rows are numbered from 0, as the screen shows them.
// It knows nothing about songs or the screen: the program turns the rows into songs when an action is chosen.
class Selection {
public:
    void reset(int rows);                       // a list was made again with this many rows: nothing is ticked
    void clear();

    int rows() const;
    int count() const;
    bool contains(int row) const;
    std::vector<int> picked() const;            // the ticked rows, smallest first

    void set(int row, bool on);
    void toggle(int row);
    void selectAll();

    // A click on a row while selecting. ctrl: add or take away just this row. shift: everything from the last
    // clicked row to this one (it replaces the selection unless ctrl is held too). Neither: toggles the row.
    // Gives the rows whose tick changed, so the screen can redraw only those.
    std::vector<int> click(int row, bool ctrl, bool shift);

private:
    std::vector<char> on;
    int total = 0;
    int anchor = -1;                            // the row the last plain or ctrl click was on
};
