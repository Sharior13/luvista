#pragma once
#include <map>
#include <string>
#include "slint.h"
#include "../core/library.h"
#include "../core/playerController.h"

// Turns picture FILES into pictures the screen can draw, and remembers them so a file is read only once.
// Which file belongs to a song or a playlist is not decided here: the audio/core code says so.
class ImageCache {
public:
    ImageCache(TrackList& tracks, PlayerController& player);

    slint::Image file(const std::string& path);                 // empty if the file cannot be read
    void forget(const std::string& path);                       // the file changed on disk: read it again next time
    slint::Image cover(int songNumber);                         // the album picture of a song, empty = the screen draws a gradient
    slint::Image playlist(const std::string& name);             // the chosen picture, or the cover of its first song with one

private:
    TrackList& tracks;
    PlayerController& player;
    std::map<std::string, slint::Image> images;                 // by file path
    std::map<int, std::string> coverPaths;                      // by song id ("" = the song has no picture)
};
