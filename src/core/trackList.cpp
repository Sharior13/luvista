#include "trackList.h"
#include "appPaths.h"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <system_error>

namespace fs = std::filesystem;


// Says yes if a file is a music file.
// It only looks at the ending of the name, like ".mp3".
static bool isMusic(const fs::path& p)
{
    std::string ending = p.extension().string();

    // make all letters small, so ".MP3" and ".mp3" are the same
    for (char& c : ending)
        c = (char)std::tolower((unsigned char)c);

    return ending == ".mp3" || ending == ".wav" || ending == ".m4a" ||
        ending == ".aac" || ending == ".aif" || ending == ".aiff";
}


// A line looks like:  5 <tab> C:/music/Adele - Hello.mp3 <tab> Hello <tab> ...
// This gives back one piece of the line. Piece 0 is the first, piece 1 is the second...
static std::string piece(const std::string& line, int number)
{
    int tabsSeen = 0;
    std::string result = "";

    for (char c : line) {
        if (c == '\t') {
            tabsSeen++;
        }
        else if (tabsSeen == number) {
            result += c;
        }
    }

    return result;
}


// Opens the notebook and gives back line number n. "" if there is no such line.
static std::string lineNumber(int n)
{
    std::ifstream notebook(AppPaths::indexFile());
    std::string line;
    int number = 0;

    while (std::getline(notebook, line)) {
        number++;

        if (number == n)
            return line;
    }

    return "";
}


// A file named "Adele - Hello" is cut at " - ": artist = "Adele", title = "Hello".
// If there is no " - " in the name, the artist is "Unknown artist" and the title is the whole name.
static void cutName(const std::string& fileName, std::string& artist, std::string& title)
{
    size_t place = fileName.find(" - ");

    if (place != std::string::npos && place > 0 && place + 3 < fileName.size()) {
        artist = fileName.substr(0, place);
        title = fileName.substr(place + 3);
    }
    else {
        artist = "Unknown artist";
        title = fileName;
    }
}


// What we can read from the tags inside a music file.
struct Tags {
    std::string title;
    std::string artist;
    std::string album;
};

// ID3v2 layout (what mp3 files use):
//   10 byte header:  "ID3", version, revision, flags, size (4 bytes, 7 bits each)
//   then frames:     id, size, flags, data.   Title = TIT2, Artist = TPE1, Album = TALB.
// Version 2.2 uses 3-letter ids (TT2, TP1, TAL) and a 3 byte size.


// Size stored with only 7 bits in each byte ("synchsafe").
static unsigned int syncsafe(const unsigned char* b)
{
    return ((unsigned int)(b[0] & 0x7F) << 21) |
        ((unsigned int)(b[1] & 0x7F) << 14) |
        ((unsigned int)(b[2] & 0x7F) << 7) |
        (unsigned int)(b[3] & 0x7F);
}


// Normal big-endian number of 3 or 4 bytes.
static unsigned int bigEndian(const unsigned char* b, int bytes)
{
    unsigned int n = 0;

    for (int i = 0; i < bytes; i++)
        n = (n << 8) | b[i];

    return n;
}


// Adds one character (a Unicode number) to a UTF-8 string.
static void addChar(std::string& out, unsigned int c)
{
    if (c < 0x80) {
        out += (char)c;
    }
    else if (c < 0x800) {
        out += (char)(0xC0 | (c >> 6));
        out += (char)(0x80 | (c & 0x3F));
    }
    else if (c < 0x10000) {
        out += (char)(0xE0 | (c >> 12));
        out += (char)(0x80 | ((c >> 6) & 0x3F));
        out += (char)(0x80 | (c & 0x3F));
    }
    else {
        out += (char)(0xF0 | (c >> 18));
        out += (char)(0x80 | ((c >> 12) & 0x3F));
        out += (char)(0x80 | ((c >> 6) & 0x3F));
        out += (char)(0x80 | (c & 0x3F));
    }
}


