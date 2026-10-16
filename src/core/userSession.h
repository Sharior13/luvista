#pragma once
#include <string>
#include <vector>
#include "profiles.h"

// Who is logged in. It decides which profile folder the rest of the program reads and writes
// (through AppPaths::setActiveProfile) and knows the rules of signing in, creating and editing a profile.
// It does not play music and does not know about the screen.
class UserSession {
public:
    UserSession();

    // --- the login screen
    std::vector<std::string> cards() const;                  // the profiles that get a card
    bool select(const std::string& name);                    // a card was clicked. false if that profile is gone or its card needs the password
    bool isLocked(const std::string& name) const;            // the card asks for a password (signIn does the logging in)
    bool signIn(const std::string& userOrEmail, const std::string& password, std::string& error);
    void startGuest();                                       // Guest Mode: an empty, throw-away profile

    // --- the "create profile" steps. create() makes the profile, finishCreate() logs it in.
    bool create(const std::string& name, const std::string& email, const std::string& password,
                bool remember, std::string& error);
    bool setPendingPicture(const std::string& imagePath);    // the picture chosen on the last step
    std::string pendingPicture() const;
    bool finishCreate();

    void signOut();                                          // back to the throw-away guest folder

    // --- the logged in profile
    bool isLoggedIn() const;
    bool isGuest() const;
    const std::string& name() const;                         // "Guest" for guest mode
    std::string initial() const;                             // the letter in the round avatar
    std::string email() const;
    std::string picture() const;                             // the picture file, "" if none
    bool setPicture(const std::string& imagePath);           // false for guests and for files that are not png/jpg
    bool updateAccount(const std::string& newName, const std::string& email,
                       const std::string& currentPassword, const std::string& newPassword, std::string& error);

    // --- for the screen
    std::string pictureOf(const std::string& profile) const;
    std::string initialOf(const std::string& profile) const;

private:
    void enter(const std::string& profile);

    Profiles profiles;
    std::string active;                // the real name of the logged in profile, "" = nobody
    bool guest = false;
    std::string pending;               // created but not finished yet
};
