#ifdef _WIN32
#define NOMINMAX                          //keeps windows.h from breaking std::min and std::max
#endif
#include "uiBridge.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include "miniaudio.h"                    //only the declarations. The code lives in miniaudioBackend.cpp

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

namespace fs = std::filesystem;

const int ALL_SONGS = 0;
const int FAVORITES = 1;
const int PLAYLIST = 2;
const int HOME_ROWS = 12;                 //how many songs "Recently Added" shows on the home page


//small helpers

//"Hello" -> "hello"
static std::string lower(std::string s)
{
    for (int i = 0; i < (int)s.size(); i++)
        s[i] = (char)std::tolower((unsigned char)s[i]);

    return s;
}

//a slint string as a normal string.
static std::string toStd(const slint::SharedString& s)
{
    return std::string(std::string_view(s));
}

//which song page a slint list name means
static int pageNumber(const slint::SharedString& name)
{
    std::string n = toStd(name);

    if (n == "songs") return ALL_SONGS;
    if (n == "favorites") return FAVORITES;
    if (n == "playlist") return PLAYLIST;
    return -1;
}

//3:22
static std::string lengthText(double seconds)
{
    int total = (int)(seconds + 0.5);
    char text[16];

    std::snprintf(text, sizeof text, "%d:%02d", total / 60, total % 60);
    return text;
}

//"1 week", "2 weeks"
static std::string countOf(long long n, const std::string& word)
{
    if (n == 1)
        return "1 " + word;

    return std::to_string(n) + " " + word + "s";
}

//how long ago a song was added: "today", "yesterday", "3 days", "2 weeks"
static std::string ago(long long added)
{
    long long days = ((long long)std::time(nullptr) - added) / 86400;

    if (added <= 0 || days <= 0) return "today";
    if (days == 1) return "yesterday";
    if (days < 7) return countOf(days, "day");
    if (days < 30) return countOf(days / 7, "week");
    if (days < 365) return countOf(days / 30, "month");
    return countOf(days / 365, "year");
}

//how long a song file is, in seconds. 0 if the file cannot be read
static double fileSeconds(const std::string& path)
{
    ma_decoder decoder;

    if (ma_decoder_init_file(path.c_str(), nullptr, &decoder) != MA_SUCCESS)
        return 0.0;

    ma_uint64 frames = 0;
    double seconds = 0.0;

    if (ma_decoder_get_length_in_pcm_frames(&decoder, &frames) == MA_SUCCESS && decoder.outputSampleRate > 0)
        seconds = (double)frames / (double)decoder.outputSampleRate;

    ma_decoder_uninit(&decoder);
    return seconds;
}

//opens the windows "pick a folder" window, gives back "" if the user cancels
static std::string chooseFolder()
{
#ifdef _WIN32
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);    //fine if it was already done

    BROWSEINFOW info = {};
    info.lpszTitle = L"Choose a folder with music";
    info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    PIDLIST_ABSOLUTE chosen = SHBrowseForFolderW(&info);
    if (chosen == nullptr)
        return "";

    wchar_t path[MAX_PATH] = {};
    bool ok = SHGetPathFromIDListW(chosen, path) != FALSE;
    CoTaskMemFree(chosen);

    if (!ok)
        return "";

    return fs::path(path).string();
#else
    return "";
#endif
}


//setup

//the bridge gets the screen, the player and the librarian. every page starts on "recently added"
UiBridge::UiBridge(AppWindow& window, PlayerController& p, TrackList& t)
    : ui(window), player(p), tracks(t)
{
    for (int i = 0; i < 3; i++)
        pages[i].sort = 0;

    openPlaylist = -1;
    lastCurrent = -1;
}


//fills the screen, hooks up the buttons and starts the clock, call once before ui.run()
void UiBridge::start()
{
    std::srand((unsigned)std::time(nullptr));     //for the shuffle buttons

    state().set_volume(player.volume() / 100.0f); //the screen starts at 100%, the player at 50%

    loadLibrary();
    refreshPlaylists();
    refreshPage(ALL_SONGS);
    refreshPage(FAVORITES);
    refreshPage(PLAYLIST);
    syncPlayerBar(true);                          //also fills the home page, the queue and the heart

    connectPlayerBar();
    connectSongPages();
    connectPlaylists();
    connectOtherScreens();

    //four times a second, move the seek bar, and go to the next song when one ends
    timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(250), [this] {
        player.update();
        syncPlayerBar(false);
        });
}


