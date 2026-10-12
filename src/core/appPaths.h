#pragma once
#include <string>

// Knows where Luvista keeps its files: C:\Users\<you>\AppData\Local\Luvista
class AppPaths {
public:
    static std::string root();                                    // the main folder
    static std::string indexFile();                               // the list of all songs
    static std::string musicFoldersFile();                        // the music folders the user added
    static std::string removedFile();                             // songs the user removed: scans must not bring them back
    static std::string profilesFolder();                          // holds one folder per profile
    static std::string profileFolder(const std::string& profile); // favorites, recent, playlists, play counts

    // The profile that is logged in. favorites, recent, playlists and play counts all
    // read and write inside this profile's folder, so changing it switches every one of them.
    // Before anyone logs in it is "guest" (a throw-away profile).
    static std::string activeProfile();
    static void setActiveProfile(const std::string& profile);
    static std::string activeFolder();                            // profileFolder(activeProfile())

    static std::string recordingsFolder();                        // microphone recordings of the logged in profile
    static std::string coversFolder();                            // album pictures cut out of the songs

    // The folder the library scans on the very first start: the user's Music folder ("" if it cannot be found).
    static std::string defaultMusicFolder();
};
