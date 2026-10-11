#pragma once
#include <string>
#include <vector>

// The lyrics of one song, read from a ".lrc" file that sits next to the song
// (Hello.mp3 -> Hello.lrc). A line in that file looks like:  [01:23.45] some words
// Lines without a time stamp are ignored, because they cannot be followed along.
class Lyrics {
private:
    struct Line {
        double time;            // seconds into the song when the line starts
        std::string text;
    };

    std::vector<Line> lines;

public:
    // Looks for the .lrc file of a song. True if it holds at least one timed line.
    bool load(const std::string& songPath);
    void clear();

    int count() const;
    double time(int i) const;               // i starts at 0
    std::string text(int i) const;

    // The line being sung at this second. -1 if the song has not reached the first line yet.
    int indexAt(double seconds) const;
};
