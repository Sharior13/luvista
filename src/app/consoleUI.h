#pragma once
#include <string>
#include "../core/playerController.h"

// THE WAITER (the screen and the keyboard)
//
// Draws the text, reads the key you press, and tells the DJ what you asked for.
// It never talks to the speaker directly.

class ConsoleUI {
private:
    PlayerController& player;   // the DJ

    void draw(const std::string& message);   // draws the whole screen

public:
    ConsoleUI(PlayerController& p);

    // Looks for songs in the folder, then runs until you press X
    void run(const std::string& folder);
};
