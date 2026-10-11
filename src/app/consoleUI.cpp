#include "consoleUI.h"
#include "../audio/miniaudioBackend.h"
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <conio.h>
#include <windows.h>

namespace fs = std::filesystem;


// How the design writes dates: "yesterday", "2 weeks", "4 weeks".
// added = seconds since 1970 (0 = unknown).
static std::string agoLabel(long long added)
{
    if (added <= 0)
        return "unknown";

    long long now = (long long)std::time(nullptr);
    long long secs = now - added;

    if (secs < 0)
        secs = 0;

    if (secs < 60)       return "just now";
    if (secs < 3600)     return std::to_string(secs / 60) + " min";
    if (secs < 86400)    return std::to_string(secs / 3600) + " hours";
    if (secs < 172800)   return "yesterday";
    if (secs < 604800)   return std::to_string(secs / 86400) + " days";
    if (secs < 2592000)  return std::to_string(secs / 604800) + " weeks";
    if (secs < 31536000) return std::to_string(secs / 2592000) + " months";
    return std::to_string(secs / 31536000) + " years";
}


// 0 = OFF, 1 = ALL, 2 = ONE (the repeat modes)
static std::string repeatLabel(int mode)
{
    if (mode == 1) return "ALL";
    if (mode == 2) return "ONE";
    return "OFF";
}


// the screens get the player to talk to
ConsoleUI::ConsoleUI(PlayerController& p) : player(p)
{
}

// search words (empty = show all)
std::string ConsoleUI::askSearch()
{
    std::cout << "Search (empty = show all): ";

    std::string query;
    std::getline(std::cin, query);
    return query;
}


// "Title  |  Artist  |  Album  |  yesterday" for one row (empty if no such song)
std::string ConsoleUI::describe(int songNumber)
{
    if (songNumber <= 0)
        return "";

    return player.songTitle(songNumber) + "  |  " +
        player.songArtist(songNumber) + "  |  " +
        player.songAlbum(songNumber) + "  |  " +
        agoLabel(player.songAdded(songNumber));
}


// clear the screen and redraw everything
void ConsoleUI::draw(const std::string& message)
{
    system("cls");

    std::cout << "Playing Now: " << player.songName();

    if (player.currentIndex() != 0)
        std::cout << " - " << player.songArtist(player.currentIndex());

    if (player.isFavorite())
        std::cout << " [Favorite]";

    if (recorder.isRecording())
        std::cout << " [RECORDING]";

    if (player.currentIndex() != 0)
        std::cout << "  (" << player.currentIndex()
            << "/" << player.total() << ")\n";
    else if (player.songName() != "")
        std::cout << "  (recording)\n";        // playing a file that is not in the library
    else
        std::cout << "  (nothing yet)\n";

    std::cout << "Time: " << (int)player.position()
        << " / " << (int)player.duration() << " s"
        << "   Volume: " << player.volume() << "%\n";

    std::cout << "Shuffle: " << (player.isShuffleOn() ? "ON" : "OFF")
        << "   Repeat: " << repeatLabel(player.repeat())
        << "   In queue: " << player.queueCount() << "\n";

    if (player.currentIndex() != 0) {
        std::string coming = player.nextSongTitle();

        if (coming != "")
            std::cout << "Next: " << coming << "\n";
    }

    std::cout << "-----------------------\n"
        << "[N] Next song\n"
        << "[V] Previous song\n"
        << "[E] Add this song to the queue\n"
        << "[U] Show the queue\n"
        << "[H] Shuffle on / off\n"
        << "[Y] Repeat off / all / one\n"
        << "[P] Pause\n"
        << "[R] Resume / Play\n"
        << "[Space] Pause / Play (one button)\n"
        << "[S] ReStart\n"
        << "[F] Forward 10s\n"
        << "[B] Back 10s\n"
        << "[J] Jump to a second\n"
        << "[+] Volume Up\n"
        << "[-] Volume Down\n"
        << "[L] Favorite on / off\n"
        << "[G] Search all songs\n"
        << "[A] Show Favorites\n"
        << "[Q] Recently played\n"
        << "[I] My recordings\n"
        << "[C] Create playlist\n"
        << "[T] Add this song to a playlist\n"
        << "[D] Delete playlist / remove song\n"
        << "[M] My playlists\n"
        << "[O] Record on / off (microphone)\n"
        << "[X] Exit\n"
        << "-----------------------\n";

    if (player.lastError() != "")
        std::cout << "Problem: " << player.lastError() << "\n";

    if (message != "")
        std::cout << message << "\n";

    std::cout << ">";
}


// favorites screen
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


// recently played screen
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

