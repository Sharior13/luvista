#pragma once
#include <string>

// Finds the album picture of a song.
//   1. a picture inside the file (mp3: ID3 "APIC", m4a: "covr") is cut out once and kept in
//      the covers folder, named after the song id, so later runs only look for that file.
//   2. otherwise a picture next to the song: cover.jpg, folder.jpg, front.png, album.jpg ...
// Only jpg and png are used, because that is what the screen can show.
namespace CoverArt
{
    // The path of an image file, or "" if the song has no picture.
    std::string forSong(int songId, const std::string& songPath);

    // Takes the picture out of the song's tags and saves it to outPath (the right ending is added).
    // Gives back the real path of the saved file, or "" if the song has no picture inside.
    std::string extract(const std::string& songPath, const std::string& outPathWithoutEnding);
}
