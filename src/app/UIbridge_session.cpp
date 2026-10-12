#include "UIbridge.h"
#include "uiConvert.h"
#include "../core/osServices.h"

using UiConvert::toSlint;
using UiConvert::toStd;

// the login screen: pick a card, sign in, make a profile, guest mode, log out
void UiBridge::connectSession()
{
    sessionState().on_select_profile([this](int card) {
        if (card >= 0 && card < (int)profileCards.size() && session.select(profileCards[card]))
            enterProfile();
        });

    sessionState().on_login([this](slint::SharedString user, slint::SharedString password) {
        std::string why;

        if (session.signIn(toStd(user), toStd(password), why)) {
            enterProfile();
        }
        else {
            sessionState().set_error(toSlint(why));
        }
        });

    // step 1 of a new profile: the rules (name, email, password) belong to the profiles code, the screen shows its answer
    sessionState().on_create_profile([this](slint::SharedString name, slint::SharedString email, slint::SharedString password) {
        std::string why;

        if (!session.create(toStd(name), toStd(email), toStd(password), sessionState().get_remember(), why)) {
            sessionState().set_error(toSlint(why));
            return false;
        }

        sessionState().set_error("");
        sessionState().set_new_picture(slint::Image());
        return true;
        });

    // step 2: a picture for the new profile
    sessionState().on_pick_picture([this] {
        std::string file = Os::chooseImage();

        if (file.empty() || !session.setPendingPicture(file))
            return;

        images.forget(session.pendingPicture());
        sessionState().set_new_picture(images.file(session.pendingPicture()));
        });

    sessionState().on_finish_profile([this] {
        if (session.finishCreate())
            enterProfile();
        });

    sessionState().on_continue_as_guest([this] {
        session.startGuest();
        enterProfile();
        });

    sessionState().on_logout([this] { leaveProfile(); });
}


// the cards of the login screen
void UiBridge::refreshCards()
{
    profileCards = session.cards();
    std::vector<ProfileInfo> cards;

    for (const std::string& name : profileCards) {
        ProfileInfo card;
        card.name = toSlint(name);
        card.picture = images.file(session.pictureOf(name));    // empty: the screen draws a gradient with the icon
        cards.push_back(card);
    }

    sessionState().set_profiles(std::make_shared<slint::VectorModel<ProfileInfo>>(cards));
}


// the name, letter, picture and email of whoever is logged in
void UiBridge::showProfile()
{
    state().set_profile_name(toSlint(session.isLoggedIn() ? session.name() : "Guest"));
    state().set_profile_initial(toSlint(session.isLoggedIn() ? session.initial() : "G"));
    state().set_profile_picture(images.file(session.picture()));
    state().set_account_email(toSlint(session.email()));

    std::vector<slint::SharedString> languages, themes;

    for (const std::string& language : Settings::languages()) languages.push_back(toSlint(language));
    for (const std::string& theme : Settings::themes()) themes.push_back(toSlint(theme));

    state().set_languages(std::make_shared<slint::VectorModel<slint::SharedString>>(languages));
    state().set_themes(std::make_shared<slint::VectorModel<slint::SharedString>>(themes));
    state().set_language_index(settings.language());
    state().set_theme_index(settings.theme());
    state().set_autoplay(settings.autoplay());
}


// somebody logged in: from now on favorites, playlists, recent songs, play counts, recordings and settings are theirs
void UiBridge::enterProfile()
{
    settings.load();
    player.setAutoplay(settings.autoplay());
    recordings.refresh();

    openPlaylist = -1;
    library.resetPage(Page::AllSongs);
    library.resetPage(Page::Favorites);
    library.resetPage(Page::Playlist);
    library.resetPage(Page::Artist);
    recordingSearch = "";
    recordingSort = 0;

    showProfile();
    state().set_search_text("");
    state().set_selected_playlist(-1);
    state().set_section(NavSection::Home);

    refreshPlaylists();
    refreshAll();
    syncPlayerBar(true);                         // also fills the home page, the queue and the heart

    sessionState().set_error("");
    sessionState().set_logged_in(true);
}


// back to the login screen: the music stops, a recording in progress is saved
void UiBridge::leaveProfile()
{
    if (recorder.isRecording())
        recorder.stop();                         // this still belongs to the profile that is leaving

    state().set_record_state(RecordState::Idle);
    state().set_record_seconds(0);
    state().set_record_level(0);
    state().set_dialog(DialogKind::None);

    player.stop();
    player.setShuffle(false);
    player.setRepeatMode(0);
    session.signOut();

    state().set_playing(false);
    state().set_position(0);
    state().set_duration(0);
    syncPlayerBar(true);

    refreshCards();
    sessionState().set_error("");
    sessionState().set_logged_in(false);
}


// the Profile page and the edit-account window
void UiBridge::connectAccount()
{
    state().on_change_profile_picture([this] {
        std::string file = Os::chooseImage();

        if (file.empty())
            return;

        images.forget(session.picture());

        if (session.setPicture(file)) {
            images.forget(session.picture());
            showProfile();
        }
        });

    state().on_account_saved([this](slint::SharedString user, slint::SharedString email,
                                    slint::SharedString current, slint::SharedString fresh) {
        std::string why;

        if (!session.updateAccount(toStd(user), toStd(email), toStd(current), toStd(fresh), why)) {
            state().set_dialog_error(toSlint(why));
            return false;
        }

        state().set_dialog_error("");
        showProfile();
        return true;
        });
}


// the Settings page. The choices are saved by Settings, the player is told about the ones it must follow
void UiBridge::connectSettings()
{
    state().on_autoplay_toggled([this](bool on) {
        settings.setAutoplay(on);
        player.setAutoplay(on);
        });

    state().on_language_changed([this](int index) { settings.setLanguage(index); showProfile(); });
    state().on_theme_changed([this](int index) { settings.setTheme(index); showProfile(); });
}