// one line -> a number (0 if it is not a number)
static int readNumber()
{
    std::string line;
    std::getline(std::cin, line);
    return std::atoi(line.c_str());
}


// ask a name, create the playlist
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


// pick a playlist, add the song playing now
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


// Delete menu: 1 = whole playlist, 2 = this song out of a playlist,
// 3 = any song out of a playlist, 4 = clear all favorites, 5 = forget a song.
std::string ConsoleUI::deleteScreen()
{
    system("cls");

    std::cout << "\n========== DELETE ==========\n\n"
        << "[1] Delete a whole playlist\n"
        << "[2] Remove this song from a playlist\n"
        << "[3] Remove any song from a playlist\n"
        << "[4] Clear ALL favorites\n"
        << "[5] Remove a song from the library\n\n"
        << "Pick 1-5 (0 = cancel): ";

    int choice = readNumber();

    if (choice < 1 || choice > 5)
        return "Cancelled";

    // option 4 needs no playlist
    if (choice == 4) {
        if (player.favoriteCount() == 0)
            return "Favorites are already empty";

        player.clearFavorites();
        return "All favorites cleared";
    }

    int count = player.playlistCount();

    if (count == 0)
        return "No playlists yet. Press C to create one";

    system("cls");
    std::cout << "\n========== DELETE ==========\n\n";

    for (int i = 1; i <= count; i++) {
        std::cout << "[" << i << "] " << player.playlistName(i) << "\n";
    }

    std::cout << "\nPick a number (0 = cancel): ";
    int pick = readNumber();

    if (pick < 1 || pick > count)
        return "Cancelled";

    std::string name = player.playlistName(pick);

    if (choice == 1) {
        if (player.deletePlaylist(name))
            return "Deleted playlist: " + name;

        return "Could not delete " + name;
    }

    if (choice == 3) {
        if (player.playlistSongCount(name) == 0)
            return name + " has no songs";

        // show the songs so a number can be picked
        system("cls");
        std::cout << "\n========== " << name << " ==========\n\n";

        for (int n = 1; n <= player.playlistSongCount(name); n++) {
            std::cout << "[" << n << "] "
                << describe(player.playlistSongNumber(name, n)) << "\n";
        }

        std::cout << "\nRemove which song (0 = cancel): ";
        int row = readNumber();

        if (row < 1 || row > player.playlistSongCount(name))
            return "Cancelled";

        if (player.removeFromPlaylistRow(name, row))
            return "Removed from " + name;

        return "Could not remove it";
    }

    // 5: forget one song from the whole library
    if (choice == 5) {
        if (player.total() == 0)
            return "The library is empty";

        system("cls");
        std::cout << "\n========== REMOVE FROM LIBRARY ==========\n\n"
            << "(the music file stays on disk)\n\n";

        for (int n = 1; n <= player.total(); n++) {
            std::cout << "[" << n << "] " << describe(n) << "\n";
        }

        std::cout << "\nRemove which song (0 = cancel): ";
        int song = readNumber();

        if (song < 1 || song > player.total())
            return "Cancelled";

        std::string title = player.songTitle(song);

        if (player.removeSongFromLibrary(song))
            return "Removed from library: " + title;

        return "Could not remove " + title;
    }

    // choice 2
    if (player.currentIndex() == 0)
        return "No song is playing";

    if (player.removeCurrentFromPlaylist(name))
        return "Removed " + player.songName() + " from " + name;

    return player.songName() + " was not in " + name;
}


// search all songs, pick one to play
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


// list playlists, open one to see its songs
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

// the queue screen: the songs waiting to play next
std::string ConsoleUI::showQueue()
{
    int count = player.queueCount();

    if (count == 0)
        return "The queue is empty";

    system("cls");
    std::cout << "\n========== QUEUE ==========\n\n";

    for (int n = 1; n <= count; n++) {
        int song = player.queueSongNumber(n);

        if (song == 0)
            std::cout << "[" << n << "] (gone from the library)\n";
        else
            std::cout << "[" << n << "] " << describe(song) << "\n";
    }

    std::cout << "\n[C] Clear the queue\nAny other key = back\n";

    int key = _getch();

    if (key == 'c' || key == 'C') {
        player.clearQueue();
        return "Queue cleared";
    }

    return "";
}


// last time a file was written, seconds since 1970 (0 = Windows will not say)
static long long writtenAt(const fs::path& p)
{
    std::error_code problem;
    auto when = fs::last_write_time(p, problem);

    if (problem)
        return 0;

    // file clock -> wall clock: both clocks read "now" and we borrow the difference
    auto offset = when - fs::file_time_type::clock::now();
    auto sameMoment = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        std::chrono::system_clock::now() + offset);

    return (long long)std::chrono::system_clock::to_time_t(sameMoment);
}


