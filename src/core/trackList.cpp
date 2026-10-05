#include "trackList.h"
#include <fstream>
#include <filesystem>
#include <cctype>
#include <system_error>

using namespace std;
namespace fs = std::filesystem;

// The notebook where we write one song address per line
static const char* INDEX_FILE = "Song-index.dat";


// Says yes if a file is a music file.
// It only looks at the ending of the name, like ".mp3".
static bool isMusic(const fs::path& p)
{
    string ending = p.extension().string();

    // make all letters small, so ".MP3" and ".mp3" are the same
    for (char& c : ending)
        c = tolower((unsigned char)c);

    return ending == ".mp3" || ending == ".wav" || ending == ".m4a" ||
           ending == ".aac" || ending == ".aif" || ending == ".aiff";
}


// Starts with an empty folder name and zero songs.
TrackList::TrackList()
{
    folder = "";
    count = 0;
}


// Walks through the folder (and the folders inside it).
// For every music file: count it, and write its address in Song-index.dat.
bool TrackList::scan(const string& folderPath)
{
    count = 0;
    folder = folderPath;

    error_code problem;   // collects errors instead of crashing

    if (!fs::is_directory(folder, problem))
        return false;     // the folder is not there

    ofstream notebook(INDEX_FILE);   // starts a fresh, empty notebook

    fs::recursive_directory_iterator it(
        folder, fs::directory_options::skip_permission_denied, problem);

    for (; !problem && it != fs::recursive_directory_iterator(); it.increment(problem)) {

        if (it->is_regular_file(problem) && isMusic(it->path())) {
            count++;
            notebook << it->path().string() << "\n";
        }
    }

    return true;
}


// Says how many songs we found.
int TrackList::getCount() const
{
    return count;
}


// Opens the notebook and reads down to line number n.
// That line is the song address. If there is no such line, gives back "".
string TrackList::getTrack(int n) const
{
    ifstream notebook(INDEX_FILE);
    string line;
    int lineNumber = 0;

    while (getline(notebook, line)) {
        lineNumber++;

        if (lineNumber == n)
            return line;
    }

    return "";
}
