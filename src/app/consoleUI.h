#pragma once
#include <string>
#include "../core/playerController.h"
#include "../audio/Recorder.h"

// Text UI: draw(), a screen per feature, keys bound in run().
class ConsoleUI {
private:
    PlayerController& player;
    Recorder recorder;          // microphone recorder

    void draw(const std::string& message);   // clear the screen and redraw everything
    std::string askSearch();                 // search words (empty = show everything)
    std::string describe(int songNumber);    // "Title  |  Artist  |  Album" for one row

public:
    ConsoleUI(PlayerController& p);
    void showFavorites();
    void showRecent();
    std::string createPlaylistScreen();      // ask a name, make the playlist
    std::string addToPlaylistScreen();       // pick a playlist, add the song playing now
    std::string deleteScreen();              // delete a playlist / remove songs / clear favorites
    void showPlaylists();                    // list playlists, open one to see its songs
    std::string searchSongsScreen();         // search all songs, pick one to play
    std::string toggleRecording(const std::string& folder);  // record on / off (microphone)
    // scan the folder, then run until X is pressed
    void run(const std::string& folder);
};
