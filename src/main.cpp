#include <string>
#include "audio/miniaudioBackend.h"
#include "core/trackList.h"
#include "core/playerController.h"
#include "app/consoleUI.h"

const std::string MUSIC_FOLDER = "C:/Users/LENOVO/Documents/CODEs/Music Player/luvista/src/audio";

int main()
{
    // static = one copy that lives OUTSIDE the stack. The song array alone is
    // about 1.4 MB, while a normal stack only has 1 MB - so it must not go there.
    static MiniaudioBackend audio;
    static TrackList tracks;
    static PlayerController player(audio, tracks);

    ConsoleUI ui(player);
    ui.run(MUSIC_FOLDER);
     //auto ui = AppWindow::create();
    // ui->run();
    return 0;
}
