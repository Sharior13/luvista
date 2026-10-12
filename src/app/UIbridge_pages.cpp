#include "UIbridge.h"
#include "uiConvert.h"
#include "../core/osServices.h"
#include <cstdlib>

using UiConvert::toSlint;
using UiConvert::toStd;

// Home, All Songs, Favorites, a playlist, an artist. Every one is "ask the Library which songs, show them".
void UiBridge::connectSongPages()
{
    state().on_song_clicked([this](slint::SharedString list, int row) {
        int page = pageIdOf(list);

        if (page >= 0)
            playRow((PageId)page, row);
        });

    state().on_play_list([this](slint::SharedString list) {
        int page = pageIdOf(list);

        if (page >= 0)
            playRow((PageId)page, 0);
        });

    // the big shuffle button of a page: shuffle on, start somewhere on that page
    state().on_shuffle_list([this](slint::SharedString list) {
        int page = pageIdOf(list);

        if (page < 0 || shown[page].empty())
            return;

        player.setShuffle(true);
        playRow((PageId)page, std::rand() % (int)shown[page].size());
        refreshQueue();
        });

    state().on_list_search_changed([this](slint::SharedString list, slint::SharedString text) {
        int page = pageIdOf(list);

        if (page == RECORDINGS) {
            recordingSearch = toStd(text);
            refreshRecordings();
        }
        else if (page >= 0) {
            library.setSearch(pageOf((PageId)page), toStd(text));
            if (page == ALL_SONGS)
                state().set_search_text("");            // the page's own search replaced the top bar one
            refreshPage((PageId)page);
        }
        });

    state().on_list_sort_changed([this](slint::SharedString list, int index) {
        int page = pageIdOf(list);

        if (page == RECORDINGS) {
            recordingSort = index;
            refreshRecordings();
        }
        else if (page >= 0) {
            library.setSort(pageOf((PageId)page), index);
            refreshPage((PageId)page);
        }
        });

    state().on_add_folder([this] { addFolder(); });
    state().on_remove_music_folder([this](int row) { removeFolder(row); });

    state().on_search_changed([this](slint::SharedString text) { searchEverywhere(toStd(text)); });
    state().on_search_submitted([this](slint::SharedString text) { searchEverywhere(toStd(text)); });

    state().on_section_changed([this](NavSection section) {
        if (section == NavSection::Home) refreshHome();
        if (section == NavSection::Songs) resetPage(ALL_SONGS);
        if (section == NavSection::Favorites) resetPage(FAVORITES);
        if (section == NavSection::Recordings) { recordings.refresh(); resetPage(RECORDINGS); }
        if (section == NavSection::Profile) refreshTopArtists();
        });

    state().on_artist_clicked([this](slint::SharedString name) {
        library.setArtist(toStd(name));
        state().set_artist_name(name);
        resetPage(ARTIST);
        });

    // a card of the home page: play that song
    state().on_track_clicked([this](TrackInfo card) {
        int n = library.findSong(toStd(card.title), toStd(card.subtitle));

        if (n != 0) {
            player.playSong(n);
            syncPlayerBar(false);
        }
        });

    // only "Recently Added" has a page to go to: All Songs, newest first
    state().on_show_all([this](slint::SharedString which) {
        if (toStd(which) == "recently-added")
            openSongsPage();
        });
}


void UiBridge::openSongsPage()
{
    state().set_selected_playlist(-1);
    state().set_section(NavSection::Songs);
    resetPage(ALL_SONGS);
}


// A page was opened: its search and sort start fresh
void UiBridge::resetPage(PageId page)
{
    if (page == RECORDINGS) {
        recordingSearch = "";
        recordingSort = 0;
        refreshRecordings();
        return;
    }

    library.resetPage(pageOf(page));

    if (page == ALL_SONGS)
        state().set_search_text("");

    refreshPage(page);
}


// the Library decides which songs; this turns them into rows
void UiBridge::refreshPage(PageId page)
{
    if (page == RECORDINGS) {
        refreshRecordings();
        return;
    }

    PageResult result = library.build(pageOf(page));
    std::vector<char> favorites = library.favoriteFlags();
    std::vector<SongInfo> rows;

    shown[page] = result.rows;

    for (int n : result.rows)
        rows.push_back(infoOf(n, favorites));

    auto model = std::make_shared<slint::VectorModel<SongInfo>>(rows);
    float seconds = (float)result.seconds;

    if (page == ALL_SONGS) {
        state().set_songs(model);
        state().set_songs_seconds(seconds);
        state().set_songs_count(library.size());
    }
    else if (page == FAVORITES) {
        state().set_favorites(model);
        state().set_favorites_seconds(seconds);
        state().set_favorites_count(result.sourceCount);
    }
    else if (page == PLAYLIST) {
        state().set_playlist_songs(model);
        state().set_playlist_seconds(seconds);
    }
    else {
        state().set_artist_songs(model);
        state().set_artist_seconds(seconds);
        state().set_artist_art(rows.empty() ? slint::Image() : rows[0].art);
    }
}


// a row was clicked: play from there, the page becomes the queue
void UiBridge::playRow(PageId page, int row)
{
    if (row < 0 || row >= (int)shown[page].size())
        return;

    if (page == RECORDINGS) {
        int recording = shown[page][row];
        player.playFilePath(recordings.path(recording), recordings.title(recording));
    }
    else {
        player.playQueue(shown[page], row, library.queueName(pageOf(page)));
    }

    syncPlayerBar(false);
}


