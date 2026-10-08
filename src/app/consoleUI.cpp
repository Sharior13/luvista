#include "consoleUI.h"
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <conio.h>
#include <windows.h>


// The waiter is given the DJ to talk to.
ConsoleUI::ConsoleUI(PlayerController& p) : player(p)
{
}

// Asks for search words. Empty answer = no filter (show everything).
std::string ConsoleUI::askSearch()
{
    std::cout << "Search (empty = show all): ";

    std::string query;
    std::getline(std::cin, query);
    return query;
}


// One line of text for a song: Title  |  Artist  |  Album
std::string ConsoleUI::describe(int songNumber)
{
    return player.songTitle(songNumber) + "  |  " +
        player.songArtist(songNumber) + "  |  " +
        player.songAlbum(songNumber);
}


// Clears the screen and draws everything again.
void ConsoleUI::draw(const std::string& message)
{
    system("cls");

    std::cout << "Playing Now: " << player.songName();

    if (player.currentIndex() != 0)
        std::cout << " - " << player.songArtist(player.currentIndex());

    if (player.isFavorite())
        std::cout << " [Favorite]";

    std::cout << "  (" << player.currentIndex()
        << "/" << player.total() << ")\n";

    std::cout << "Time: " << (int)player.position()
        << " / " << (int)player.duration() << " s"
        << "   Volume: " << player.volume() << "%\n";

    std::cout << "-----------------------\n"
        << "[N] Next song\n"
        << "[V] Previous song\n"
        << "[P] Pause\n"
        << "[R] Resume / Play\n"
        << "[Space] Pause / Play (one button)\n"
        << "[S] ReStart\n"
        << "[F] Forward 10s\n"
        << "[B] Back 10s\n"
        << "[+] Volume Up\n"
        << "[-] Volume Down\n"
        << "[L] Favorite on / off\n"
        << "[G] Search all songs\n"
        << "[A] Show Favorites\n"
        << "[Q] Recently played\n"
        << "[C] Create playlist\n"
        << "[T] Add this song to a playlist\n"
        << "[M] My playlists\n"
        << "[X] Exit\n"
        << "-----------------------\n";

    if (player.lastError() != "")
        std::cout << "Problem: " << player.lastError() << "\n";

    if (message != "")
        std::cout << message << "\n";

    std::cout << ">";
}


// Shows the favorite songs (names only).
void ConsoleUI::showFavorites()
{
    system("cls");

    std::cout << "\n========== FAVORITES ==========\n\n";

    if (player.favoriteCount() == 0) {
        std::cout << "No favorite songs.\n\nPress any key to return...";
        _getch();
        return;
    }

    std::string query = askSearch();

    system("cls");
    std::cout << "\n========== FAVORITES ==========\n";

    if (query != "")
        std::cout << "Search: " << query << "\n";

    std::cout << "\n";

    int shown = 0;

    for (int n = 1; ; n++) {
        int row = player.findFavorite(query, n);

        if (row == 0)
            break;

        std::cout << "[" << n << "] " << describe(player.favoriteSongNumber(row)) << "\n";
        shown++;
    }

    if (shown == 0)
        std::cout << "No songs match.\n";

    std::cout << "\nPress any key to return...";
    _getch();
}


// Shows the recently played songs, newest first (names only).
void ConsoleUI::showRecent()
{
    system("cls");

    std::cout << "\n========== RECENTLY PLAYED ==========\n\n";

    if (player.recentCount() == 0) {
        std::cout << "No recently played songs.\n\nPress any key to return...";
        _getch();
        return;
    }

    std::string query = askSearch();

    system("cls");
    std::cout << "\n========== RECENTLY PLAYED ==========\n";

    if (query != "")
        std::cout << "Search: " << query << "\n";

    std::cout << "\n";

    int shown = 0;

    for (int n = 1; ; n++) {
        int row = player.findRecent(query, n);

        if (row == 0)
            break;

        std::cout << "[" << n << "] " << describe(player.recentSongNumber(row)) << "\n";
        shown++;
    }

    if (shown == 0)
        std::cout << "No songs match.\n";

    std::cout << "\nPress any key to return...";
    _getch();
}

// Reads one line and turns it into a number. Gives 0 if it is not a number.
static int readNumber()
{
    std::string line;
    std::getline(std::cin, line);
    return std::atoi(line.c_str());
}


// Asks for a name and makes a new playlist. Gives back a message for the screen.
std::string ConsoleUI::createPlaylistScreen()
{
    system("cls");

    std::cout << "\n========== NEW PLAYLIST ==========\n\n"
        << "Playlist name (empty = cancel): ";

    std::string name;
    std::getline(std::cin, name);

    if (name == "")
        return "Cancelled";

    if (player.createPlaylist(name))
        return "Playlist created: " + name;

    return "Could not create it (name already used, or it has a bad letter)";
}


