#pragma once
#include <string>
#include "../core/playerController.h"

class ConsoleUI {
private:
    PlayerController& player;   // the DJ

    void draw(const std::string& message);   // draws the whole screen
    

public:
    ConsoleUI(PlayerController& p);
    void showFavorites();
    void showRecent();
    // Playlist screens
    std::string createPlaylistScreen();      // asks for a name, makes the playlist
    std::string addToPlaylistScreen();       // picks a playlist, adds the song playing now
    void showPlaylists();                    // lists playlists, open one to see its songs
    // Looks for songs in the folder, then runs until you press X
    void run(const std::string& folder);
};
