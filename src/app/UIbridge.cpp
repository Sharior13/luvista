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
#include "../audio/CoverArt.h"
#include "../core/appPaths.h"

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <commdlg.h>
#endif

namespace fs = std::filesystem;

const int ALL_SONGS = 0;
const int FAVORITES = 1;
const int PLAYLIST = 2;
const int RECORDINGS = 3;
const int PAGE_COUNT = 4;
const int HOME_ROWS = 12;                 //how many songs "Recently Added" and "Most Played" show on the home page


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

//true when s is valid UTF-8. Slint aborts the whole program (not just the one string) on anything else.
static bool isUtf8(const std::string& s)
{
    const unsigned char* p = (const unsigned char*)s.data();
    size_t n = s.size();
    size_t i = 0;

    while (i < n) {
        unsigned char c = p[i];

        if (c < 0x80) {
            i++;
            continue;
        }

        int len;
        unsigned int cp;

        if (c >= 0xC2 && c <= 0xDF) { len = 2; cp = c & 0x1F; }
        else if (c >= 0xE0 && c <= 0xEF) { len = 3; cp = c & 0x0F; }
        else if (c >= 0xF0 && c <= 0xF4) { len = 4; cp = c & 0x07; }
        else
            return false;                                   //a stray byte, e.g. a Windows-1252 letter like e-acute or a curly apostrophe

        if (i + len > n)
            return false;                                   //the character is cut off

        for (int k = 1; k < len; k++) {
            if ((p[i + k] & 0xC0) != 0x80)
                return false;

            cp = (cp << 6) | (p[i + k] & 0x3F);
        }

        if (len == 3 && (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)))
            return false;                                   //too long a form, or a lone surrogate

        if (len == 4 && (cp < 0x10000 || cp > 0x10FFFF))
            return false;

        i += len;
    }

    return true;
}

//a normal string as a slint string. Text that is not valid UTF-8 (file names come from windows
//in the ANSI code page) is converted first, so one odd song name can never crash the program.
static slint::SharedString toSlint(const std::string& text)
{
    if (isUtf8(text))
        return slint::SharedString(std::string_view(text));

    std::string fixed;

#ifdef _WIN32
    int wideLength = MultiByteToWideChar(CP_ACP, 0, text.data(), (int)text.size(), nullptr, 0);

    if (wideLength > 0) {
        std::wstring wide((size_t)wideLength, L'\0');
        MultiByteToWideChar(CP_ACP, 0, text.data(), (int)text.size(), &wide[0], wideLength);

        int utf8Length = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLength, nullptr, 0, nullptr, nullptr);

        if (utf8Length > 0) {
            fixed.assign((size_t)utf8Length, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLength, &fixed[0], utf8Length, nullptr, nullptr);
        }
    }
#endif

    if (fixed.empty() || !isUtf8(fixed)) {                  //last resort: read every byte as a Latin-1 letter
        fixed.clear();

        for (unsigned char c : text) {
            if (c < 0x80) {
                fixed += (char)c;
            }
            else {
                fixed += (char)(0xC0 | (c >> 6));
                fixed += (char)(0x80 | (c & 0x3F));
            }
        }
    }

    return slint::SharedString(std::string_view(fixed));
}

