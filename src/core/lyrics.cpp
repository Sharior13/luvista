#include "lyrics.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <system_error>

namespace fs = std::filesystem;


// Cuts the spaces off both ends.
static std::string trim(const std::string& s)
{
    size_t first = s.find_first_not_of(" \t\r\n");

    if (first == std::string::npos)
        return "";

    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}


// "01:23.45" -> 83.45. False if the text is not a time ("ar:Adele" is not).
static bool parseTime(const std::string& tag, double& seconds)
{
    size_t colon = tag.find(':');

    if (colon == std::string::npos || colon == 0)
        return false;

    for (size_t i = 0; i < tag.size(); i++) {
        char c = tag[i];

        if (!(c >= '0' && c <= '9') && c != ':' && c != '.')
            return false;
    }

    double minutes = std::atof(tag.substr(0, colon).c_str());
    double rest = std::atof(tag.substr(colon + 1).c_str());

    seconds = minutes * 60.0 + rest;
    return true;
}


bool Lyrics::load(const std::string& songPath)
{
    lines.clear();

    // Hello.mp3 -> Hello.lrc
    fs::path lrc = fs::path(songPath).replace_extension(".lrc");
    std::error_code problem;

    if (!fs::exists(lrc, problem)) {
        lrc.replace_extension(".LRC");

        if (!fs::exists(lrc, problem))
            return false;
    }

    std::ifstream in(lrc, std::ios::binary);
    std::string line;
    double offset = 0;              // [offset:+500] moves every line by that many milliseconds

    while (std::getline(in, line)) {
        // a file saved with a byte-order mark starts with three odd bytes
        if (line.compare(0, 3, "\xEF\xBB\xBF") == 0)
            line = line.substr(3);

        line = trim(line);

        // read the [..] tags at the front of the line
        std::vector<double> times;
        size_t at = 0;

        while (at < line.size() && line[at] == '[') {
            size_t close = line.find(']', at);

            if (close == std::string::npos)
                break;

            std::string tag = line.substr(at + 1, close - at - 1);
            double seconds = 0;

            if (parseTime(tag, seconds)) {
                times.push_back(seconds);
            }
            else if (tag.compare(0, 7, "offset:") == 0) {
                offset = std::atof(tag.substr(7).c_str()) / 1000.0;
            }

            at = close + 1;
        }

        std::string text = trim(line.substr(at));

        // one line can start at several times: "[00:10.00][01:30.00] chorus"
        if (text == "")
            continue;

        for (double t : times)
            lines.push_back({ t, text });
    }

    // the offset tag may sit anywhere in the file, so it is applied once everything is read
    // (a positive offset makes the lyrics come sooner, as the lrc format says)
    for (Line& l : lines) {
        l.time -= offset;

        if (l.time < 0)
            l.time = 0;
    }

    std::stable_sort(lines.begin(), lines.end(), [](const Line& a, const Line& b) {
        return a.time < b.time;
        });

    return !lines.empty();
}


void Lyrics::clear()
{
    lines.clear();
}


int Lyrics::count() const
{
    return (int)lines.size();
}


double Lyrics::time(int i) const
{
    if (i < 0 || i >= (int)lines.size())
        return 0;

    return lines[i].time;
}


std::string Lyrics::text(int i) const
{
    if (i < 0 || i >= (int)lines.size())
        return "";

    return lines[i].text;
}


// The last line that has already started.
int Lyrics::indexAt(double seconds) const
{
    int found = -1;
    int low = 0;
    int high = (int)lines.size() - 1;

    while (low <= high) {
        int middle = (low + high) / 2;

        if (lines[middle].time <= seconds) {
            found = middle;
            low = middle + 1;
        }
        else {
            high = middle - 1;
        }
    }

    return found;
}
