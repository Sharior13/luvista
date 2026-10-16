#include "selection.h"
#include <algorithm>

void Selection::reset(int rows)
{
    on.assign(rows > 0 ? (size_t)rows : 0, 0);
    total = 0;
    anchor = -1;
}


void Selection::clear()
{
    std::fill(on.begin(), on.end(), (char)0);
    total = 0;
    anchor = -1;
}


int Selection::rows() const { return (int)on.size(); }
int Selection::count() const { return total; }


bool Selection::contains(int row) const
{
    return row >= 0 && row < (int)on.size() && on[row] != 0;
}


std::vector<int> Selection::picked() const
{
    std::vector<int> rows;

    for (int i = 0; i < (int)on.size(); i++) {
        if (on[i])
            rows.push_back(i);
    }

    return rows;
}


void Selection::set(int row, bool value)
{
    if (row < 0 || row >= (int)on.size() || (on[row] != 0) == value)
        return;

    on[row] = value ? 1 : 0;
    total += value ? 1 : -1;
}


void Selection::toggle(int row)
{
    set(row, !contains(row));
}


void Selection::selectAll()
{
    std::fill(on.begin(), on.end(), (char)1);
    total = (int)on.size();
}


std::vector<int> Selection::click(int row, bool ctrl, bool shift)
{
    std::vector<int> changed;

    if (row < 0 || row >= (int)on.size())
        return changed;

    std::vector<char> before = on;

    if (shift && anchor >= 0 && anchor < (int)on.size()) {
        if (!ctrl) {
            std::fill(on.begin(), on.end(), (char)0);
            total = 0;
        }

        int from = std::min(anchor, row);
        int to = std::max(anchor, row);

        for (int i = from; i <= to; i++)
            set(i, true);
        // the anchor stays where it was, so the next shift-click starts from the same row
    }
    else {
        toggle(row);
        anchor = row;
    }

    for (int i = 0; i < (int)on.size(); i++) {
        if (on[i] != before[i])
            changed.push_back(i);
    }

    return changed;
}
