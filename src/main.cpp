#include <string>
#include "audio/miniaudioBackend.h"
#include "audio/Recorder.h"
#include "core/appPaths.h"
#include "core/library.h"
#include "core/playerController.h"
#include "core/recordings.h"
#include "core/settings.h"
#include "core/trackList.h"
#include "core/userSession.h"
#include "app/UIbridge.h"
#include "app.h"

// The program is put together here and nowhere else:
//   audio/  plays and records sound          core/  the rules: library, queue, playlists, profiles, settings
//   app/    connects the screen (ui/) to core and audio
int main()
{
    MiniaudioBackend audio;
    Recorder recorder;
    TrackList tracks;
    UserSession session;                                // before anyone logs in, everything goes into the guest folder
    PlayerController player(audio, tracks);
    Library library(player, tracks);
    Settings settings;
    Recordings recordings;

    // Songs found earlier are loaded from the index even if this folder does not exist.
    // "Add folder" on the All Songs page adds more.
    player.openFolder(AppPaths::defaultMusicFolder());

    auto ui = AppWindow::create();
    UiBridge bridge(*ui, player, tracks, library, session, settings, recordings, recorder);
    bridge.start();

    ui->run();
    return 0;
}