//play, next, previous, seek, volume and the heart
void UiBridge::connectPlayerBar()
{
    state().on_toggle_play([this] { togglePlay(); });
    state().on_next([this] { player.next(); syncPlayerBar(false); });
    state().on_previous([this] { player.previous(); syncPlayerBar(false); });
    state().on_seek([this](float seconds) { player.seekTo(seconds); syncPlayerBar(false); });
    state().on_volume_changed([this](float v) { player.setVolume((int)(v * 100 + 0.5f)); });
    state().on_favorite_toggled([this](bool on) { setFavorite(on); });
}


//rows, play, shuffle, search, sort and "add a new folder" on all songs, favorites and playlists
void UiBridge::connectSongPages()
{
    state().on_song_clicked([this](slint::SharedString list, int row) {
        int page = pageNumber(list);

        if (page >= 0)
            playRow(page, row);
        });

    state().on_play_list([this](slint::SharedString list) {
        int page = pageNumber(list);

        if (page >= 0)
            playRow(page, 0);
        });

    //playerController has no shuffle mode yet, so this only starts on a random song
    state().on_shuffle_list([this](slint::SharedString list) {
        int page = pageNumber(list);

        if (page < 0 || pages[page].shown.empty())
            return;

        playRow(page, std::rand() % (int)pages[page].shown.size());
        });

    state().on_list_search_changed([this](slint::SharedString list, slint::SharedString text) {
        int page = pageNumber(list);

        if (page < 0)
            return;

        pages[page].search = lower(toStd(text));
        refreshPage(page);
        });

    state().on_list_sort_changed([this](slint::SharedString list, int index) {
        int page = pageNumber(list);

        if (page < 0)
            return;

        pages[page].sort = index;
        refreshPage(page);
        });

    state().on_add_folder([this] { addFolder(); });
}


//the playlists in the sidebar and the "+" popup in the player bar
void UiBridge::connectPlaylists()
{
    state().on_playlist_clicked([this](int i) {
        openPlaylist = i;
        resetPage(PLAYLIST);
        });

    //the screen has no name box yet, so new playlists are called "New playlist", "New playlist 2"
    state().on_add_playlist([this] {
        if (player.createPlaylist(newPlaylistName())) {
            refreshPlaylists();
            refreshChoices();
        }
        });

    state().on_add_current_track([this] { openPopup(); });
    state().on_playlist_toggled([this](int row) { toggleInPlaylist(row); });
    state().on_new_playlist_from_track([this] { newPlaylistFromTrack(); });

    state().on_playlist_picker_search([this](slint::SharedString text) {
        pickerSearch = lower(toStd(text));
        refreshChoices();
        });
}


//sidebar pages, the home cards, the queue panel, guest mode and logout
void UiBridge::connectOtherScreens()
{
    //pages are built again when opened, so their search box and sort start fresh
    state().on_section_changed([this](auto section) {
        int s = static_cast<int>(section);        //0 home, 1 songs, 2 favorites, 3 recordings

        if (s == 0) refreshHome();
        if (s == 1) resetPage(ALL_SONGS);
        if (s == 2) resetPage(FAVORITES);
        });

    state().on_track_clicked([this](TrackInfo card) {
        playCard(toStd(card.title), toStd(card.subtitle));
        });

    //only "Recently Added" has a page to go to: All Songs, newest first
    state().on_show_all([this](slint::SharedString which) {
        if (toStd(which) == "recently-added")
            openSongsPage();
        });

    //the player plays the library in order, so the queue is the library and row i is song i + 1
    state().on_queue_track_clicked([this](int row) {
        player.playSong(row + 1);
        syncPlayerBar(false);
        });

    state().on_recent_track_clicked([this](int row) {
        if (row >= 0 && row < (int)recentNumbers.size()) {
            player.playSong(recentNumbers[row]);
            syncPlayerBar(false);
        }
        });

    session().on_continue_as_guest([this] { session().set_logged_in(true); });

    session().on_logout([this] {
        player.pause();
        syncPlayerBar(false);
        session().set_logged_in(false);
        });
}


//library and song pages