// 95 seconds -> "1:35"
static std::string clockTime(double seconds)
{
    int s = (int)(seconds + 0.5);
    std::string out = std::to_string(s / 60) + ":";

    if (s % 60 < 10)
        out += "0";

    out += std::to_string(s % 60);
    return out;
}


// the Recordings page: every rec_*.mp3 in the Windows recordings folder.
// These files are NOT in the library - picking one plays it directly.
std::string ConsoleUI::showRecordings()
{
    const int maxRows = 500;

    int numbers[maxRows];
    std::string paths[maxRows];
    int count = 0;

    std::error_code problem;
    fs::directory_iterator it(Recorder::folder(), problem), end;

    for (; !problem && it != end && count < maxRows; it.increment(problem)) {
        std::error_code nope;

        if (!it->is_regular_file(nope))
            continue;

        std::string name = it->path().filename().string();

        if (name.size() < 9 || name.compare(0, 4, "rec_") != 0 ||
            name.compare(name.size() - 4, 4, ".mp3") != 0)
            continue;                                  // not one of our recordings

        int n = std::atoi(name.c_str() + 4);

        if (n <= 0)
            continue;

        numbers[count] = n;
        paths[count] = it->path().string();
        count++;
    }

    // sort by the number in the name: rec_1, rec_2, rec_3, ... (selection sort)
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            if (numbers[j] < numbers[i]) {
                int tmpNumber = numbers[i];
                numbers[i] = numbers[j];
                numbers[j] = tmpNumber;

                std::string tmpPath = paths[i];
                paths[i] = paths[j];
                paths[j] = tmpPath;
            }
        }
    }

    if (count == 0)
        return "No recordings yet. Press O to record one";

    system("cls");
    std::cout << "\n========== RECORDINGS ==========\n\n";

    if (recorder.isRecording())
        std::cout << "[RECORDING] press O on the main screen to stop\n\n";

    std::cout << count << " recordings in\n" << Recorder::folder() << "\n\n";

    // one engine just for measuring lengths - it never touches the song playing now
    MiniaudioBackend probe;

    for (int i = 0; i < count; i++) {
        std::cout << "[" << (i + 1) << "] Recording " << numbers[i]
            << "   " << agoLabel(writtenAt(paths[i])) << "   ";

        if (probe.load(paths[i]))
            std::cout << clockTime(probe.duration());
        else
            std::cout << "?";

        std::cout << "\n";
    }

    std::cout << "\nPlay a number (0 = back): ";
    int pick = readNumber();

    if (pick < 1 || pick > count)
        return "";

    std::string title = "Recording " + std::to_string(numbers[pick - 1]);

    if (player.playFilePath(paths[pick - 1], title))
        return "Playing " + title;

    return "Could not open it: " + player.lastError();
}


// record on / off: saves rec_1.mp3, rec_2.mp3, ... in the Sound Recordings folder
std::string ConsoleUI::toggleRecording()
{
    if (recorder.isRecording()) {
        std::string saved = recorder.stop();

        if (saved == "")
            return "Could not save the recording: " + recorder.lastError();

        return "Saved: " + saved;
    }

    if (recorder.start(Recorder::nextRecordingPath()))
        return "Recording... press O again to stop";

    return "Could not start recording: " + recorder.lastError();
}


// main loop: scan the folder, then run until X is pressed
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

            case 'E':
                if (player.currentIndex() == 0)
                    message = "No song is playing";
                else if (player.addToQueue(player.currentIndex()))
                    message = "Added to the queue";
                else
                    message = "Could not add to the queue";
                break;

            case 'U':
                message = showQueue();
                break;

            case 'H':
                player.toggleShuffle();
                message = player.isShuffleOn() ? "Shuffle ON" : "Shuffle OFF";
                break;

            case 'Y':
                player.toggleRepeat();
                message = "Repeat: " + repeatLabel(player.repeat());
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

            case 'J': {
                std::cout << "\nJump to which second (0 = cancel): ";
                int sec = readNumber();

                if (sec > 0) {
                    player.seekTo(sec);
                    message = "Jumped to second " + std::to_string(sec);
                }
                else
                    message = "Cancelled";
                break;
            }

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

            case 'I':
                message = showRecordings();
                break;

            case 'C':
                message = createPlaylistScreen();
                break;

            case 'T':
                message = addToPlaylistScreen();
                break;

            case 'D':
                message = deleteScreen();
                break;

            case 'M':
                showPlaylists();
                message = "";
                break;

            case 'O':
                message = toggleRecording();
                break;

            case 'X':
                recorder.stop();   // never leave a recording file open
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