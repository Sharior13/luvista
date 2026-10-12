#pragma once
#include <string>

// Questions about a sound file that do not need the player.
namespace AudioInfo
{
    // how long the file is, in seconds. 0 if it cannot be read
    double seconds(const std::string& path);
}
