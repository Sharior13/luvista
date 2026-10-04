#include <string>
#include "app.h"
#include "audio/miniaudioBackend.h"
#include "core/trackList.h"
#include "core/playerController.h"
#include "app/consoleUI.h"

// main only connects the pieces. It does no real work.
//
//   ConsoleUI  ->  PlayerController  ->  AudioBackend (MiniaudioBackend)
//   (screen)       (rules)               (speaker)

// Change this if you move the project or the music
//const std::string MUSIC_FOLDER = "C:/Users/LENOVO/Documents/CODEs/Music Player/luvista/src/audio";

int main()
{
    //MiniaudioBackend audio;
    //TrackList tracks;
    //PlayerController player(audio, tracks);
    //ConsoleUI ui(player);

     auto ui = AppWindow::create();
     ui->run();

    //ui.run(MUSIC_FOLDER);
    return 0;
}