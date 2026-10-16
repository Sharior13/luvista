#pragma once
#include <string>

// Finds the album picture of a song.
//   1. a picture inside the file (mp3: ID3 "APIC", m4a: "covr") is cut out once and kept in
//      the covers folder, named after the song's path, so later runs only look for that file.
//   2. otherwise a picture next to the song: cover.jpg, folder.jpg, front.png, album.jpg ... but only in
//      an album folder (a folder full of loose songs does not have "an album picture").
// Only jpg and png are used, because that is what the screen can show.
namespace CoverArt
{
    // The path of an image file, or "" if the song has no picture. libraryRoot is the folder the library
    // scans: pictures lying in it are not used, they would belong to every loose song there.
    std::string forSong(const std::string& songPath, const std::string& libraryRoot);

    // Deletes pictures from the old days when they were kept under the song's number. Safe to call on every start.
    void purgeNumbered();

    // Takes the picture out of the song's tags and saves it to outPath (the right ending is added).
    // Gives back the real path of the saved file, or "" if the song has no picture inside.
    std::string extract(const std::string& songPath, const std::string& outPathWithoutEnding);
}
