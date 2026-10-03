#pragma once
#include <string>
#include "../core/playerController.h"

// ConsoleUI is the screen and the keyboard.
// It shows the text, reads the keys, and asks the PlayerController
// to do the work. It never touches the speaker directly.
class ConsoleUI {
private:
    PlayerController& player;

    void draw(const std::string& message);

public:
    ConsoleUI(PlayerController& p);

    // Opens the music folder and runs the player until the user presses X
    void run(const std::string& folder);
};