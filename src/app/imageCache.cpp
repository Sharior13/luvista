#include "imageCache.h"
#include "uiConvert.h"
#include "../audio/CoverArt.h"
#include <algorithm>

ImageCache::ImageCache(TrackList& t, PlayerController& p)
    : tracks(t), player(p)
{
}


slint::Image ImageCache::file(const std::string& path)
{
    if (path.empty())
        return slint::Image();

    auto found = images.find(path);

    if (found != images.end())
        return found->second;

    slint::Image picture = slint::Image::load_from_path(UiConvert::toSlint(path));
    images[path] = picture;
    return picture;
}


void ImageCache::forget(const std::string& path)
{
    images.erase(path);
}


slint::Image ImageCache::cover(int number)
{
    int id = tracks.getId(number);

    if (id == 0)
        return slint::Image();

    auto found = coverPaths.find(id);

    if (found == coverPaths.end())
        found = coverPaths.insert({ id, CoverArt::forSong(tracks.getTrack(number), tracks.getFolder()) }).first;

    return file(found->second);
}


slint::Image ImageCache::playlist(const std::string& name)
{
    std::string chosen = player.playlistArt(name);

    if (!chosen.empty())
        return file(chosen);

    int rows = std::min(player.playlistSongCount(name), 5);

    for (int row = 1; row <= rows; row++) {
        slint::Image picture = cover(player.playlistSongNumber(name, row));

        if (picture.size().width > 0)
            return picture;
    }

    return slint::Image();
}