//copies every song out of the librarian. this also reads each file's length, so it can take a moment
void UiBridge::loadLibrary()
{
    library.clear();
    library.push_back(Song());                    //slot 0, never used

    int songs = player.total();

    for (int n = 1; n <= songs; n++) {
        Song s;
        s.number = n;
        s.title = player.songTitle(n);
        s.artist = player.songArtist(n);
        s.album = player.songAlbum(n);
        s.titleLow = lower(s.title);
        s.artistLow = lower(s.artist);
        s.albumLow = lower(s.album);
        s.added = player.songAdded(n);
        s.seconds = fileSeconds(tracks.getTrack(n));
        library.push_back(s);
    }

    state().set_songs_count(songs);               //the number in the sidebar
}


//one song as the screen wants it, the cover is left empty, so the screen draws its gradient
SongInfo UiBridge::infoOf(int number) const
{
    SongInfo info;

    if (number < 1 || number >= (int)library.size())
        return info;

    const Song& s = library[number];

    info.title = slint::SharedString(s.title);
    info.artist = slint::SharedString(s.artist);
    info.album = slint::SharedString(s.album);
    info.date_added = slint::SharedString(ago(s.added));    //the library date. Favorites and playlists keep no date.
    info.length = slint::SharedString(lengthText(s.seconds));
    return info;
}


//the song numbers of a page, oldest first, before search and sort
std::vector<int> UiBridge::pageSource(int page) const
{
    std::vector<int> numbers;

    if (page == ALL_SONGS) {
        for (int n = 1; n < (int)library.size(); n++)
            numbers.push_back(n);
    }
    else if (page == FAVORITES) {
        for (int row = 1; row <= player.favoriteCount(); row++) {
            int n = player.favoriteSongNumber(row);

            if (n > 0)
                numbers.push_back(n);
        }
    }
    else if (openPlaylist >= 0 && openPlaylist < (int)playlistNames.size()) {
        const std::string& name = playlistNames[openPlaylist];

        for (int row = 1; row <= player.playlistSongCount(name); row++) {
            int n = player.playlistSongNumber(name, row);

            if (n > 0)
                numbers.push_back(n);
        }
    }

    return numbers;
}


//fills one page: finds its songs, applies the search and the sort, and gives the rows to the screen
void UiBridge::refreshPage(int page)
{
    SongPage& p = pages[page];
    std::vector<int> all = pageSource(page);
    std::vector<int> rows;

    //search: the text is in the title, the artist or the album
    for (int n : all) {
        const Song& s = library[n];

        if (p.search == "" ||
            s.titleLow.find(p.search) != std::string::npos ||
            s.artistLow.find(p.search) != std::string::npos ||
            s.albumLow.find(p.search) != std::string::npos)
            rows.push_back(n);
    }

    //sort: "recently added" is the list turned around, the others sort by words or by length
    if (p.sort == 0) {
        std::reverse(rows.begin(), rows.end());
    }
    else {
        std::stable_sort(rows.begin(), rows.end(), [&](int a, int b) {
            if (p.sort == 1) return library[a].titleLow < library[b].titleLow;
            if (p.sort == 2) return library[a].artistLow < library[b].artistLow;
            return library[a].seconds < library[b].seconds;
            });
    }

    std::vector<SongInfo> infos;
    double seconds = 0;

    for (int n : rows) {
        infos.push_back(infoOf(n));
        seconds += library[n].seconds;
    }

    p.shown = rows;
    auto model = std::make_shared<slint::VectorModel<SongInfo>>(infos);

    if (page == ALL_SONGS) {
        state().set_songs(model);
        state().set_songs_seconds((float)seconds);
    }
    else if (page == FAVORITES) {
        state().set_favorites(model);
        state().set_favorites_seconds((float)seconds);
        state().set_favorites_count((int)all.size());         //the sidebar number ignores the search
    }
    else {
        state().set_playlist_songs(model);
        state().set_playlist_seconds((float)seconds);
    }
}


//a page was opened: clear its search and sort
void UiBridge::resetPage(int page)
{
    pages[page].search = "";
    pages[page].sort = 0;
    refreshPage(page);
}


//plays row number `row` of a page
void UiBridge::playRow(int page, int row)
{
    if (row < 0 || row >= (int)pages[page].shown.size())
        return;

    player.playSong(pages[page].shown[row]);
    syncPlayerBar(false);
}