//which song page a slint list name means
static int pageNumber(const slint::SharedString& name)
{
    std::string n = toStd(name);

    if (n == "songs") return ALL_SONGS;
    if (n == "favorites") return FAVORITES;
    if (n == "playlist") return PLAYLIST;
    if (n == "recordings") return RECORDINGS;
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

//is this file somewhere inside that folder?
static bool isInside(const std::string& file, const std::string& folder)
{
    fs::path f = fs::path(file).lexically_normal();
    fs::path d = fs::path(folder).lexically_normal();

    auto fileIt = f.begin();

    for (auto dirIt = d.begin(); dirIt != d.end(); ++dirIt, ++fileIt) {
        if (dirIt->empty())                  //a folder written with a "/" at the end has an empty last part
            break;

        if (fileIt == f.end() || *fileIt != *dirIt)
            return false;
    }

    return true;
}

//the first free "rec_1.mp3", "rec_2.mp3" ... inside a folder.
//(Recorder::nextRecordingPath() always uses Documents\Sound Recordings, but every profile keeps its own recordings,
//so the bridge picks the name itself and hands the whole path to Recorder::start())
static std::string nextRecordingFile(const std::string& folder)
{
    std::error_code problem;

    for (int number = 1; number < 100000; number++) {
        fs::path candidate = fs::path(folder) / ("rec_" + std::to_string(number) + ".mp3");

        if (!fs::exists(candidate, problem))
            return candidate.string();
    }

    return (fs::path(folder) / "rec_new.mp3").string();
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

//opens the windows "pick a picture" window (png or jpg), gives back "" if the user cancels
static std::string chooseImage()
{
#ifdef _WIN32
    wchar_t path[MAX_PATH] = {};

    OPENFILENAMEW info = {};
    info.lStructSize = sizeof(info);
    info.lpstrFilter = L"Pictures (*.png;*.jpg;*.jpeg)\0*.png;*.jpg;*.jpeg\0";
    info.lpstrFile = path;
    info.nMaxFile = MAX_PATH;
    info.lpstrTitle = L"Choose a picture";
    info.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (!GetOpenFileNameW(&info))
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
    for (int i = 0; i < PAGE_COUNT; i++)
        pages[i].sort = 0;

    openPlaylist = -1;
    lastCurrent = -1;
    lastLyric = -2;
}


//fills the screen, hooks up the buttons and starts the clock, call once before ui.run()
void UiBridge::start()
{
    std::srand((unsigned)std::time(nullptr));

    state().set_volume(player.volume() / 100.0f); //the screen starts at 100%, the player at 50%

    //only the login screen is showing, so only things that do not belong to a profile are filled in.
    //favorites, playlists, recent songs and so on are filled in by enterProfile() once someone logs in
    loadLibrary();
    refreshProfileList();

    connectPlayerBar();
    connectSongPages();
    connectPlaylists();
    connectOtherScreens();
    connectSession();

    //four times a second, move the seek bar, and go to the next song when one ends
    timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(250), [this] {
        if (!session().get_logged_in())
            return;

        player.update();
        syncPlayerBar(false);
        });
}


//play, next, previous, seek, volume, shuffle, repeat and the heart
void UiBridge::connectPlayerBar()
{
    state().on_toggle_play([this] { togglePlay(); });
    state().on_next([this] { player.next(); syncPlayerBar(false); });
    state().on_previous([this] { player.previous(); syncPlayerBar(false); });
    state().on_seek([this](float seconds) { player.seekTo(seconds); syncPlayerBar(false); });
    state().on_volume_changed([this](float v) { player.setVolume((int)(v * 100 + 0.5f)); });
    state().on_favorite_toggled([this](bool on) { setFavorite(on); });

    //the queue is reordered, the playing song stays where it is
    state().on_shuffle_toggled([this](bool on) {
        player.setShuffle(on);
        refreshQueue();
        });

    //repeat means "play this song again when it ends". The next button still moves on
    state().on_repeat_toggled([this](bool on) { player.setRepeat(on); });
}


//rows, play, shuffle, search, sort and "add a new folder" on all songs, favorites, playlists and recordings
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

    //shuffle goes on, the list is queued in random order and a random song starts
    state().on_shuffle_list([this](slint::SharedString list) {
        int page = pageNumber(list);

        if (page < 0 || pages[page].shown.empty())
            return;

        player.playQueueShuffled(pages[page].shown, pageName(page));
        state().set_shuffle(true);
        syncPlayerBar(true);
        });

    state().on_list_search_changed([this](slint::SharedString list, slint::SharedString text) {
        int page = pageNumber(list);

        if (page < 0)
            return;

        if (page == ALL_SONGS)
            globalSearch = "";                    //the page's own box takes over from the top bar

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

    state().on_edit_playlist_art([this](int i) { editPlaylistArt(i); });

    state().on_add_current_track([this] { openPopup(); });
    state().on_playlist_toggled([this](int row) { toggleInPlaylist(row); });
    state().on_new_playlist_from_track([this] { newPlaylistFromTrack(); });

    state().on_playlist_picker_search([this](slint::SharedString text) {
        pickerSearch = lower(toStd(text));
        refreshChoices();
        });
}


//sidebar pages, the home cards, the queue panel, the top bar and recording
void UiBridge::connectOtherScreens()
{
    //pages are built again when opened, so their search box and sort start fresh
    state().on_section_changed([this](auto section) {
        int s = static_cast<int>(section);        //0 home, 1 songs, 2 favorites, 3 recordings

        if (s == 0) refreshHome();
        if (s == 1) resetPage(ALL_SONGS);
        if (s == 2) resetPage(FAVORITES);
        if (s == 3) resetPage(RECORDINGS);
        });

    state().on_track_clicked([this](TrackInfo card) {
        playCard(toStd(card.title), toStd(card.subtitle));
        });

    //only "Recently Added" has a page to go to: All Songs, newest first
    state().on_show_all([this](slint::SharedString which) {
        if (toStd(which) == "recently-added")
            openSongsPage();
        });

    //a row of the queue panel: play from there, the queue stays as it is
    state().on_queue_track_clicked([this](int row) {
        player.playQueueRow(row);
        syncPlayerBar(true);
        });

    //the "Recently played" tab: the queue becomes that list
    state().on_recent_track_clicked([this](int row) {
        if (row >= 0 && row < (int)recentNumbers.size()) {
            player.playQueue(recentNumbers, row, "Recently played");
            syncPlayerBar(true);
        }
        });

    //the top bar looks through everything, whatever page is open. The results are shown on the All Songs page
    state().on_search_changed([this](slint::SharedString text) { searchEverywhere(toStd(text)); });
    state().on_search_submitted([this](slint::SharedString text) { searchEverywhere(toStd(text)); });

    //the picture in the top bar goes back to the login screen, to pick another profile
    state().on_profile_clicked([this] { leaveProfile(); });

    //the record button starts the microphone, and the next click stops it and saves the file
    state().on_start_recording([this] { toggleRecording(); });
}


//the login screen: profiles, add profile, guest mode
void UiBridge::connectSession()
{
    session().on_select_profile([this](int i) {
        if (i >= 0 && i < (int)profileNames.size())
            enterProfile(profileNames[i], false);
        });

    //the screen has no name box yet, so new profiles are called "Profile 1", "Profile 2"
    session().on_add_profile([this] {
        if (profiles.create(profiles.nextName()))
            refreshProfileList();
        });

    //guest mode starts empty and leaves nothing behind
    session().on_continue_as_guest([this] {
        profiles.resetGuest();
        enterProfile(Profiles::GUEST, true);
        });

    //login(name, password) picks a profile by its name. Profiles have no passwords yet, so the password is not looked at
    session().on_login([this](slint::SharedString name, slint::SharedString) {
        std::string found = profiles.find(toStd(name));

        if (found == "") {
            session().set_error(toSlint("There is no profile called \"" + toStd(name) + "\""));
            return;
        }

        enterProfile(found, false);
        });

    session().on_logout([this] { leaveProfile(); });
}


//library and song pages

//copies every song out of the librarian. this also reads each file's length, so the first time it can take a moment
void UiBridge::loadLibrary()
{
    library.clear();
    library.push_back(Song());                    //slot 0, never used

    int songs = player.total();
    int visible = 0;

    //recordings of every profile live inside the profiles folder, the ones of this profile in its own folder
    std::string allRecordings = AppPaths::profilesFolder();
    std::string myRecordings = AppPaths::recordingsFolder();

    for (int n = 1; n <= songs; n++) {
        Song s;
        s.number = n;
        s.id = tracks.getId(n);
        s.path = tracks.getTrack(n);
        s.title = player.songTitle(n);
        s.artist = player.songArtist(n);
        s.album = player.songAlbum(n);
        s.titleLow = lower(s.title);
        s.artistLow = lower(s.artist);
        s.albumLow = lower(s.album);
        s.added = player.songAdded(n);
        s.recording = isInside(s.path, allRecordings);
        s.mine = s.recording && isInside(s.path, myRecordings);
        s.seconds = lengthOf(s);
        library.push_back(s);

        if (!s.recording)
            visible++;
    }

    state().set_songs_count(visible);             //the number in the sidebar
}


//is this a song number that exists?
bool UiBridge::valid(int number) const
{
    return number >= 1 && number < (int)library.size();
}


//the length of a song. Opening a file to measure it is slow, so each song is measured only once
double UiBridge::lengthOf(const Song& s)
{
    auto found = lengths.find(s.id);

    if (found != lengths.end())
        return found->second;

    double seconds = fileSeconds(s.path);

    if (seconds > 0)
        lengths[s.id] = seconds;

    return seconds;
}


//a picture file as the screen wants it. Empty if the file cannot be read
slint::Image UiBridge::imageFile(const std::string& path)
{
    if (path == "")
        return slint::Image();

    auto found = images.find(path);

    if (found != images.end())
        return found->second;

    slint::Image picture = slint::Image::load_from_path(toSlint(path));
    images[path] = picture;
    return picture;
}


//the album picture of a song. Empty means the screen draws its gradient.
//recordings never get one, their rows show the microphone instead
slint::Image UiBridge::coverOf(int number)
{
    if (!valid(number) || library[number].recording)
        return slint::Image();

    const Song& s = library[number];
    auto found = coverPaths.find(s.id);

    if (found == coverPaths.end())
        found = coverPaths.insert({ s.id, CoverArt::forSong(s.id, s.path) }).first;

    return imageFile(found->second);
}


//the picture of a playlist: the one the user chose, or else the cover of its first song that has one
slint::Image UiBridge::playlistImage(const std::string& name)
{
    std::string chosen = player.playlistArt(name);

    if (chosen != "")
        return imageFile(chosen);

    int rows = std::min(player.playlistSongCount(name), 5);

    for (int row = 1; row <= rows; row++) {
        slint::Image cover = coverOf(player.playlistSongNumber(name, row));

        if (cover.size().width > 0)
            return cover;
    }

    return slint::Image();
}


//one song as the screen wants it
SongInfo UiBridge::infoOf(int number)
{
    SongInfo info;

    if (!valid(number))
        return info;

    const Song& s = library[number];

    info.title = toSlint(s.title);
    info.artist = toSlint(s.artist);
    info.album = toSlint(s.album);
    info.date_added = toSlint(ago(s.added));    //the library date. Favorites and playlists keep no date.
    info.length = toSlint(lengthText(s.seconds));
    info.art = coverOf(number);
    return info;
}


//one song as a card on the home page
TrackInfo UiBridge::cardOf(int number)
{
    TrackInfo card;

    if (!valid(number))
        return card;

    card.title = toSlint(library[number].title);
    card.subtitle = toSlint(library[number].artist);
    card.art = coverOf(number);
    return card;
}


//the song numbers of a page, oldest first, before search and sort
std::vector<int> UiBridge::pageSource(int page) const
{
    std::vector<int> numbers;

    if (page == ALL_SONGS) {
        for (int n = 1; n < (int)library.size(); n++) {
            //while the top bar is searching, this page also holds the recordings
            if (!library[n].recording || (globalSearch != "" && library[n].mine))
                numbers.push_back(n);
        }
    }
    else if (page == RECORDINGS) {
        for (int n = 1; n < (int)library.size(); n++) {
            if (library[n].mine)
                numbers.push_back(n);
        }
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


//the name the queue panel shows for a page: "Next from: Favorites"
std::string UiBridge::pageName(int page) const
{
    if (page == ALL_SONGS) return globalSearch != "" ? "Search results" : "All Songs";
    if (page == FAVORITES) return "Favorites";
    if (page == RECORDINGS) return "Recordings";

    if (openPlaylist >= 0 && openPlaylist < (int)playlistNames.size())
        return playlistNames[openPlaylist];

    return "Playlist";
}


//fills one page: finds its songs, applies the search and the sort, and gives the rows to the screen
void UiBridge::refreshPage(int page)
{
    SongPage& p = pages[page];
    std::vector<int> all = pageSource(page);
    std::vector<int> rows;

    //while the top bar is searching, the songs of a playlist whose name has the text are hits too
    std::vector<int> inMatchingPlaylist;

    if (page == ALL_SONGS && globalSearch != "") {
        for (const std::string& name : playlistNames) {
            if (lower(name).find(globalSearch) == std::string::npos)
                continue;

            for (int row = 1; row <= player.playlistSongCount(name); row++)
                inMatchingPlaylist.push_back(player.playlistSongNumber(name, row));
        }
    }

    //search: the text is in the title, the artist or the album
    for (int n : all) {
        const Song& s = library[n];

        if (p.search == "" ||
            std::find(inMatchingPlaylist.begin(), inMatchingPlaylist.end(), n) != inMatchingPlaylist.end() ||
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
    else if (page == RECORDINGS) {
        state().set_recordings(model);
        state().set_recordings_count((int)all.size());
    }
    else {
        state().set_playlist_songs(model);
        state().set_playlist_seconds((float)seconds);
    }
}


//fills every page
void UiBridge::refreshAllPages()
{
    for (int page = 0; page < PAGE_COUNT; page++)
        refreshPage(page);
}


//a page was opened: clear its search and sort (and the search box in the top bar)
void UiBridge::resetPage(int page)
{
    globalSearch = "";
    pages[page].search = "";
    pages[page].sort = 0;
    state().set_search_text("");
    refreshPage(page);
}


//plays row number `row` of a page. The rest of the page becomes the queue
void UiBridge::playRow(int page, int row)
{
    if (row < 0 || row >= (int)pages[page].shown.size())
        return;

    player.playQueue(pages[page].shown, row, pageName(page));
    syncPlayerBar(true);                          //the queue changed even if the song did not
}


//the search box in the top bar. It looks through everything: all songs, recordings, favorites and playlists
//(a song is a hit if its title, artist or album has the text, or if it is in a playlist with that name).
//There is no results page yet, so the hits are shown on the All Songs page, whichever page was open.
//Emptying the box ends the search
void UiBridge::searchEverywhere(const std::string& text)
{
    globalSearch = lower(text);
    pages[ALL_SONGS].search = globalSearch;

    if (globalSearch != "") {
        state().set_selected_playlist(-1);
        state().set_section(static_cast<decltype(state().get_section())>(1));    //1 = songs
    }

    refreshPage(ALL_SONGS);
}


//playlist
//reads the playlist names and pictures for the sidebar
void UiBridge::refreshPlaylists()
{
    playlistNames.clear();
    std::vector<PlaylistInfo> infos;

    for (int i = 1; i <= player.playlistCount(); i++) {
        PlaylistInfo info;
        info.title = toSlint(player.playlistName(i));
        info.art = playlistImage(player.playlistName(i));       // empty: gradient

        playlistNames.push_back(player.playlistName(i));
        infos.push_back(info);
    }

    state().set_playlists(std::make_shared<slint::VectorModel<PlaylistInfo>>(infos));

    if (openPlaylist >= (int)playlistNames.size())
        openPlaylist = -1;
}


//click on the open playlist's picture: pick an image and use it
void UiBridge::editPlaylistArt(int index)
{
    if (index < 0 || index >= (int)playlistNames.size())
        return;

    std::string name = playlistNames[index];
    std::string file = chooseImage();

    if (file == "")
        return;

    images.erase(player.playlistArt(name));       //forget the old picture, a new one is about to take its place

    if (!player.setPlaylistArt(name, file))
        return;

    images.erase(player.playlistArt(name));
    refreshPlaylists();
    refreshChoices();
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
            choice.title = toSlint(name);
            choice.art = playlistImage(name);
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

    refreshPlaylists();                           //a playlist without a chosen picture borrows a song's cover
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


//home page, queue and lyrics
//fills the three lists of the home page: recent activity, most played and recently added
void UiBridge::refreshHome()
{
    std::vector<TrackInfo> recent;

    for (int row = 1; row <= player.recentCount(); row++) {
        int n = player.recentSongNumber(row);

        if (valid(n))
            recent.push_back(cardOf(n));
    }

    std::vector<TrackInfo> popular;

    for (int n : player.mostPlayedSongNumbers(HOME_ROWS)) {
        if (valid(n))
            popular.push_back(cardOf(n));
    }

    std::vector<TrackInfo> added;

    for (int n = (int)library.size() - 1; n >= 1 && (int)added.size() < HOME_ROWS; n--) {
        if (!library[n].recording)
            added.push_back(cardOf(n));
    }

    state().set_recent(std::make_shared<slint::VectorModel<TrackInfo>>(recent));
    state().set_most_played(std::make_shared<slint::VectorModel<TrackInfo>>(popular));
    state().set_recently_added(std::make_shared<slint::VectorModel<TrackInfo>>(added));
}


//fills the queue panel: the queue in play order, and the recently played tab
void UiBridge::refreshQueue()
{
    std::vector<SongInfo> queue;

    for (int row = 0; row < player.queueCount(); row++)
        queue.push_back(infoOf(player.queueSongNumber(row)));     //a song that is gone gives an empty row, so row numbers stay right

    state().set_queue_tracks(std::make_shared<slint::VectorModel<SongInfo>>(queue));
    state().set_queue_index(std::max(player.queuePosition(), 0));
    state().set_queue_context(toSlint(player.queueContext()));

    recentNumbers.clear();
    std::vector<SongInfo> recent;

    for (int row = 1; row <= player.recentCount(); row++) {
        int n = player.recentSongNumber(row);

        if (!valid(n))
            continue;

        recentNumbers.push_back(n);
        recent.push_back(infoOf(n));
    }

    state().set_recently_played(std::make_shared<slint::VectorModel<SongInfo>>(recent));
}


//a new song started: load its lyrics (the .lrc file next to it). The line being sung is set by syncPlayerBar
void UiBridge::refreshLyrics(int current)
{
    std::vector<LyricLine> rows;

    if (valid(current) && lyrics.load(library[current].path)) {
        for (int i = 0; i < lyrics.count(); i++) {
            LyricLine line;
            line.time = (float)lyrics.time(i);
            line.text = toSlint(lyrics.text(i));
            rows.push_back(line);
        }
    }
    else {
        lyrics.clear();
    }

    state().set_lyrics(std::make_shared<slint::VectorModel<LyricLine>>(rows));
    state().set_lyric_index(-1);
    lastLyric = -1;
}


//a home card only knows the title and the artist, so look the song up by those and play it
void UiBridge::playCard(const std::string& title, const std::string& artist)
{
    for (int n = 1; n < (int)library.size(); n++) {
        if (library[n].title != title || library[n].artist != artist)
            continue;

        //the queue is the list the song belongs to: All Songs, or the recordings
        int page = library[n].recording ? RECORDINGS : ALL_SONGS;
        std::vector<int> list = pageSource(page);
        auto at = std::find(list.begin(), list.end(), n);

        if (at == list.end()) {
            list = { n };                         //a recording of another profile cannot be here, but never crash on it
            at = list.begin();
        }

        player.playQueue(list, (int)(at - list.begin()), pageName(page));
        syncPlayerBar(true);
        return;
    }
}


//player bar
//the play button already flipped AppState.playing, so it tells us what the user wants now
void UiBridge::togglePlay()
{
    bool wantPlay = state().get_playing();

    if (player.currentIndex() == 0) {
        // nothing loaded yet: start with the first row of All Songs
        if (wantPlay)
            playRow(ALL_SONGS, 0);
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
            state().set_title(toSlint(player.songTitle(cur)));
            state().set_artist(toSlint(player.songArtist(cur)));
        }

        state().set_cover_art(cur == 0 ? slint::Image() : coverOf(cur));
        refreshLyrics(cur);

        //a new song started, so everything that shows "what is playing" or "what was played" changes
        refreshHome();
        refreshQueue();
        refreshHeartAndTick();
    }

    float position = (float)player.position();

    state().set_position(position);
    state().set_duration((float)player.duration());
    state().set_playing(cur != 0 && !player.isPaused());

    //move the highlight to the line being sung
    int line = lyrics.count() > 0 ? lyrics.indexAt(position) : -1;

    if (line != lastLyric) {
        lastLyric = line;
        state().set_lyric_index(line);
    }
}


//other
//"add a new folder": ask for a folder, scan it, and show the new songs
void UiBridge::addFolder()
{
    std::string folder = chooseFolder();

    if (folder == "" || !player.openFolder(folder))
        return;

    loadLibrary();
    refreshAllPages();
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


//recording
//the record button: start the microphone, or (if it is already running) stop it and keep the file
void UiBridge::toggleRecording()
{
    if (recorder.isRecording()) {
        finishRecording();
        return;
    }

    std::string file = nextRecordingFile(AppPaths::recordingsFolder());

    if (!recorder.start(file))
        std::fprintf(stderr, "Luvista: cannot record: %s\n", recorder.lastError().c_str());
}


//stops the microphone, and the new recording shows up on the Recordings page
void UiBridge::finishRecording()
{
    std::string saved = recorder.stop();

    if (saved == "") {
        std::fprintf(stderr, "Luvista: the recording was not saved: %s\n", recorder.lastError().c_str());
        return;
    }

    player.openFolder(AppPaths::recordingsFolder());      //the librarian finds the new file. Old songs keep their numbers
    loadLibrary();
    refreshAllPages();
    refreshHome();
    refreshQueue();
}


//profiles
//the cards on the login screen
void UiBridge::refreshProfileList()
{
    profileNames = profiles.list();
    std::vector<ProfileInfo> cards;

    for (const std::string& name : profileNames) {
        ProfileInfo card;
        card.name = toSlint(name);
        card.picture = imageFile(profiles.picture(name));       // empty: gradient with the icon
        cards.push_back(card);
    }

    session().set_profiles(std::make_shared<slint::VectorModel<ProfileInfo>>(cards));
}


//a profile was picked: from now on favorites, playlists, recent songs, play counts and recordings are its own
void UiBridge::enterProfile(const std::string& name, bool guest)
{
    AppPaths::setActiveProfile(name);

    openPlaylist = -1;
    globalSearch = "";

    for (int i = 0; i < PAGE_COUNT; i++) {
        pages[i].search = "";
        pages[i].sort = 0;
    }

    player.openFolder(AppPaths::recordingsFolder());      //this profile's recordings join the library

    state().set_profile_initial(toSlint(guest ? "G" : Profiles::initial(name)));
    state().set_profile_picture(guest ? slint::Image() : imageFile(profiles.picture(name)));
    state().set_search_text("");
    state().set_selected_playlist(-1);
    state().set_section(static_cast<decltype(state().get_section())>(0));    //0 = home

    loadLibrary();
    refreshPlaylists();
    refreshAllPages();
    syncPlayerBar(true);                          //also fills the home page, the queue and the heart

    session().set_error("");
    session().set_logged_in(true);
}


//back to the login screen: the music stops, a recording in progress is saved
void UiBridge::leaveProfile()
{
    if (recorder.isRecording())
        finishRecording();                        //this still belongs to the profile that is leaving

    player.stop();

    state().set_playing(false);
    state().set_position(0);
    state().set_duration(0);
    state().set_shuffle(false);
    state().set_repeat(false);
    player.setShuffle(false);
    player.setRepeat(false);
    syncPlayerBar(true);

    AppPaths::setActiveProfile(Profiles::GUEST);  //nobody is logged in, so nothing is anyone's
    refreshProfileList();
    session().set_logged_in(false);
}