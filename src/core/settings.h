#pragma once
#include <string>

// Remembers the music folder between runs, so you only type it once.

// Gives back the saved folder, or "" if nothing was saved yet
std::string loadFolderSetting();

// Saves the folder for next time
void saveFolderSetting(const std::string& folder);
