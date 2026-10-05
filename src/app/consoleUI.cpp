#include "consoleUI.h"
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <conio.h>   // _getch() reads one key at once, no Enter needed

using namespace std;


// The waiter is given the DJ to talk to.
ConsoleUI::ConsoleUI(PlayerController& p) : player(p)
{
}


// Clears the screen and draws everything again.
void ConsoleUI::draw(const string& message)
{
    system("cls");   // clear the screen

    cout << "\nPlaying Now: " << player.songName()
         << "  (" << player.currentIndex() << "/" << player.total() << ")\n";

    cout << "Time: " << (int)player.position() << " / " << (int)player.duration() << " s"
         << "   Volume: " << player.volume() << "%\n";

    cout << "-----------------------\n"
         << "[N] Next song\n"
         << "[V] Previous song\n"
         << "[P] Pause\n"
         << "[R] Resume / Play\n"
         << "[S] ReStart\n"
         << "[F] Forward 10s\n"
         << "[B] Back 10s\n"
         << "[+] Volume Up\n"
         << "[-] Volume Down\n"
         << "[X] Exit\n"
         << "-----------------------\n";

    if (player.lastError() != "")
        cout << "Problem: " << player.lastError() << "\n";

    if (message != "")
        cout << message << "\n";

    cout << ">";
}


// The main loop of the program:
//   check song finished -> draw -> wait for a key -> do what the key says -> repeat
void ConsoleUI::run(const string& folder)
{
    // no songs? say so and stop
    if (!player.openFolder(folder)) {
        cout << "No songs found in: " << folder << "\n"
             << "Check MUSIC_FOLDER in main.cpp.\n"
             << "Press any key to close...";
        _getch();
        return;
    }

    player.next();   // starts song 1

    string message = "";
    char key = ' ';

    while (key != 'X') {
        player.update();   // if the song finished, start the next one
        draw(message);

        key = toupper((unsigned char)_getch());   // wait for a key. 'p' and 'P' both work

        switch (key) {
        case 'N':
            player.next();
            message = "Next song";
            break;
        case 'V':
            player.previous();
            message = "Previous song";
            break;
        case 'P':
            player.pause();
            message = "Paused";
            break;
        case 'R':
            player.resume();
            message = "Playing";
            break;
        case 'S':
            player.restart();
            message = "Restarted";
            break;
        case 'F':
            player.forward();
            message = "Forward 10s";
            break;
        case 'B':
            player.back();
            message = "Back 10s";
            break;

        case '+':
        case '=':                     // '=' is the same key as '+', so no Shift needed
            player.volumeUp();
            message = "Volume up";
            break;

        case '-':
            player.volumeDown();
            message = "Volume down";
            break;

        case 'X':
            break;                    // the loop ends

        default:
            message = "Wrong key, try again";
        }
    }

    system("cls");
    cout << "Bye!" << endl;
}
