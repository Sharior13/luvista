#pragma once
#include <string>
#include <vector>

// The choices on the Settings page. They belong to the profile that is logged in
// (the file settings.dat inside its folder), so load() again after every login.
class Settings {
public:
    void load();                                   // reads the logged in profile's file (defaults if there is none)

    bool autoplay() const;
    void setAutoplay(bool on);                     // every set... saves right away

    int language() const;                          // index into languages()
    void setLanguage(int index);
    int theme() const;                             // index into themes()
    void setTheme(int index);

    static std::vector<std::string> languages();   // what the pickers offer
    static std::vector<std::string> themes();

private:
    void save() const;

    bool autoplayOn = true;
    int languageIndex = 0;
    int themeIndex = 0;
};