// the top bar search: All Songs, with the text in the search
void UiBridge::searchEverywhere(const std::string& text)
{
    library.setEverywhereSearch(text);

    if (!text.empty()) {
        state().set_selected_playlist(-1);
        state().set_section(NavSection::Songs);
    }

    refreshPage(ALL_SONGS);
}


// the "..." menu of a row
void UiBridge::connectSongMenus()
{
    state().on_song_menu_action([this](slint::SharedString list, int row, slint::SharedString action) {
        int page = pageIdOf(list);

        if (page >= 0)
            songMenuAction((PageId)page, row, toStd(action));
        });

    // the "Add to playlist" window of a row
    state().on_song_playlists_requested([this](slint::SharedString list, int row) {
        int page = pageIdOf(list);

        if (page < 0 || page == RECORDINGS || row < 0 || row >= (int)shown[page].size())
            return;

        pickWindow.open(shown[page][row]);
        refreshPickChoices();
        });

    state().on_song_playlist_toggled([this](int row) {
        std::string name = pickWindow.toggle(row);

        if (!name.empty())
            playlistChanged(name);
        });

    state().on_song_playlist_search([this](slint::SharedString text) {
        pickWindow.setSearch(toStd(text));
        refreshPickChoices();
        });

    // confirmed in the "Remove from library" / "Delete recording" window
    state().on_delete_song([this](slint::SharedString list, int row) {
        int page = pageIdOf(list);

        if (page >= 0)
            deleteRow((PageId)page, row);
        });
}


void UiBridge::songMenuAction(PageId page, int row, const std::string& action)
{
    if (row < 0 || row >= (int)shown[page].size())
        return;

    int n = shown[page][row];

    if (action == "play") {
        playRow(page, row);
        return;
    }

    if (page == RECORDINGS) {                           // n is a recording row here, not a song number
        if (action == "show-file")
            Os::showInFolder(recordings.path(n));
        return;
    }

    if (action == "favorite") {
        player.toggleFavoriteSong(n);
        refreshAll();
        refreshHeartAndTick();
    }
    else if (action == "queue-add" || action == "queue-remove") {
        if (action == "queue-add")
            player.addToQueue(n);
        else
            player.removeFromQueue(n);

        refreshAll();                                   // the rows know if they are queued
        refreshQueue();
    }
    else if (action == "show-file") {
        Os::showInFolder(library.path(n));
    }
    else if (action == "remove-from-playlist") {
        if (openPlaylist >= 0 && openPlaylist < (int)playlistNames.size()) {
            const std::string& name = playlistNames[openPlaylist];
            player.removeFromPlaylistRow(name, player.playlistRowOf(name, n));
            playlistChanged(name);
        }
    }
}


void UiBridge::deleteRow(PageId page, int row)
{
    if (row < 0 || row >= (int)shown[page].size())
        return;

    if (page == RECORDINGS) {
        recordings.remove(shown[page][row]);            // a recording has no other copy, so the file goes
        refreshRecordings();
        return;
    }

    player.removeSongFromLibrary(shown[page][row]);
    reloadLibrary();
}


// the three rows of cards on the home page
void UiBridge::refreshHome()
{
    auto cards = [this](const std::vector<int>& numbers) {
        std::vector<TrackInfo> rows;

        for (size_t i = 0; i < numbers.size() && (int)i < HOME_ROWS; i++)
            rows.push_back(cardOf(numbers[i]));

        return std::make_shared<slint::VectorModel<TrackInfo>>(rows);
    };

    state().set_recent(cards(library.recentlyPlayed()));
    state().set_most_played(cards(library.mostPlayed(HOME_ROWS)));
    state().set_recently_added(cards(library.recentlyAdded(HOME_ROWS)));
}


// the Top Artists row of the profile page
void UiBridge::refreshTopArtists()
{
    std::vector<ArtistInfo> tiles;

    for (const std::string& name : library.topArtists(ARTIST_TILES)) {
        ArtistInfo tile;
        tile.name = toSlint(name);

        for (int n : library.songsOfArtist(name)) {     // the first song of the artist that has a picture
            slint::Image art = images.cover(n);

            if (art.size().width > 0) {
                tile.art = art;
                break;
            }
        }

        tiles.push_back(tile);
    }

    state().set_top_artists(std::make_shared<slint::VectorModel<ArtistInfo>>(tiles));
}


// "128 songs in 2 folders, 3 recordings"
void UiBridge::refreshStorage()
{
    std::string text = UiConvert::countOf(library.size(), "song") + " in " + UiConvert::countOf(folders.count(), "folder");

    text += ", " + UiConvert::countOf(recordings.count(), "recording");
    state().set_storage_text(toSlint(text));
    refreshFolders();
}


// the list in the settings: the default folder first, then the ones the user added
void UiBridge::refreshFolders()
{
    std::vector<MusicFolderInfo> rows;

    for (int row = 0; row < folders.count(); row++) {
        MusicFolderInfo info;
        info.path = toSlint(folders.path(row));
        info.is_default = folders.isDefault(row);
        rows.push_back(info);
    }

    state().set_music_folders(std::make_shared<slint::VectorModel<MusicFolderInfo>>(rows));
}


// "Add folder": the user picks one, it joins the list and its songs join the library
void UiBridge::addFolder()
{
    std::string folder = Os::chooseFolder();

    if (folder.empty())
        return;

    std::string why;

    state().set_folder_error(folders.add(folder, why) ? "" : toSlint(why));
    reloadLibrary();
}


// confirmed in the "Remove folder?" window: its songs leave the library, the files stay
void UiBridge::removeFolder(int row)
{
    std::string why;

    state().set_folder_error(folders.remove(row, why) ? "" : toSlint(why));
    reloadLibrary();
}
