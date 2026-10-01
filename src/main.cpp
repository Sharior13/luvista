#include <iostream>
#include <cstdlib>
#include <cctype>
#include <string>
#include "audio/audioBackend.h"

using namespace std;

int main()
{
    AudioBackend obj;
    float volume = 0.5f;
    string message = "";   // result of the last key

    if (!obj.load("C:/Users/LENOVO/Documents/CODEs/Music Player/luvista/src/audio/Sapphire.mp3")) {
        cout << "Failed to load song" << endl;
        return 1;
    }

    obj.play();   // start the song

    char key = ' ';

    while (key != 'X') {
        system("cls");

        cout << "\nPlaying Now: " << obj.SongName() << endl;
        cout << "Time: " << (int)obj.position() << " / " << (int)obj.duration() << " s"
            << "   Volume: " << (int)(volume * 100) << "%" << endl;
        cout << "-----------------------\n"
            << "[P] Pause\n"
            << "[R] Resume / Play\n"
            << "[S] ReStart\n"
            << "[F] Forward 10s\n"
            << "[B] Back 10s\n"
            << "[+] Volume Up\n"
            << "[-] Volume Down\n"
            << "[X] Exit\n"
            << "-----------------------\n";

        if (message != "")
            cout << message << endl;

        cout << "> ";
        cin >> key;
        key = toupper(key);   // so 'p' and 'P' both work

        if (key == 'P') {
            obj.pause();
            message = "Paused";
        }
            
        else if (key == 'R') {
            obj.play();
            message = "Playing";
        }

        else if (key == 'S') {
            obj.restart();
            message = "Restarted";
        }

        else if (key == 'F') {
            obj.seek(obj.position() + 10);
            message = "Forward 10s";
        }

        else if (key == 'B') {
            obj.seek(obj.position() - 10);
            message = "Back 10s";
        }

        else if (key == '+') {
            volume = volume + 0.1f;
            if (volume > 1.0f) volume = 1.0f;
            obj.set_volume(volume);
            message = "Volume up";
        }

        else if (key == '-') {
            volume = volume - 0.1f;
            if (volume < 0.0f) volume = 0.0f;
            obj.set_volume(volume);
            message = "Volume down";
        }

        else if (key == 'X') {
            message = "Bye!";
        }
        else {
            message = "Wrong key, try again";
        }
    }

    system("cls");
    cout << "Bye!" << endl;
    return 0;
}