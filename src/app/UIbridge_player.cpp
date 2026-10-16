#include "UIbridge.h"
#include "uiConvert.h"

using UiConvert::toSlint;

// play, next, previous, seek, volume, the heart, shuffle and repeat
void UiBridge::connectPlayerBar()
{
    state().on_toggle_play([this] { togglePlay(); });
    state().on_next([this] { player.next(); syncPlayerBar(false); });
    state().on_previous([this] { player.previous(); syncPlayerBar(false); });
    state().on_seek([this](float seconds) { player.seekTo(seconds); syncPlayerBar(false); });
    state().on_volume_changed([this](float v) { player.setVolume((int)(v * 100 + 0.5f)); });
    state().on_favorite_toggled([this](bool on) { setFavorite(on); });

    // the buttons only ask: the player flips its own mode and the screen shows what the player says
    state().on_shuffle_clicked([this] {
        player.toggleShuffle();
        syncPlayerBar(false);
        refreshQueue();
        });

    state().on_repeat_clicked([this] {
        player.toggleRepeat();
        syncPlayerBar(false);
        refreshQueue();
        });

    // row 0 of the queue panel is the playing song, the rows after it are what plays next
    state().on_queue_track_clicked([this](int row) {
        if (row >= 1 && row < (int)queueNumbers.size()) {
            player.playUpcoming(queueNumbers[row]);
            syncPlayerBar(false);
        }
        });

    state().on_recent_track_clicked([this](int row) {
        if (row >= 0 && row < (int)recentNumbers.size()) {
            player.playSong(recentNumbers[row]);
            syncPlayerBar(false);
        }
        });
}


// the play button already flipped AppState.playing, so it tells us what the user wants now
void UiBridge::togglePlay()
{
    bool wantPlay = state().get_playing();

    if (!player.hasSong()) {
        if (wantPlay) {
            if (shown[ALL_SONGS].empty())
                refreshPage(ALL_SONGS);                  // the Songs page was never opened: make its list now

            if (!shown[ALL_SONGS].empty())
                player.playQueue(shown[ALL_SONGS], 0, library.queueName(Page::AllSongs));
            else
                player.next();                   // no rows to start from: the player starts the library
        }
    }
    else if (wantPlay) {
        player.resume();
    }
    else {
        player.pause();
    }

    syncPlayerBar(false);
}


// the heart was clicked. if nothing is playing, the heart is put back
void UiBridge::setFavorite(bool on)
{
    if (player.currentIndex() != 0 && player.isFavorite() != on)
        player.toggleFavorite();

    refreshAll();                                        // the page that is open and the sidebar numbers
    refreshHeartAndTick();
}


// the heart and the tick that replaces the "+" when the playing song is in a playlist
void UiBridge::refreshHeartAndTick()
{
    int cur = player.currentIndex();

    state().set_current_favorite(cur != 0 && player.isFavorite());
    state().set_track_saved_in(cur == 0 ? 0 : (int)player.playlistsWithSong(cur).size());
}


// copies what the player knows into the player bar
void UiBridge::syncPlayerBar(bool force)
{
    int cur = player.currentIndex();
    std::string name = player.hasSong() ? player.songName() : "";     // a recording has no library number, only a name

    if (force || cur != lastCurrent || name != lastName) {
        lastCurrent = cur;
        lastName = name;

        if (cur != 0) {
            state().set_title(toSlint(player.songTitle(cur)));
            state().set_artist(toSlint(player.songArtist(cur)));
            state().set_album(toSlint(player.songAlbum(cur)));
        }
        else {
            state().set_title(toSlint(name));
            state().set_artist(name.empty() ? "" : "Voice memo");
            state().set_album("");
        }

        state().set_cover_art(cur == 0 ? slint::Image() : images.cover(cur));
        refreshLyrics(cur);

        // a new song started, so everything that shows "what is playing" or "what was played" changes
        refreshHome();
        refreshQueue();
        refreshHeartAndTick();
        popup.open(cur);
        refreshPopupChoices();
    }

    float position = (float)player.position();

    state().set_position(position);
    state().set_duration((float)player.duration());
    state().set_playing(player.hasSong() && !player.isPaused());
    state().set_shuffle(player.isShuffle());
    state().set_repeat_mode(player.repeatMode());

    // move the highlight to the line being sung
    int line = lyrics.count() > 0 ? lyrics.indexAt(position) : -1;

    if (line != lastLyric) {
        lastLyric = line;
        state().set_lyric_index(line);
    }
}


// the playing song, then what plays next. queueNumbers[row] is the song of a row, for clicks
void UiBridge::refreshQueue()
{
    int cur = player.currentIndex();
    std::vector<char> favorites = library.favoriteFlags();
    std::vector<SongInfo> queue;
    queueNumbers.clear();

    if (cur != 0) {
        queueNumbers.push_back(cur);

        for (int n : player.upcoming(QUEUE_ROWS))
            queueNumbers.push_back(n);

        for (int n : queueNumbers)
            queue.push_back(infoOf(n, favorites));
    }

    state().set_queue_tracks(std::make_shared<slint::VectorModel<SongInfo>>(queue));
    state().set_queue_index(0);
    state().set_queue_context(cur == 0 ? slint::SharedString() : toSlint(player.queueContext()));

    recentNumbers = library.recentlyPlayed();
    std::vector<SongInfo> recent;

    for (int n : recentNumbers)
        recent.push_back(infoOf(n, favorites));

    state().set_recently_played(std::make_shared<slint::VectorModel<SongInfo>>(recent));
}


// a new song started: load its lyrics (the .lrc file next to it)
void UiBridge::refreshLyrics(int number)
{
    std::vector<LyricLine> rows;

    if (library.valid(number) && lyrics.load(library.path(number))) {
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
