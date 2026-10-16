#pragma once
#include <string>
#include <vector>
#include "playerController.h"

// The folders the library reads its songs from: the default one (the user's Music folder, always the first,
// cannot be taken off) and the ones the user added. They are kept in music-folders.dat.
class MusicFolders {
public:
    explicit MusicFolders(PlayerController& player);

    void load();                                              // reads the list from the file

    int count() const;                                        // folders in the list, the default one included
    std::string path(int row) const;                          // row 0 is the default folder
    bool isDefault(int row) const;

    bool add(const std::string& folder, std::string& error);  // adds the folder to the list (the program reads it, on another thread). false + a sentence for the screen
    bool remove(int row, std::string& error);                 // its songs leave the library, the files stay on disk

private:
    PlayerController& player;
    std::string defaultFolder;
    std::vector<std::string> added;                           // the user's folders, without the default one

    std::vector<std::string> all() const;
    void save() const;
};