// Shows the playlists, asks which one, and adds the song playing now.
std::string ConsoleUI::addToPlaylistScreen()
{
    system("cls");

    std::cout << "\n========== ADD TO PLAYLIST ==========\n\n";

    int count = player.playlistCount();

    if (count == 0)
        return "No playlists yet. Press C to create one";

    if (player.currentIndex() == 0)
        return "No song is playing";

    std::cout << "Song: " << player.songName() << "\n\n";

    for (int i = 1; i <= count; i++) {
        std::cout << "[" << i << "] " << player.playlistName(i) << "\n";
    }

    std::cout << "\nPick a number (0 = cancel): ";
    int pick = readNumber();

    if (pick < 1 || pick > count)
        return "Cancelled";

    std::string name = player.playlistName(pick);

    if (player.addCurrentToPlaylist(name))
        return "Added to " + name;

    return "Already in " + name;
}


// Searches every song. Pick a number from the results to play it.
std::string ConsoleUI::searchSongsScreen()
{
    system("cls");

    std::cout << "\n========== SEARCH SONGS ==========\n\n";

    std::string query = askSearch();

    if (query == "")
        return "Cancelled";

    std::cout << "\n";

    int shown = 0;

    for (int n = 1; ; n++) {
        int song = player.findSong(query, n);

        if (song == 0)
            break;

        std::cout << "[" << n << "] " << describe(song) << "\n";
        shown++;
    }

    if (shown == 0)
        return "No songs match: " + query;

    std::cout << "\nPlay a number (0 = cancel): ";
    int pick = readNumber();

    if (pick < 1 || pick > shown)
        return "Cancelled";

    player.playSong(player.findSong(query, pick));
    return "Playing";
}


// Shows all playlists. Pick one to see its songs.
void ConsoleUI::showPlaylists()
{
    while (true) {
        system("cls");

        std::cout << "\n========== MY PLAYLISTS ==========\n\n";

        int count = player.playlistCount();

        if (count == 0) {
            std::cout << "No playlists yet.\n\nPress any key to return...";
            _getch();
            return;
        }

        for (int i = 1; i <= count; i++) {
            std::string name = player.playlistName(i);
            std::cout << "[" << i << "] " << name
                << "  (" << player.playlistSongCount(name) << " songs)\n";
        }

        std::cout << "\nOpen a number (0 = back): ";
        int pick = readNumber();

        if (pick < 1 || pick > count)
            return;

        std::string name = player.playlistName(pick);

        system("cls");
        std::cout << "\n========== " << name << " ==========\n\n";

        if (player.playlistSongCount(name) == 0) {
            std::cout << "No songs in this playlist.\n\nPress any key to go back...";
            _getch();
            continue;
        }

        std::string query = askSearch();

        system("cls");
        std::cout << "\n========== " << name << " ==========\n";

        if (query != "")
            std::cout << "Search: " << query << "\n";

        std::cout << "\n";

        int shown = 0;

        for (int n = 1; ; n++) {
            int row = player.findInPlaylist(name, query, n);

            if (row == 0)
                break;

            std::cout << "[" << n << "] "
                << describe(player.playlistSongNumber(name, row)) << "\n";
            shown++;
        }

        if (shown == 0)
            std::cout << "No songs match.\n";

        std::cout << "\nPress any key to go back...";
        _getch();
    }
}

void ConsoleUI::run(const std::string& folder)
{
    if (!player.openFolder(folder)) {
        std::cout << "No songs found in: " << folder << "\n"
            << "Check MUSIC_FOLDER in main.cpp.\n"
            << "Press any key to close...";
        _getch();
        return;
    }

    player.next();

    std::string message = "";

    while (true) {
        player.update();
        draw(message);

        if (_kbhit()) {
            char key = (char)std::toupper((unsigned char)_getch());

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

            case ' ':
                player.togglePlay();
                message = player.isPaused() ? "Paused" : "Playing";
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

            case 'G':
                message = searchSongsScreen();
                break;

            case 'A':
                showFavorites();
                message = "";
                break;

            case 'Q':
                showRecent();
                message = "";
                break;

            case 'C':
                message = createPlaylistScreen();
                break;

            case 'T':
                message = addToPlaylistScreen();
                break;

            case 'M':
                showPlaylists();
                message = "";
                break;

            case 'X':
                system("cls");
                std::cout << "Bye!" << std::endl;
                return;

            default:
                message = "Wrong key, try again";
            }
        }

        Sleep(100);
    }
}