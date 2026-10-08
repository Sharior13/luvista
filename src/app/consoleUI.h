#pragma once
#include <string>
#include "../core/playerController.h"

class ConsoleUI {
private:
    PlayerController& player;   // the DJ

    void draw(const std::string& message);   // draws the whole screen
    std::string askSearch();                 // asks for search words (empty = show everything)
    std::string describe(int songNumber);    // "Title  |  Artist  |  Album" for one row

public:
    ConsoleUI(PlayerController& p);
    void showFavorites();
    void showRecent();
    // Playlist screens
    std::string createPlaylistScreen();      // asks for a name, makes the playlist
    std::string addToPlaylistScreen();       // picks a playlist, adds the song playing now
    std::string deleteScreen();              // deletes a playlist, or takes this song out of one
    void showPlaylists();                    // lists playlists, open one to see its songs
    std::string searchSongsScreen();         // searches all songs, pick one to play
    // Looks for songs in the folder, then runs until you press X
    void run(const std::string& folder);
};