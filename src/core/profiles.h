#pragma once
#include <string>
#include <vector>

// The people who can log in. Each profile is a folder inside Luvista/profiles
// (its favorites, recent songs, playlists and play counts live in there).
// "guest" is a throw-away profile: it is emptied every time guest mode starts, and never listed.
class Profiles {
public:
    static const char* GUEST;                       // "guest"

    std::vector<std::string> list() const;          // every profile name, A to Z. Guest is left out
    bool exists(const std::string& name) const;     // big/small letters do not matter
    std::string find(const std::string& name) const;   // the real spelling of a name, "" if there is no such profile
    bool create(const std::string& name);           // false if the name is empty, bad or already used
    std::string nextName() const;                   // "Profile 1", "Profile 2" ... the first free one

    // The profile picture is the file "picture.png" / ".jpg" inside the profile's folder.
    std::string picture(const std::string& name) const;                         // "" if there is none
    bool setPicture(const std::string& name, const std::string& imagePath);     // copies the image in

    void resetGuest();                              // wipes everything the guest left behind

    // The first letter of a name, in capitals, for the round avatar ("alex" -> "A")
    static std::string initial(const std::string& name);
};
