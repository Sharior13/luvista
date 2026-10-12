#include "UIbridge.h"
#include "uiConvert.h"
#include <chrono>
#include <cstdlib>
#include <ctime>

using UiConvert::toSlint;
using UiConvert::toStd;

UiBridge::UiBridge(AppWindow& window, PlayerController& p, TrackList& t, Library& l,
                   UserSession& s, Settings& st, Recordings& r, Recorder& rec, MusicFolders& f)
    : ui(window), player(p), tracks(t), library(l), session(s), settings(st), recordings(r), recorder(rec), folders(f),
      images(t, p), popup(p), pickWindow(p)
{
}


Page UiBridge::pageOf(PageId id)
{
    switch (id) {
    case FAVORITES: return Page::Favorites;
    case PLAYLIST: return Page::Playlist;
    case ARTIST: return Page::Artist;
    default: return Page::AllSongs;
    }
}


// which song page a slint list name means
int UiBridge::pageIdOf(const slint::SharedString& listName)
{
    std::string n = toStd(listName);

    if (n == "songs") return ALL_SONGS;
    if (n == "favorites") return FAVORITES;
    if (n == "playlist") return PLAYLIST;
    if (n == "artist") return ARTIST;
    if (n == "recordings") return RECORDINGS;
    return -1;
}


// Fills the screen, hooks up the buttons and starts the clock.
void UiBridge::start()
{
    std::srand((unsigned)std::time(nullptr));

    state().set_volume(player.volume() / 100.0f);

    library.reload();
    recordings.refresh();
    refreshCards();
    refreshPlaylists();
    state().set_songs_count(library.size());
    refreshAll();
    syncPlayerBar(true);
    showProfile();

    connectPlayerBar();
    connectSongPages();
    connectSongMenus();
    connectPlaylists();
    connectRecorder();
    connectSession();
    connectAccount();
    connectSettings();

    // four times a second: move the seek bar, go to the next song when one ends, move the recorder's meter
    timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(250), [this] {
        player.update();
        syncPlayerBar(false);
        tickRecorder();
        });
}


void UiBridge::refreshAll()
{
    for (int page = 0; page < RECORDINGS; page++)
        refreshPage((PageId)page);

    refreshRecordings();
    refreshHome();
    refreshTopArtists();
    refreshStorage();
}


// songs came or went (a folder was added, a song removed): every number may have moved
void UiBridge::reloadLibrary()
{
    library.reload();
    state().set_songs_count(library.size());
    refreshAll();
    refreshPlaylists();
    syncPlayerBar(true);
}


SongInfo UiBridge::infoOf(int number, const std::vector<char>& favorites)
{
    SongInfo info;

    if (!library.valid(number))
        return info;

    const SongMeta& s = library.meta(number);

    info.title = toSlint(s.title);
    info.artist = toSlint(s.artist);
    info.album = toSlint(s.album);
    info.date_added = toSlint(UiConvert::ago(s.added));
    info.length = toSlint(UiConvert::lengthText(s.seconds));
    info.art = images.cover(number);
    info.favorite = number < (int)favorites.size() && favorites[number] != 0;
    info.queued = player.isQueued(number);
    return info;
}


SongInfo UiBridge::infoOf(int number)
{
    return infoOf(number, library.favoriteFlags());
}


TrackInfo UiBridge::cardOf(int number)
{
    TrackInfo card;

    if (!library.valid(number))
        return card;

    card.title = toSlint(library.meta(number).title);
    card.subtitle = toSlint(library.meta(number).artist);
    card.art = images.cover(number);
    return card;
}
