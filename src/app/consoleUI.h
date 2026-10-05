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
    // Looks for songs in the folder, then runs until you press X
    void run(const std::string& folder);
};
