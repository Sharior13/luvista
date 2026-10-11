#include <string>
#include "audio/miniaudioBackend.h"
#include "core/trackList.h"
#include "core/playerController.h"
#include "app/uiBridge.h"
#include "app.h"

// Folder scanned at startup. "add a new folder" on the All Songs page adds more.
const std::string MUSIC_FOLDER = "C:/Users/lenovo/Music";

int main()
{
    MiniaudioBackend audio;
    TrackList tracks;
    PlayerController player(audio, tracks);
    player.openFolder(MUSIC_FOLDER);        // songs found earlier are loaded from the index even if this fails

    auto ui = AppWindow::create();
    UiBridge bridge(*ui, player, tracks);
    bridge.start();

    ui->run();
    return 0;
}
