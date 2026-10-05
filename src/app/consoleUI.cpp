#include "consoleUI.h"
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <conio.h>
#include <windows.h>

using namespace std;


// The waiter is given the DJ to talk to.
ConsoleUI::ConsoleUI(PlayerController& p) : player(p)
{
}

// Clears the screen and draws everything again.
void ConsoleUI::draw(const string& message)
{
    system("cls");

    cout << "Playing Now: " << player.songName();

    if (player.isFavorite())
        cout << " [Favorite]";

    cout << "  (" << player.currentIndex()
        << "/" << player.total() << ")\n";

    cout << "Time: " << (int)player.position()
        << " / " << (int)player.duration() << " s"
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
        << "[L] Favorite on / off\n"
        << "[A] Show Favorites\n"
        << "[Q] Recently played\n"
        << "[X] Exit\n"
        << "-----------------------\n";

    if (player.lastError() != "")
        cout << "Problem: " << player.lastError() << "\n";

    if (message != "")
        cout << message << "\n";

    cout << ">";

}
void ConsoleUI::showFavorites(){
    system("cls");

    cout << "\n========== FAVORITES ==========\n\n";

    int count = player.favoriteCount();

    if (count == 0) {
        cout << "No favorite songs.\n";
    }
    else {
        for (int i = 1; i <= count; i++) {
            cout << "[" << i << "] "
                << player.favorite(i) << "\n";
        }
    }

    cout << "\nPress any key to return...";
    _getch();
}

void ConsoleUI::showRecent(){
    system("cls");

    cout << "\n========== RECENTLY PLAYED ==========\n";

    int recentCount = player.recentCount();

    if (recentCount == 0) {
        cout << "No recently played songs.\n";
    }
    else {
        for (int i = 1; i <= recentCount; i++) {
            cout << "[" << i << "] "
                << player.recentSong(i) << "\n";
        }
    }

    cout << "\nPress any key to return...";
    _getch();
}

void ConsoleUI::run(const string& folder)
{
    if (!player.openFolder(folder)) {
        cout << "No songs found in: " << folder << "\n"
            << "Check MUSIC_FOLDER in main.cpp.\n"
            << "Press any key to close...";
        _getch();
        return;
    }

    player.next();

    string message = "";

    while (true) {
        player.update();
        draw(message);

        if (_kbhit()) {
            char key = toupper((unsigned char)_getch());

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
            case '=':
                player.volumeUp();
                message = "Volume up";
                break;

            case '-':
                player.volumeDown();
                message = "Volume down";
                break;

            case 'L':
                player.toggleFavorite();
                message = "Favorite changed";
                break;

            case 'A':
                showFavorites();
                message = "";
                break;

            case 'Q':
                showRecent();
                message = "";
                break;

            case 'X':
                system("cls");
                cout << "Bye!" << endl;
                return;

            default:
                message = "Wrong key, try again";
            }
        }

        Sleep(100);
    }
}
