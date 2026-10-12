#include "UIbridge.h"
#include "uiConvert.h"
#include "../core/osServices.h"

using UiConvert::toSlint;
using UiConvert::toStd;

// the playlists of the sidebar, the "+" popup of the player bar and the "Add to playlist" window
void UiBridge::connectPlaylists()
{
    state().on_playlist_clicked([this](int i) {
        openPlaylist = i;

        if (i >= 0 && i < (int)playlistNames.size())
            library.setPlaylist(playlistNames[i]);

        resetPage(PLAYLIST);
        });

    // the screen asked for a name already. with: "" = an empty playlist, "current" = start with the
    // playing song, "song" = start with the song of the "Add to playlist" window
    state().on_create_playlist([this](slint::SharedString name, slint::SharedString with) {
        return createPlaylistNamed(toStd(name), toStd(with));
        });

    state().on_rename_playlist([this](int index, slint::SharedString name) {
        if (index < 0 || index >= (int)playlistNames.size())
            return false;

        if (!player.renamePlaylist(playlistNames[index], toStd(name))) {
            state().set_dialog_error("That name is empty, has a letter that is not allowed, or is already used");
            return false;
        }

        refreshPlaylists();
        refreshPopupChoices();
        refreshPickChoices();

        if (index == openPlaylist)
            library.setPlaylist(playlistNames[index]);

        return true;
        });

    state().on_delete_playlist([this](int index) {
        if (index < 0 || index >= (int)playlistNames.size())
            return;

        player.deletePlaylist(playlistNames[index]);
        openPlaylist = -1;
        refreshPlaylists();
        refreshPopupChoices();
        refreshPickChoices();
        refreshHeartAndTick();
        });

    state().on_edit_playlist_art([this](int index) { editPlaylistArt(index); });

    // the "+" of the player bar
    state().on_add_current_track([this] {
        popup.open(player.currentIndex());
        refreshPopupChoices();
        });

    state().on_playlist_toggled([this](int row) {
        std::string name = popup.toggle(row);

        if (!name.empty())
            playlistChanged(name);
        });

    state().on_playlist_picker_search([this](slint::SharedString text) {
        popup.setSearch(toStd(text));
        refreshPopupChoices();
        });
}


// the sidebar list
void UiBridge::refreshPlaylists()
{
    playlistNames.clear();
    std::vector<PlaylistInfo> infos;

    for (int i = 1; i <= player.playlistCount(); i++) {
        std::string name = player.playlistName(i);
        PlaylistInfo info;

        info.title = toSlint(name);
        info.art = images.playlist(name);               // empty: the screen draws a gradient
        playlistNames.push_back(name);
        infos.push_back(info);
    }

    state().set_playlists(std::make_shared<slint::VectorModel<PlaylistInfo>>(infos));

    if (openPlaylist >= (int)playlistNames.size())
        openPlaylist = -1;
}


static std::shared_ptr<slint::VectorModel<PlaylistChoice>> choicesOf(const std::vector<PlaylistRow>& rows, ImageCache& images)
{
    std::vector<PlaylistChoice> choices;

    for (const PlaylistRow& row : rows) {
        PlaylistChoice choice;
        choice.title = toSlint(row.name);
        choice.art = images.playlist(row.name);
        choice.count = row.count;
        choice.saved = row.saved;
        choice.checked = row.checked;
        choices.push_back(choice);
    }

    return std::make_shared<slint::VectorModel<PlaylistChoice>>(choices);
}


void UiBridge::refreshPopupChoices()
{
    state().set_playlist_choices(choicesOf(popup.rows(), images));
}


void UiBridge::refreshPickChoices()
{
    state().set_song_playlist_choices(choicesOf(pickWindow.rows(), images));
}


// a playlist got or lost a song: everything that shows it is made again
void UiBridge::playlistChanged(const std::string& name)
{
    refreshPlaylists();
    refreshPopupChoices();
    refreshPickChoices();
    refreshHeartAndTick();

    if (openPlaylist >= 0 && openPlaylist < (int)playlistNames.size() && playlistNames[openPlaylist] == name)
        refreshPage(PLAYLIST);
}


bool UiBridge::createPlaylistNamed(const std::string& typed, const std::string& with)
{
    std::string name = typed.empty() ? player.freePlaylistName("New playlist") : typed;

    if (!player.createPlaylist(name)) {
        state().set_dialog_error("That name is empty, has a letter that is not allowed, or is already used");
        return false;
    }

    // the library trims the name, so ask for the name it really holds
    std::string made = player.playlistName(player.playlistCount());

    if (with == "current" && player.currentIndex() != 0) {
        player.addToPlaylist(made, player.currentIndex());
        popup.markSaved(made);
    }
    else if (with == "song" && pickWindow.song() != 0) {
        player.addToPlaylist(made, pickWindow.song());
        pickWindow.markSaved(made);
    }

    state().set_dialog_error("");
    playlistChanged(made);
    return true;
}


// click on the open playlist's picture: pick an image and use it
void UiBridge::editPlaylistArt(int index)
{
    if (index < 0 || index >= (int)playlistNames.size())
        return;

    std::string name = playlistNames[index];
    std::string file = Os::chooseImage();

    if (file.empty())
        return;

    images.forget(player.playlistArt(name));            // forget the old picture, a new one is about to take its place

    if (!player.setPlaylistArt(name, file))
        return;

    images.forget(player.playlistArt(name));
    playlistChanged(name);
}
