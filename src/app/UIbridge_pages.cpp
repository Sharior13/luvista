#include "UIbridge.h"
#include "uiConvert.h"
#include "../core/osServices.h"
#include <algorithm>
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
            clearTicks();
            refreshRecordings();
        }
        else if (page >= 0) {
            library.setSearch(pageOf((PageId)page), toStd(text));
            if (page == ALL_SONGS)
                state().set_search_text("");            // the page's own search replaced the top bar one
            clearTicks();                               // the rows are different ones now
            refreshPage((PageId)page);
        }
        });

    state().on_list_sort_changed([this](slint::SharedString list, int index) {
        int page = pageIdOf(list);

        if (page == RECORDINGS) {
            recordingSort = index;
            clearTicks();
            refreshRecordings();
        }
        else if (page >= 0) {
            library.setSort(pageOf((PageId)page), index);
            clearTicks();
            refreshPage((PageId)page);
        }
        });

    state().on_add_folder([this] { addFolder(); });
    state().on_remove_music_folder([this](int row) { removeFolder(row); });

    state().on_search_changed([this](slint::SharedString text) { searchEverywhere(toStd(text)); });
    state().on_search_submitted([this](slint::SharedString text) { searchEverywhere(toStd(text)); });

    state().on_section_changed([this](NavSection section) {
        endSelecting();                                 // ticks belong to the page they were made on

        if (section == NavSection::Home) refreshHome();
        if (section == NavSection::Songs) resetPage(ALL_SONGS);
        if (section == NavSection::Favorites) resetPage(FAVORITES);
        if (section == NavSection::Recordings) { recordings.refresh(); resetPage(RECORDINGS); }
        if (section == NavSection::Albums) { albumSearch = ""; albumSort = 0; refreshAlbums(); }
        if (section == NavSection::Artists) { artistSearch = ""; artistSort = 0; refreshArtists(); }
        if (section == NavSection::Profile) { refreshTopArtists(); refreshTopSongs(); }
        });

    state().on_artist_clicked([this](slint::SharedString name) { openArtist(toStd(name)); });

    // the Albums page
    state().on_album_clicked([this](int index) {
        if (index >= 0 && index < (int)albumEntries.size())
            openAlbum(albumEntries[index].name);
        });

    state().on_albums_search_changed([this](slint::SharedString text) {
        albumSearch = toStd(text);
        refreshAlbums();
        });

    state().on_albums_sort_changed([this](int index) {
        albumSort = index;
        refreshAlbums();
        });

    // the Artists page
    state().on_artist_tile_clicked([this](int index) {
        if (index >= 0 && index < (int)artistEntries.size())
            openArtist(artistEntries[index].name);
        });

    state().on_artists_search_changed([this](slint::SharedString text) {
        artistSearch = toStd(text);
        refreshArtists();
        });

    state().on_artists_sort_changed([this](int index) {
        artistSort = index;
        refreshArtists();
        });

    // any artist or album name anywhere on the screen
    ui.global<Nav>().on_open_artist([this](slint::SharedString name) { openArtist(toStd(name)); });
    ui.global<Nav>().on_open_album([this](slint::SharedString name) { openAlbum(toStd(name)); });

    // the Most Played list of the profile page
    state().on_top_song_clicked([this](int row) {
        if (row >= 0 && row < (int)topSongNumbers.size()) {
            player.playQueue(topSongNumbers, row, "Most played");
            syncPlayerBar(false);
        }
        });

    // a card of the home page: play that song
    state().on_track_clicked([this](TrackInfo card) {
        int n = library.findSong(toStd(card.title), toStd(card.subtitle));

        if (n != 0) {
            player.playSong(n);
            syncPlayerBar(false);
        }
        });

    // "Show all" of a home row: the page of that row
    state().on_show_all([this](slint::SharedString which) {
        std::string name = toStd(which);

        if (name == "recent")
            openPage(NavSection::Recent, RECENT);
        else if (name == "most-played")
            openPage(NavSection::MostPlayed, MOST_PLAYED);
        else if (name == "recently-added")
            openPage(NavSection::RecentlyAdded, RECENTLY_ADDED);
    });
}


void UiBridge::openPage(NavSection section, PageId page)
{
    endSelecting();
    state().set_selected_playlist(-1);
    state().set_section(section);
    resetPage(page);
}


// the artist page, from any artist name or a Top Artist tile
void UiBridge::openArtist(const std::string& name)
{
    endSelecting();
    library.setArtist(name);
    state().set_artist_name(toSlint(name));
    state().set_selected_playlist(-1);
    state().set_section(NavSection::Artist);
    resetPage(ARTIST);
}


