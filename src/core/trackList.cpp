#include "trackList.h"
#include <fstream>
#include <filesystem>
#include <cctype>
#include <system_error>

using namespace std;
namespace fs = std::filesystem;

// The file where the song addresses are written, one per line
static const char* INDEX_FILE = "Song-index.dat";


// Checks if a file is a music file by looking at the end of its name.
static bool isMusic(const fs::path& p)
{
    string ext = p.extension().string();

    // make letters small, so .MP3 and .mp3 are the same
    for (char& c : ext)
        c = tolower((unsigned char)c);

    return ext == ".mp3" || ext == ".wav" || ext == ".m4a" ||
           ext == ".aac" || ext == ".aif" || ext == ".aiff";
}


TrackList::TrackList()
{
    folder = "";
    count = 0;
}

// Opens the folder and looks at every file inside (and inside sub-folders).
// Each song found is counted and its address is written in Song-index.dat.
bool TrackList::scan(const string& folderPath)
{
    count = 0;
    folder = folderPath;

    error_code ec;

    if (!fs::is_directory(folder, ec))
        return false;

    ofstream out(INDEX_FILE);

    fs::recursive_directory_iterator it(
        folder, fs::directory_options::skip_permission_denied, ec);

    for (; !ec && it != fs::recursive_directory_iterator(); it.increment(ec)) {

        if (it->is_regular_file(ec) && isMusic(it->path())) {
            count++;
            out << it->path().string() << "\n";
        }
    }

    return true;
}

int TrackList::getCount() const
{
    return count;
}

// Opens the song list and reads down to line n.
// Gives back "" if there is no such song.
string TrackList::getTrack(int n) const
{
    ifstream in(INDEX_FILE);
    string line;
    int i = 0;

    while (getline(in, line)) {
        i++;

        if (i == n)
            return line;
    }

    return "";
}