//playlist
//reads the playlist names for the sidebar
void UiBridge::refreshPlaylists()
{
    playlistNames.clear();
    std::vector<PlaylistInfo> infos;

    for (int i = 1; i <= player.playlistCount(); i++) {
        PlaylistInfo info;
        info.title = slint::SharedString(player.playlistName(i));    // the cover stays empty: gradient

        playlistNames.push_back(player.playlistName(i));
        infos.push_back(info);
    }

    state().set_playlists(std::make_shared<slint::VectorModel<PlaylistInfo>>(infos));

    if (openPlaylist >= (int)playlistNames.size())
        openPlaylist = -1;
}


//true if the song is in the playlist
bool UiBridge::inPlaylist(const std::string& name, int number) const
{
    for (int row = 1; row <= player.playlistSongCount(name); row++) {
        if (player.playlistSongNumber(name, row) == number)
            return true;
    }
    return false;
}


//"New playlist", or "New playlist 2", "New playlist 3" ... if the name is taken
std::string UiBridge::newPlaylistName() const
{
    std::string name = "New playlist";
    int number = 1;

    while (std::find(playlistNames.begin(), playlistNames.end(), name) != playlistNames.end()) {
        number++;
        name = "New playlist " + std::to_string(number);
    }

    return name;
}


//fills the "+" popup. Playlists that had the song when it opened come first
void UiBridge::refreshChoices()
{
    int cur = player.currentIndex();
    std::vector<PlaylistChoice> rows;
    choiceNames.clear();

    for (int pass = 0; pass < 2; pass++) {
        for (const std::string& name : playlistNames) {
            bool saved = std::find(savedAtOpen.begin(), savedAtOpen.end(), name) != savedAtOpen.end();

            if (saved != (pass == 0))
                continue;

            if (pickerSearch != "" && lower(name).find(pickerSearch) == std::string::npos)
                continue;

            PlaylistChoice choice;
            choice.title = slint::SharedString(name);
            choice.count = player.playlistSongCount(name);
            choice.saved = saved;
            choice.checked = cur != 0 && inPlaylist(name, cur);

            rows.push_back(choice);
            choiceNames.push_back(name);
        }
    }

    state().set_playlist_choices(std::make_shared<slint::VectorModel<PlaylistChoice>>(rows));
}


//sets the heart, and the tick that replaces the "+" when the song is in a playlist
void UiBridge::refreshHeartAndTick()
{
    int cur = player.currentIndex();
    int savedIn = 0;

    if (cur != 0) {
        for (const std::string& name : playlistNames) {
            if (inPlaylist(name, cur))
                savedIn++;
        }
    }

    state().set_current_favorite(cur != 0 && player.isFavorite());
    state().set_track_saved_in(savedIn);
}


//the "+" popup just opened: remember which playlists already have the song
void UiBridge::openPopup()
{
    int cur = player.currentIndex();

    savedAtOpen.clear();
    pickerSearch = "";

    if (cur != 0) {
        for (const std::string& name : playlistNames) {
            if (inPlaylist(name, cur))
                savedAtOpen.push_back(name);
        }
    }

    refreshChoices();
}


//a playlist row in the popup was clicked: add the playing song, or take it out
void UiBridge::toggleInPlaylist(int row)
{
    int cur = player.currentIndex();

    if (cur == 0 || row < 0 || row >= (int)choiceNames.size())
        return;

    std::string name = choiceNames[row];

    if (inPlaylist(name, cur))
        player.removeCurrentFromPlaylist(name);
    else
        player.addCurrentToPlaylist(name);

    refreshChoices();
    refreshHeartAndTick();

    if (openPlaylist >= 0 && playlistNames[openPlaylist] == name)
        refreshPage(PLAYLIST);                    //the playlist on screen just changed
}


//"New playlist" in the popup, make a playlist and put the playing song in it
void UiBridge::newPlaylistFromTrack()
{
    std::string name = newPlaylistName();

    if (!player.createPlaylist(name))
        return;

    if (player.currentIndex() != 0) {
        player.addCurrentToPlaylist(name);
        savedAtOpen.push_back(name);
    }

    refreshPlaylists();
    refreshChoices();
    refreshHeartAndTick();
}