// the album page, from any album name or a tile of the Albums page
void UiBridge::openAlbum(const std::string& name)
{
    endSelecting();
    library.setAlbum(name);
    state().set_album_name(toSlint(name));
    state().set_album_subtitle(toSlint(library.albumArtist(name)));
    state().set_selected_playlist(-1);
    state().set_section(NavSection::Album);
    resetPage(ALBUM);
}


void UiBridge::openSongsPage()
{
    endSelecting();
    state().set_selected_playlist(-1);
    state().set_section(NavSection::Songs);
    resetPage(ALL_SONGS);
}


// A page was opened: its search and sort start fresh
void UiBridge::resetPage(PageId page)
{
    endSelecting();

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


// the first picture among the first few songs of a list (the banner of an artist or album page)
slint::Image UiBridge::firstArt(const std::vector<int>& numbers)
{
    for (size_t i = 0; i < numbers.size() && i < 8; i++) {
        slint::Image art = images.cover(numbers[i]);

        if (art.size().width > 0)
            return art;
    }

    return slint::Image();
}


// the Library decides which songs; this turns them into rows. The rows are only made when they are looked at.
void UiBridge::refreshPage(PageId page)
{
    if (page == RECORDINGS) {
        refreshRecordings();
        return;
    }

    PageResult result = library.build(pageOf(page));

    shown[page] = result.rows;
    keepSelection(page, (int)result.rows.size());

    auto numbers = std::make_shared<std::vector<int>>(result.rows);
    auto favorites = std::make_shared<std::vector<char>>(library.favoriteFlags());

    auto model = std::make_shared<LazyModel<SongInfo>>((int)numbers->size(), [this, page, numbers, favorites](int row) {
        SongInfo info = infoOf((*numbers)[row], *favorites);
        info.selected = selectionPage == (int)page && selection.contains(row);
        return info;
        });

    models[page] = model;
    float seconds = (float)result.seconds;

    switch (page) {
    case ALL_SONGS:
        state().set_songs(model);
        state().set_songs_seconds(seconds);
        break;
    case FAVORITES:
        state().set_favorites(model);
        state().set_favorites_seconds(seconds);
        state().set_favorites_count(result.sourceCount);
        break;
    case PLAYLIST:
        state().set_playlist_songs(model);
        state().set_playlist_seconds(seconds);
        break;
    case ARTIST:
        state().set_artist_songs(model);
        state().set_artist_seconds(seconds);
        state().set_artist_art(firstArt(result.rows));
        break;
    case ALBUM:
        state().set_album_songs(model);
        state().set_album_seconds(seconds);
        state().set_album_art(firstArt(result.rows));
        break;
    case RECENT:
        state().set_recent_songs(model);
        state().set_recent_seconds(seconds);
        break;
    case MOST_PLAYED:
        state().set_most_played_songs(model);
        state().set_most_played_seconds(seconds);
        break;
    case RECENTLY_ADDED:
        state().set_recently_added_songs(model);
        state().set_recently_added_seconds(seconds);
        break;
    default:
        break;
    }
}


// the Albums page: one tile per album, made when it scrolls into view
void UiBridge::refreshAlbums()
{
    albumEntries = library.albums(albumSearch, albumSort);

    auto entries = std::make_shared<std::vector<AlbumEntry>>(albumEntries);

    auto model = std::make_shared<LazyModel<AlbumInfo>>((int)entries->size(), [this, entries](int row) {
        const AlbumEntry& entry = (*entries)[row];
        AlbumInfo tile;

        tile.name = toSlint(entry.name);
        tile.artist = toSlint(entry.artist);
        tile.songs = entry.songs;
        tile.art = firstArt(library.songsOfAlbum(entry.name));
        return tile;
        });

    state().set_albums(model);
}


// the Artists page: one tile per artist, made when it scrolls into view
void UiBridge::refreshArtists()
{
    artistEntries = library.artists(artistSearch, artistSort);

    auto entries = std::make_shared<std::vector<ArtistEntry>>(artistEntries);

    auto model = std::make_shared<LazyModel<ArtistInfo>>((int)entries->size(), [this, entries](int row) {
        ArtistInfo tile;

        tile.name = toSlint((*entries)[row].name);
        tile.art = firstArt(library.songsOfArtist((*entries)[row].name));
        return tile;
        });

    state().set_artists(model);
}


// Settings > Removed songs
void UiBridge::refreshRemoved()
{
    std::vector<RemovedInfo> rows;

    for (const RemovedSong& r : tracks.removedSongs()) {
        RemovedInfo info;
        info.title = toSlint(r.title);
        info.artist = toSlint(r.artist);
        rows.push_back(info);
    }

    state().set_removed_songs(std::make_shared<slint::VectorModel<RemovedInfo>>(rows));
}


// the Most Played list of the profile page
void UiBridge::refreshTopSongs()
{
    topSongNumbers = library.mostPlayed(TOP_SONGS);

    std::vector<char> favorites = library.favoriteFlags();
    std::vector<SongInfo> rows;

    for (int n : topSongNumbers)
        rows.push_back(infoOf(n, favorites));

    state().set_top_songs(std::make_shared<slint::VectorModel<SongInfo>>(rows));
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
    if (row == -1) {                                    // the window was opened from a selection
        deleteSelection(page);
        return;
    }

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


// "Add folder": the user picks one, it joins the list and its songs join the library (found on another thread)
void UiBridge::addFolder()
{
    std::string folder = Os::chooseFolder();

    if (folder.empty())
        return;

    std::string why;
    bool added = folders.add(folder, why);

    state().set_folder_error(added ? "" : toSlint(why));
    refreshStorage();

    if (added)
        scanFolder(folder);                             // on another thread: the window stays usable
}


// confirmed in the "Remove folder?" window: its songs leave the library, the files stay
void UiBridge::removeFolder(int row)
{
    std::string why;

    state().set_folder_error(folders.remove(row, why) ? "" : toSlint(why));
    player.libraryChanged();
    reloadLibrary();
}


// ---- selecting several songs ---------------------------------------------------------------------------
// The rules (what a click ticks) are in core/selection. Here the ticks are shown on the rows and an action
// from the selection menu is turned into calls on the player, one for every ticked song.

void UiBridge::connectSelection()
{
    // the select button: boxes on or off. Off also forgets the ticks
    state().on_select_mode_toggled([this](bool on) {
        if (on) {
            selecting = true;
            syncSelection();
        }
        else {
            endSelecting();
        }
        });

    // a click on a row while selecting (or ctrl / shift click at any time)
    state().on_row_selected([this](slint::SharedString list, int row, bool ctrl, bool shift) {
        int page = pageIdOf(list);

        if (page < 0 || row < 0 || row >= (int)shown[page].size())
            return;

        if (selectionPage != page || selection.rows() != (int)shown[page].size()) {
            selection.reset((int)shown[page].size());
            selectionPage = page;
        }

        for (int changed : selection.click(row, ctrl, shift)) {
            if (models[page])
                models[page]->refreshRow(changed);
        }

        selecting = true;
        syncSelection();
        });

    state().on_select_all([this](slint::SharedString list) {
        int page = pageIdOf(list);

        if (page < 0)
            return;

        if (selectionPage != page || selection.rows() != (int)shown[page].size()) {
            selection.reset((int)shown[page].size());
            selectionPage = page;
        }

        selection.selectAll();

        if (models[page])
            models[page]->refreshAll();

        selecting = true;
        syncSelection();
        });

    state().on_selection_action([this](slint::SharedString list, slint::SharedString action) {
        int page = pageIdOf(list);

        if (page >= 0)
            selectionAction((PageId)page, toStd(action));
        });
}


void UiBridge::syncSelection()
{
    state().set_select_mode(selecting);
    state().set_selected_count(selection.count());
}


// the ticks are gone and so are the boxes
void UiBridge::endSelecting()
{
    clearTicks();
    selecting = false;
    syncSelection();
}


// nothing is ticked any more (the boxes stay while selecting)
void UiBridge::clearTicks()
{
    if (selectionPage >= 0 && selectionPage < PAGE_COUNT && models[selectionPage]) {
        for (int row : selection.picked())
            models[selectionPage]->refreshRow(row);
    }

    selection.clear();
    syncSelection();
}


// A page was made again. The ticks stay when it is the same page with the same number of rows (a song was added to the
// queue, say), otherwise the rows are not the ones that were ticked and the ticks go.
void UiBridge::keepSelection(PageId page, int rows)
{
    if (selectionPage == (int)page && selection.rows() == rows)
        return;

    selection.reset(rows);
    selectionPage = page;
    syncSelection();
}


void UiBridge::selectionAction(PageId page, const std::string& action)
{
    if (selectionPage != (int)page || selection.count() == 0)
        return;

    std::vector<int> rows = selection.picked();
    std::vector<int> numbers;                           // song numbers, or recording rows on the Recordings page

    for (int row : rows) {
        if (row >= 0 && row < (int)shown[page].size())
            numbers.push_back(shown[page][row]);
    }

    if (numbers.empty())
        return;

    if (page == RECORDINGS) {
        if (action == "play") {
            player.playFilePath(recordings.path(numbers[0]), recordings.title(numbers[0]));
            syncPlayerBar(false);
        }
        else if (action == "delete") {
            state().set_pending_list(toSlint(listNameOf(page)));
            state().set_pending_row(-1);
            state().set_pending_count((int)numbers.size());
            state().set_pending_title(toSlint(UiConvert::countOf((long long)numbers.size(), "recording")));
            state().set_dialog(DialogKind::DeleteSong);
        }
        return;
    }

    if (action == "play") {
        player.playQueue(numbers, 0, library.queueName(pageOf(page)));
        syncPlayerBar(false);
        refreshQueue();
    }
    else if (action == "queue-add" || action == "queue-remove") {
        for (int n : numbers) {
            if (action == "queue-add")
                player.addToQueue(n);
            else
                player.removeFromQueue(n);
        }

        refreshAll();                                   // the rows know if they are queued
        refreshQueue();
    }
    else if (action == "favorite" || action == "unfavorite") {
        for (int n : numbers) {
            if (player.isFavoriteSong(n) != (action == "favorite"))
                player.toggleFavoriteSong(n);
        }

        refreshAll();
        refreshHeartAndTick();
    }
    else if (action == "add-playlist") {
        pickWindow.openMany(numbers);
        state().set_pending_title(toSlint(UiConvert::countOf((long long)numbers.size(), "song")));
        refreshPickChoices();
        state().set_dialog(DialogKind::PickPlaylist);
    }
    else if (action == "remove-from-playlist") {
        if (openPlaylist < 0 || openPlaylist >= (int)playlistNames.size())
            return;

        std::string name = playlistNames[openPlaylist];
        std::vector<int> inPlaylist;

        for (int n : numbers) {
            int row = player.playlistRowOf(name, n);

            if (row != 0)
                inPlaylist.push_back(row);
        }

        std::sort(inPlaylist.rbegin(), inPlaylist.rend());      // the last row first, so the rows before it stay where they are

        for (int row : inPlaylist)
            player.removeFromPlaylistRow(name, row);

        playlistChanged(name);
    }
    else if (action == "delete") {
        state().set_pending_list(toSlint(listNameOf(page)));
        state().set_pending_row(-1);
        state().set_pending_count((int)numbers.size());
        state().set_pending_title(toSlint(UiConvert::countOf((long long)numbers.size(), "song")));
        state().set_dialog(DialogKind::DeleteSong);
    }
}


// "Remove from library" / "Delete recordings" was confirmed for the ticked rows
void UiBridge::deleteSelection(PageId page)
{
    if (selectionPage != (int)page)
        return;

    std::vector<int> numbers;

    for (int row : selection.picked()) {
        if (row >= 0 && row < (int)shown[page].size())
            numbers.push_back(shown[page][row]);
    }

    // the biggest number first: taking one out moves every song behind it up, the ones in front stay put
    std::sort(numbers.rbegin(), numbers.rend());
    endSelecting();

    if (page == RECORDINGS) {
        for (int row : numbers)
            recordings.remove(row);                     // a recording has no other copy, so the file goes

        refreshRecordings();
        refreshCounts();
        refreshStorage();
        return;
    }

    for (int n : numbers)
        player.removeSongFromLibrary(n);

    reloadLibrary();
}


// ---- looking for songs in the background -------------------------------------------------------------------
// Walking a music folder and reading tags takes a while on a big library. It runs on another thread and the
// result is applied here, on the screen's thread, when it is ready.

void UiBridge::scanFolder(const std::string& folder)
{
    if (folder.empty())
        return;

    if (working) {                                      // one at a time: this one waits its turn
        pendingFolders.push_back(folder);
        return;
    }

    working = true;
    state().set_scanning(true);

    if (worker.joinable())
        worker.join();

    TrackList::Snapshot known = tracks.snapshot();

    worker = std::thread([this, folder, known]() {
        ScanResult result = TrackList::discover(folder, known, &stopping);

        slint::invoke_from_event_loop([this, result = std::move(result)]() {
            finishScan(result);
            });
        });
}


void UiBridge::finishScan(const ScanResult& result)
{
    int changed = tracks.apply(result);

    if (changed > 0) {
        player.libraryChanged();                        // numbers moved: the playing song and the queue find themselves again
        reloadLibrary();
    }
    else {
        refreshStorage();
    }

    startRepair();
}


// New songs have no length yet, and songs from an older index may have names the new rules read better
void UiBridge::startRepair()
{
    std::vector<Song> work = tracks.needsWork();

    if (work.empty()) {
        workDone();
        return;
    }

    if (worker.joinable())
        worker.join();

    TrackList::Snapshot known = tracks.snapshot();

    worker = std::thread([this, work = std::move(work), known]() {
        std::vector<SongFix> fixes = TrackList::repair(work, known, &stopping);

        slint::invoke_from_event_loop([this, fixes = std::move(fixes)]() {
            if (!fixes.empty()) {
                tracks.applyFixes(fixes);
                reloadLibrary();
            }

            workDone();
            });
        });
}


void UiBridge::workDone()
{
    working = false;
    state().set_scanning(false);

    if (!pendingFolders.empty()) {
        std::string next = pendingFolders.front();
        pendingFolders.erase(pendingFolders.begin());
        scanFolder(next);
    }
}
