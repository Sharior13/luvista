#include "UIbridge.h"
#include "uiConvert.h"
#include "../audio/CoverArt.h"
#include "../core/appPaths.h"
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


// a scan that is still running is told to stop, so closing the program does not wait for it
UiBridge::~UiBridge()
{
    stopping = true;

    if (worker.joinable())
        worker.join();
}


Page UiBridge::pageOf(PageId id)
{
    // the first PageIds are the Library's pages, in the same order
    return id < RECORDINGS ? (Page)(int)id : Page::AllSongs;
}


std::string UiBridge::listNameOf(PageId id)
{
    switch (id) {
    case FAVORITES: return "favorites";
    case PLAYLIST: return "playlist";
    case ARTIST: return "artist";
    case ALBUM: return "album";
    case RECENT: return "recent";
    case MOST_PLAYED: return "most-played";
    case RECENTLY_ADDED: return "recently-added";
    case RECORDINGS: return "recordings";
    default: return "songs";
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
    if (n == "album") return ALBUM;
    if (n == "recent") return RECENT;
    if (n == "most-played") return MOST_PLAYED;
    if (n == "recently-added") return RECENTLY_ADDED;
    if (n == "recordings") return RECORDINGS;
    return -1;
}


// Fills the screen, hooks up the buttons and starts the clock.
// Only what is on screen first (the home page, the player bar) is made here. The song lists are made when their page
// opens, their rows when they scroll into view, and the music folder is looked through after the window is up.
void UiBridge::start()
{
    std::srand((unsigned)std::time(nullptr));

    state().set_volume(player.volume() / 100.0f);

    CoverArt::purgeNumbered();                  // pictures kept under a song number once showed up on the wrong song
    library.reload();
    albumsTotal = library.albumCount();
    artistsTotal = library.artistCount();
    recordings.refresh();
    refreshCards();
    refreshPlaylists();
    refreshAll();
    syncPlayerBar(true);
    showProfile();

    connectPlayerBar();
    connectSongPages();
    connectSongMenus();
    connectSelection();
    connectPlaylists();
    connectRecorder();
    connectSession();
    connectAccount();
    connectSettings();

    state().set_open_last_profile(launch.openLast());
    refreshRemoved();
    openLastProfile();

    // four times a second: move the seek bar, go to the next song when one ends, move the recorder's meter
    timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(250), [this] {
        player.update();
        syncPlayerBar(false);
        tickRecorder();
        });

    // songs found earlier are already loaded from the index. New files in the music folder are looked for once
    // the first picture of the window is up, on another thread.
    slint::Timer::single_shot(std::chrono::milliseconds(300), [this] {
        for (int row = 0; row < folders.count(); row++)
            scanFolder(folders.path(row));              // the default folder and the ones the user added, one after the other
        });
}


int UiBridge::currentPage() const
{
    if (state().get_selected_playlist() >= 0)
        return PLAYLIST;

    switch (state().get_section()) {
    case NavSection::Songs: return ALL_SONGS;
    case NavSection::Favorites: return FAVORITES;
    case NavSection::Recordings: return RECORDINGS;
    case NavSection::Artist: return ARTIST;
    case NavSection::Album: return ALBUM;
    case NavSection::Recent: return RECENT;
    case NavSection::MostPlayed: return MOST_PLAYED;
    case NavSection::RecentlyAdded: return RECENTLY_ADDED;
    default: return -1;
    }
}


void UiBridge::refreshCounts()
{
    state().set_songs_count(library.size());
    state().set_favorites_count(player.favoriteCount());
    state().set_recordings_count(recordings.count());
    state().set_albums_count(albumsTotal);
    state().set_artists_count(artistsTotal);
}


void UiBridge::refreshAll()
{
    refreshCounts();

    int page = currentPage();

    if (page >= 0)
        refreshPage((PageId)page);

    if (state().get_section() == NavSection::Albums)
        refreshAlbums();

    if (state().get_section() == NavSection::Artists)
        refreshArtists();

    if (state().get_section() == NavSection::Profile) {
        refreshTopArtists();
        refreshTopSongs();
    }

    refreshHome();
    refreshStorage();
}


// songs came or went (a folder was added, a song removed): every number may have moved
void UiBridge::reloadLibrary()
{
    library.reload();
    albumsTotal = library.albumCount();
    artistsTotal = library.artistCount();
    refreshRemoved();
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
