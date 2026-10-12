#include "recordings.h"
#include "../audio/AudioInfo.h"
#include "appPaths.h"
#include <cctype>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

void Recordings::refresh()
{
    entries.clear();

    std::error_code problem;
    fs::path folder(AppPaths::recordingsFolder());

    if (!fs::is_directory(folder, problem))
        return;

    for (fs::directory_iterator it(folder, problem), end; !problem && it != end; it.increment(problem)) {
        std::error_code e;

        if (!it->is_regular_file(e) || it->path().extension() != ".mp3")
            continue;

        Entry entry;
        try {
            entry.path = it->path().string();
            entry.title = it->path().stem().string();
        }
        catch (const std::system_error&) {
            continue;                 // a file name the plain-text string cannot hold: skip it
        }

        // file time -> seconds since 1970
        auto stamp = fs::last_write_time(it->path(), e);
        auto sys = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            stamp - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        entry.added = e ? 0 : (long long)std::chrono::system_clock::to_time_t(sys);
        entry.seconds = AudioInfo::seconds(entry.path);
        entries.push_back(entry);
    }

    std::stable_sort(entries.begin(), entries.end(),
        [](const Entry& a, const Entry& b) { return a.added > b.added; });
}

int Recordings::count() const { return (int)entries.size(); }

std::string Recordings::path(int row) const
{
    return row >= 1 && row <= count() ? entries[row - 1].path : "";
}

std::string Recordings::title(int row) const
{
    return row >= 1 && row <= count() ? entries[row - 1].title : "";
}

long long Recordings::added(int row) const
{
    return row >= 1 && row <= count() ? entries[row - 1].added : 0;
}

double Recordings::seconds(int row) const
{
    return row >= 1 && row <= count() ? entries[row - 1].seconds : 0.0;
}

bool Recordings::remove(int row)
{
    if (row < 1 || row > count())
        return false;

    std::error_code problem;
    bool ok = fs::remove(fs::path(entries[row - 1].path), problem);

    refresh();
    return ok && !problem;
}


static std::string lowerCase(std::string text)
{
    for (char& c : text)
        c = (char)std::tolower((unsigned char)c);

    return text;
}


std::vector<int> Recordings::rows(const std::string& search, int sort) const
{
    std::string wanted = lowerCase(search);
    std::vector<int> shown;

    for (int row = 1; row <= count(); row++) {
        if (wanted.empty() || lowerCase(entries[row - 1].title).find(wanted) != std::string::npos)
            shown.push_back(row);
    }

    std::stable_sort(shown.begin(), shown.end(), [&](int a, int b) {
        const Entry& x = entries[a - 1];
        const Entry& y = entries[b - 1];

        if (sort == 1)
            return lowerCase(x.title) < lowerCase(y.title);
        if (sort == 2)
            return x.seconds > y.seconds;
        return x.added > y.added;               // newest first
        });

    return shown;
}


double Recordings::totalSeconds(const std::vector<int>& shown) const
{
    double sum = 0;

    for (int row : shown)
        sum += seconds(row);

    return sum;
}