//home page and queue
//fills the two lists of the home page: recent activity and recently added
void UiBridge::refreshHome()
{
    std::vector<TrackInfo> recent;

    for (int row = 1; row <= player.recentCount(); row++) {
        int n = player.recentSongNumber(row);

        if (n < 1)
            continue;

        TrackInfo card;
        card.title = slint::SharedString(library[n].title);
        card.subtitle = slint::SharedString(library[n].artist);
        recent.push_back(card);
    }

    std::vector<TrackInfo> added;

    for (int n = (int)library.size() - 1; n >= 1 && (int)added.size() < HOME_ROWS; n--) {
        TrackInfo card;
        card.title = slint::SharedString(library[n].title);
        card.subtitle = slint::SharedString(library[n].artist);
        added.push_back(card);
    }

    state().set_recent(std::make_shared<slint::VectorModel<TrackInfo>>(recent));
    state().set_recently_added(std::make_shared<slint::VectorModel<TrackInfo>>(added));
}


//fills the queue panel: the whole library in play order, and the recently played tab
void UiBridge::refreshQueue()
{
    int cur = player.currentIndex();
    std::vector<SongInfo> queue;

    if (cur != 0) {
        for (int n = 1; n < (int)library.size(); n++)
            queue.push_back(infoOf(n));
    }

    state().set_queue_tracks(std::make_shared<slint::VectorModel<SongInfo>>(queue));
    state().set_queue_index(cur != 0 ? cur - 1 : 0);
    state().set_queue_context(cur != 0 ? "All Songs" : "");

    recentNumbers.clear();
    std::vector<SongInfo> recent;

    for (int row = 1; row <= player.recentCount(); row++) {
        int n = player.recentSongNumber(row);

        if (n < 1)
            continue;

        recentNumbers.push_back(n);
        recent.push_back(infoOf(n));
    }

    state().set_recently_played(std::make_shared<slint::VectorModel<SongInfo>>(recent));
}


//a home card only knows the title and the artist, so look the song up by those and play it
void UiBridge::playCard(const std::string& title, const std::string& artist)
{
    for (int n = 1; n < (int)library.size(); n++) {
        if (library[n].title == title && library[n].artist == artist) {
            player.playSong(n);
            syncPlayerBar(false);
            return;
        }
    }
}


//player bar
//the play button already flipped AppState.playing, so it tells us what the user wants now
void UiBridge::togglePlay()
{
    bool wantPlay = state().get_playing();

    if (player.currentIndex() == 0) {
        // nothing loaded yet: start with the first row of All Songs
        if (wantPlay && !pages[ALL_SONGS].shown.empty())
            player.playSong(pages[ALL_SONGS].shown[0]);
    }
    else if (wantPlay) {
        player.resume();
    }
    else {
        player.pause();
    }

    syncPlayerBar(false);
}


//the heart was clicked. if nothing is playing, the heart is put back
void UiBridge::setFavorite(bool on)
{
    if (player.currentIndex() != 0 && player.isFavorite() != on)
        player.toggleFavorite();

    refreshPage(FAVORITES);
    refreshHeartAndTick();
}


//copies what the player knows into the player bar. Cheap, so it can run often
void UiBridge::syncPlayerBar(bool force)
{
    int cur = player.currentIndex();

    if (force || cur != lastCurrent) {
        lastCurrent = cur;

        if (cur == 0) {
            state().set_title("");
            state().set_artist("");
        }
        else {
            state().set_title(slint::SharedString(player.songTitle(cur)));
            state().set_artist(slint::SharedString(player.songArtist(cur)));
        }

        //a new song started, so everything that shows "what is playing" or "what was played" changes
        refreshHome();
        refreshQueue();
        refreshHeartAndTick();
    }

    state().set_position((float)player.position());
    state().set_duration((float)player.duration());
    state().set_playing(cur != 0 && !player.isPaused());
}


//other
//"add a new folder": ask for a folder, scan it, and show the new songs
void UiBridge::addFolder()
{
    std::string folder = chooseFolder();

    if (folder == "" || !player.openFolder(folder))
        return;

    loadLibrary();
    refreshPage(ALL_SONGS);
    refreshPage(FAVORITES);
    refreshPage(PLAYLIST);
    refreshHome();
    refreshQueue();
}


//"Show all" on Recently Added: go to All Songs, newest first
void UiBridge::openSongsPage()
{
    state().set_selected_playlist(-1);
    state().set_section(static_cast<decltype(state().get_section())>(1));    //1 = songs
    resetPage(ALL_SONGS);
}