// Turns the text of a frame into UTF-8. The first byte says how the text is written:
// 0 = ISO-8859-1, 1 = UTF-16 with a mark, 2 = UTF-16 big-endian, 3 = UTF-8.
// Only the first value is kept (a tag can hold several, split by a zero).
static std::string decodeText(const unsigned char* data, unsigned int size)
{
    if (size < 2)
        return "";

    int encoding = data[0];
    data++;
    size--;

    std::string out;

    if (encoding == 0 || encoding == 3) {
        for (unsigned int i = 0; i < size; i++) {
            if (data[i] == 0)
                break;

            if (encoding == 3)
                out += (char)data[i];
            else
                addChar(out, data[i]);          // Latin-1 letter -> UTF-8
        }
    }
    else if (encoding == 1 || encoding == 2) {
        bool bigEnd = (encoding == 2);
        unsigned int i = 0;

        if (encoding == 1 && size >= 2) {       // the mark tells which end comes first
            if (data[0] == 0xFF && data[1] == 0xFE) { bigEnd = false; i = 2; }
            else if (data[0] == 0xFE && data[1] == 0xFF) { bigEnd = true; i = 2; }
        }

        for (; i + 1 < size; i += 2) {
            unsigned int unit = bigEnd ? (data[i] << 8) | data[i + 1]
                : (data[i + 1] << 8) | data[i];

            if (unit == 0)
                break;

            // two units together make one big character (surrogate pair)
            if (unit >= 0xD800 && unit < 0xDC00 && i + 3 < size) {
                unsigned int next = bigEnd ? (data[i + 2] << 8) | data[i + 3]
                    : (data[i + 3] << 8) | data[i + 2];

                if (next >= 0xDC00 && next < 0xE000) {
                    unit = 0x10000 + ((unit - 0xD800) << 10) + (next - 0xDC00);
                    i += 2;
                }
            }

            addChar(out, unit);
        }
    }

    // the song list uses tabs and new lines to split things, so none may stay inside the text
    for (char& c : out) {
        if (c == '\t' || c == '\n' || c == '\r')
            c = ' ';
    }

    return out;
}


static bool readTags(const std::string& path, Tags& out)
{
    std::ifstream file(path, std::ios::binary);

    if (!file)
        return false;

    unsigned char header[10];
    file.read((char*)header, 10);

    if (file.gcount() != 10 || header[0] != 'I' || header[1] != 'D' || header[2] != '3')
        return false;                       // no ID3v2 tag at the start

    int version = header[3];                // 2, 3 or 4

    if (version < 2 || version > 4)
        return false;

    unsigned int tagSize = syncsafe(header + 6);

    if (tagSize == 0 || tagSize > 16 * 1024 * 1024)
        return false;                       // empty, or too big to be sane

    std::string tag(tagSize, '\0');
    file.read(&tag[0], tagSize);
    tag.resize((size_t)file.gcount());

    const unsigned char* t = (const unsigned char*)tag.data();
    unsigned int end = (unsigned int)tag.size();
    unsigned int pos = 0;

    // skip the extended header if there is one
    if (header[5] & 0x40) {
        if (version == 4 && end >= 4)
            pos = syncsafe(t);
        else if (version == 3 && end >= 4)
            pos = bigEndian(t, 4) + 4;
    }

    int idLength = (version == 2) ? 3 : 4;
    int headLength = (version == 2) ? 6 : 10;

    while (pos + headLength <= end) {
        if (t[pos] == 0)
            break;                          // padding: no more frames

        std::string id((const char*)t + pos, idLength);

        unsigned int size;

        if (version == 2)
            size = bigEndian(t + pos + 3, 3);
        else if (version == 3)
            size = bigEndian(t + pos + 4, 4);
        else
            size = syncsafe(t + pos + 4);

        unsigned int dataStart = pos + headLength;

        if (size > end - dataStart)
            break;                          // frame says it is bigger than the tag: stop

        if (id == "TIT2" || id == "TT2")
            out.title = decodeText(t + dataStart, size);
        else if (id == "TPE1" || id == "TP1")
            out.artist = decodeText(t + dataStart, size);
        else if (id == "TALB" || id == "TAL")
            out.album = decodeText(t + dataStart, size);

        pos = dataStart + size;
    }

    return true;
}


// The time right now, in seconds since 1970.
static long long nowSeconds()
{
    return (long long)std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}


// Looks inside the file for its real title, artist and album.
// Whatever the file does not have stays as it was. A missing album becomes "Unknown album".
static void fillFromTags(const std::string& path,
    std::string& title, std::string& artist, std::string& album)
{
    Tags tags;

    if (readTags(path, tags)) {
        if (tags.title != "")
            title = tags.title;

        if (tags.artist != "")
            artist = tags.artist;

        album = tags.album;
    }
    else {
        album = "";
    }

    if (album == "")
        album = "Unknown album";
}


// Starts with zero songs, then counts what is already in the notebook from last time.
TrackList::TrackList()
{
    folder = "";
    count = 0;
    nextId = 1;
    countSongs();
}


