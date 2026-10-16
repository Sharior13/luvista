#pragma once
#include <functional>
#include <optional>
#include <vector>
#include "slint.h"

// A list for the screen that builds a row only when the screen asks for it (when the row comes into view).
// A page of ten thousand songs then costs the rows you can see, not ten thousand rows and their pictures.
// Rows are kept once built; refreshRow() throws one away so it is built again.
template <typename T>
class LazyModel : public slint::Model<T> {
public:
    LazyModel(int count, std::function<T(int)> build)
        : total(count > 0 ? count : 0), make(std::move(build)), cache((size_t)(count > 0 ? count : 0))
    {
    }

    size_t row_count() const override { return (size_t)total; }

    std::optional<T> row_data(size_t row) const override
    {
        if (row >= (size_t)total)
            return std::nullopt;

        std::optional<T>& slot = cache[row];

        if (!slot)
            slot = make((int)row);

        return slot;
    }

    void refreshRow(int row)
    {
        if (row < 0 || row >= total)
            return;

        cache[(size_t)row].reset();
        this->notify_row_changed((size_t)row);
    }

    void refreshAll()
    {
        for (int row = 0; row < total; row++)
            refreshRow(row);
    }

private:
    int total;
    std::function<T(int)> make;
    mutable std::vector<std::optional<T>> cache;
};
