#include <string>
#include "audio/miniaudioBackend.h"
#include "core/trackList.h"
#include "core/playerController.h"
#include "app/consoleUI.h"

// THE MANAGER
//
// main does no real work. It only creates the helpers and connects them:
//
//   screen (ConsoleUI)  ->  DJ (PlayerController)  ->  speaker (MiniaudioBackend)
//                               |
//                               +-> librarian (TrackList)

// Change this if you move the project or the music
const std::string MUSIC_FOLDER = "C:/Users/LENOVO/Documents/CODEs/Music Player/luvista/src/audio";

int main()
{
    MiniaudioBackend audio;                  // the speaker
    TrackList tracks;                        // the librarian
    PlayerController player(audio, tracks);  // the DJ gets the speaker and librarian
    ConsoleUI ui(player);                    // the waiter gets the DJ

    ui.run(MUSIC_FOLDER);
    return 0;
}
