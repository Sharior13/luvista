#pragma once
#include <string>

// What the app does when it starts, kept for the whole app (not per profile):
// open the profile that was used last, and which one that was.
class LaunchSettings {
public:
    LaunchSettings();

    bool openLast() const;                      // "Open the last profile on launch" (on by default)
    void setOpenLast(bool on);

    bool hasLast() const;                       // a profile (or guest mode) was in use when the app was closed
    bool lastWasGuest() const;
    std::string lastProfile() const;            // its name, "" for guest mode

    void remember(const std::string& profile, bool guest);   // a profile was opened
    void forget();                              // logged out: the next start shows the login screen

private:
    bool open = true;
    bool any = false;
    bool guest = false;
    std::string last;

    void load();
    void save() const;
};