// Counts the songs in the notebook. Also remembers the biggest id, so a new id is never reused.
void TrackList::countSongs()
{
    std::ifstream notebook(AppPaths::indexFile());
    std::string line;

    count = 0;

    while (std::getline(notebook, line)) {
        if (line == "")
            continue;

        count++;

        int id = std::atoi(piece(line, 0).c_str());

        if (id >= nextId)
            nextId = id + 1;
    }
}


// Looks for a path in the notebook. Gives the line number, or 0 if it is not there.
int TrackList::find(const std::string& path) const
{
    std::ifstream notebook(AppPaths::indexFile());
    std::string line;
    int number = 0;

    while (std::getline(notebook, line)) {
        number++;

        if (piece(line, 1) == path)
            return number;
    }

    return 0;
}


// Old songs keep their id. New songs get a new id.
bool TrackList::scan(const std::string& folderPath)
{
    folder = folderPath;

    std::error_code problem;

    if (!fs::is_directory(folder, problem))
        return false;          // the folder is not there

    // Step 1: keep only the songs whose file is still there
    std::string kept = "";

    {
        std::ifstream notebook(AppPaths::indexFile());
        std::string line;

        while (std::getline(notebook, line)) {
            std::error_code e;
            std::string path = piece(line, 1);

            if (line == "" || !fs::exists(path, e))
                continue;

            std::string title = piece(line, 2);
            std::string artist = piece(line, 3);
            std::string album = piece(line, 4);
            std::string added = piece(line, 5);

            // an old line ("-" album) has not been read yet: fill it now, the id stays the same
            if (album == "-" || album == "")
                fillFromTags(path, title, artist, album);

            if (added == "")
                added = std::to_string(nowSeconds());

            kept += piece(line, 0) + "\t" + path + "\t" + title + "\t" +
                artist + "\t" + album + "\t" + added + "\n";
        }
    }   // the notebook is closed here, so we can write to it now

    {
        std::ofstream notebook(AppPaths::indexFile());   // this erases the old notebook
        notebook << kept;
    }

    // Step 2: walk through the folder (and the folders inside it)
    fs::recursive_directory_iterator it(
        folder, fs::directory_options::skip_permission_denied, problem);

    for (; !problem && it != fs::recursive_directory_iterator(); it.increment(problem)) {

        if (it->is_regular_file(problem) && isMusic(it->path())) {
            std::string path = it->path().string();

            if (find(path) == 0) {                        // a new song
                std::string artist, title, album;
                cutName(it->path().stem().string(), artist, title);   // the file name is the fallback
                fillFromTags(path, title, artist, album);             // the tags in the file win

                std::ofstream notebook(AppPaths::indexFile(), std::ios::app);   // add at the end
                notebook << nextId << '\t' << path << '\t' << title << '\t'
                    << artist << '\t' << album << '\t' << nowSeconds() << '\n';

                nextId++;
            }
        }
    }

    countSongs();
    return true;
}


int TrackList::getCount() const
{
    return count;
}


std::string TrackList::getTrack(int n) const
{
    return piece(lineNumber(n), 1);
}


std::string TrackList::getTitle(int n) const
{
    return piece(lineNumber(n), 2);
}


std::string TrackList::getArtist(int n) const
{
    return piece(lineNumber(n), 3);
}


std::string TrackList::getAlbum(int n) const
{
    return piece(lineNumber(n), 4);
}


// When the song was added to the library (seconds since 1970). 0 if there is no such song.
long long TrackList::getAdded(int n) const
{
    return std::atoll(piece(lineNumber(n), 5).c_str());
}


int TrackList::getId(int n) const
{
    return std::atoi(piece(lineNumber(n), 0).c_str());
}


// Reads down the notebook until it finds the id. Gives the line number, or 0.
int TrackList::positionOfId(int id) const
{
    std::ifstream notebook(AppPaths::indexFile());
    std::string line;
    int number = 0;

    while (std::getline(notebook, line)) {
        number++;

        if (std::atoi(piece(line, 0).c_str()) == id)
            return number;
    }

    return 0;
}


// Makes every letter small, so "ADELE" and "adele" are the same.
static std::string lowerCase(std::string text)
{
    for (char& c : text)
        c = (char)std::tolower((unsigned char)c);

    return text;
}


bool TrackList::matches(int n, const std::string& query) const
{
    if (n < 1 || n > count)
        return false;

    if (query == "")
        return true;

    std::string wanted = lowerCase(query);

    return lowerCase(getTitle(n)).find(wanted) != std::string::npos ||
        lowerCase(getArtist(n)).find(wanted) != std::string::npos ||
        lowerCase(getAlbum(n)).find(wanted) != std::string::npos;
}


std::string TrackList::getFolder() const
{
    return folder;
}