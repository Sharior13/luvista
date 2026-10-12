#include "trackList.h"
#include "appPaths.h"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <system_error>

namespace fs = std::filesystem;


// Says yes if a file is a music file (looks only at the ending, like ".mp3").
static bool isMusic(const fs::path& p)
{
    std::string ending;
    try {
        ending = p.extension().string();
    }
    catch (const std::system_error&) {
        return false;   // an ending we cannot read as plain text is not ".mp3" etc.
    }

    for (char& c : ending)
        c = (char)std::tolower((unsigned char)c);

    return ending == ".mp3" || ending == ".wav" || ending == ".flac" ||
        ending == ".m4a" || ending == ".aac" || ending == ".aif" || ending == ".aiff";
}


// Small letters and "/" slashes only, so two ways of writing the same address compare equal.
static std::string plainAddress(std::string text)
{
    for (char& c : text)
        c = c == '\\' ? '/' : (char)std::tolower((unsigned char)c);

    return text;
}


// Says yes when the file is a microphone recording: it lives in the Windows recordings folder
// (Documents\Sound Recordings) or inside one of the profile folders (where Luvista saves its own).
// Recordings have their own screen and must NEVER show up as music - even when the music
// folder covers them.
static bool isRecordingFile(const fs::path& p)
{
    static const std::string profiles = plainAddress(AppPaths::profilesFolder()) + "/";
    std::string where;

    try {
        where = plainAddress(p.string());
    }
    catch (const std::system_error&) {
        return false;   // a name plain text cannot hold: scan() skips such a file anyway
    }

    return where.find("sound recordings") != std::string::npos || where.rfind(profiles, 0) == 0;
}


// A line looks like:  5 <tab> C:/music/Adele - Hello.mp3 <tab> Hello <tab> ...
// This gives back one piece of the line. Piece 0 is the first, piece 1 is the second...
static std::string piece(const std::string& line, int number)
{
    int tabsSeen = 0;
    std::string result = "";

    for (char c : line) {
        if (c == '\t')
            tabsSeen++;
        else if (tabsSeen == number)
            result += c;
    }

    return result;
}


// "Adele - Hello" is cut at " - ": artist = "Adele", title = "Hello".
// If there is no " - " in the name, artist is "Unknown artist" and title is the whole name.
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


// The time right now, in seconds since 1970.
static long long nowSeconds()
{
    return (long long)std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}


// Newest first: the song with the newest "added" date moves to place 1.
static void sortNewestFirst(Song* list, int count)
{
    for (int i = 0; i < count; i++) {
        int best = i;

        for (int j = i + 1; j < count; j++)
            if (list[j].added > list[best].added)
                best = j;

        if (best != i) {
            Song temp = list[i];
            list[i] = list[best];
            list[best] = temp;
        }
    }
}


// ===========================================================================
// ID3 TAG READING  (reads the real title / artist / album inside an mp3 file)
// You do NOT need to understand this part. It works, so just leave it alone.
// ===========================================================================
struct Tags {
    std::string title;
    std::string artist;
    std::string album;
};


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


// Adds one Unicode character (a "code point") to a UTF-8 string.
// UTF-8 writes small numbers with 1 byte and bigger numbers with 2, 3 or 4 bytes:
//   up to 0x7F     -> 1 byte   (plain English letters)
//   up to 0x7FF    -> 2 bytes  (e.g. e with accent)
//   up to 0xFFFF   -> 3 bytes  (e.g. Nepali, Chinese, euro sign)
//   above that     -> 4 bytes  (e.g. emoji)
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

    const int encoding = data[0];
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

    // the index file uses tabs and new lines to split things, so none may stay inside the text
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

    const int version = header[3];          // 2, 3 or 4

    if (version < 2 || version > 4)
        return false;

    const unsigned int tagSize = syncsafe(header + 6);

    if (tagSize == 0 || tagSize > 16 * 1024 * 1024)
        return false;                       // empty, or too big to be sane

    std::string tag(tagSize, '\0');
    file.read(&tag[0], tagSize);
    tag.resize((size_t)file.gcount());

    const unsigned char* t = (const unsigned char*)tag.data();
    const unsigned int end = (unsigned int)tag.size();
    unsigned int pos = 0;

    // skip the extended header if there is one
    if (header[5] & 0x40) {
        if (version == 4 && end >= 4)
            pos = syncsafe(t);
        else if (version == 3 && end >= 4)
            pos = bigEndian(t, 4) + 4;
    }

    const int idLength = (version == 2) ? 3 : 4;
    const unsigned int headLength = (version == 2) ? 6 : 10;

    while (pos + headLength <= end) {
        if (t[pos] == 0)
            break;                          // padding: no more frames

        const std::string id((const char*)t + pos, idLength);

        unsigned int size;

        if (version == 2)
            size = bigEndian(t + pos + 3, 3);
        else if (version == 3)
            size = bigEndian(t + pos + 4, 4);
        else
            size = syncsafe(t + pos + 4);

        const unsigned int dataStart = pos + headLength;

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


// Looks inside the file for its real title, artist and album.
// Whatever the file does not have stays as it was. A missing album becomes "Unknown album".
static void fillFromTags(const std::string& path,
    std::string& title, std::string& artist, std::string& album)
{
    Tags tags;

    if (readTags(path, tags)) {
        if (!tags.title.empty())
            title = tags.title;

        if (!tags.artist.empty())
            artist = tags.artist;

        album = tags.album;
    }
    else {
        album.clear();
    }

    if (album.empty())
        album = "Unknown album";
}


// ===========================================================================
// TrackList
// ===========================================================================

TrackList::TrackList()
{
    folder = "";
    nextId = 1;
    songCount = 0;
    songs.resize(capacity);     // heap storage, so the object itself stays small
    loadFile();
}


// Reads the notebook file one time and puts every song into the list.
void TrackList::loadFile()
{
    songCount = 0;

    std::ifstream notebook(AppPaths::indexFile());
    std::string line;

    while (std::getline(notebook, line)) {
        if (line == "")
            continue;

        Song s;
        s.id = std::atoi(piece(line, 0).c_str());
        s.path = piece(line, 1);
        s.title = piece(line, 2);
        s.artist = piece(line, 3);
        s.album = piece(line, 4);
        s.added = std::atoll(piece(line, 5).c_str());

        if (isRecordingFile(s.path))
            continue;               // an old recording in the index: drop it, music library only

        if (songCount >= capacity)   // the array is full: stop reading
            break;

        songs[songCount] = s;
        songCount++;

        if (s.id >= nextId)          // so a new id is never the same as an old one
            nextId = s.id + 1;
    }

    sortNewestFirst(songs.data(), songCount);   // "Recently added": newest song on top
}


// Writes the whole list into the notebook file.
void TrackList::saveFile()
{
    std::ofstream notebook(AppPaths::indexFile());   // this erases the old notebook

    for (int i = 0; i < songCount; i++) {
        const Song& s = songs[i];

        notebook << s.id << '\t' << s.path << '\t' << s.title << '\t'
            << s.artist << '\t' << s.album << '\t' << s.added << '\n';
    }
}


// Old songs keep their id. New songs get a new id.
bool TrackList::scan(const std::string& folderPath)
{
    folder = folderPath;

    std::error_code problem;

    if (!fs::is_directory(folder, problem))
        return false;          // the folder is not there

    // Step 1: forget songs whose file is not there any more
    for (int i = songCount - 1; i >= 0; i--) {
        std::error_code e;

        if (!fs::exists(songs[i].path, e)) {
            // take songs[i] out: every song behind it moves one place up
            for (int j = i + 1; j < songCount; j++)
                songs[j - 1] = songs[j];

            songCount--;
        }
    }

    // Step 2: old songs that have no album yet get their tags read now
    for (int i = 0; i < songCount; i++) {
        Song& s = songs[i];

        if (s.album == "-" || s.album == "")
            fillFromTags(s.path, s.title, s.artist, s.album);

        if (s.added == 0)
            s.added = nowSeconds();
    }

    // Step 3: walk through the folder (and the folders inside it)
    fs::recursive_directory_iterator it(
        folder, fs::directory_options::skip_permission_denied, problem);

    for (; !problem && it != fs::recursive_directory_iterator(); it.increment(problem)) {

        // recordings belong to the Recordings page, never to the music library
        if (it->is_regular_file(problem) && isMusic(it->path()) &&
            !isRecordingFile(it->path())) {
            std::string path;
            std::string fileStem;

            try {
                path = it->path().string();
                fileStem = it->path().stem().string();
            }
            catch (const std::system_error&) {
                continue;   // a file name plain text cannot hold (emoji, other scripts ...): skip this file
            }

            if (find(path) == 0) {                        // a new song
                Song s;
                s.id = nextId;
                s.path = path;
                cutName(fileStem, s.artist, s.title);   // file name first
                fillFromTags(path, s.title, s.artist, s.album);           // tags in the file win
                s.added = nowSeconds();

                if (songCount >= capacity)   // the array is full: stop adding
                    break;

                songs[songCount] = s;
                songCount++;
                nextId++;
            }
        }
    }

    sortNewestFirst(songs.data(), songCount);   // "Recently added": newest song on top
    saveFile();
    return true;
}


// Takes song number n out of the list. The music file itself stays on disk.
bool TrackList::removeSong(int n)
{
    if (n < 1 || n > songCount)
        return false;

    // take songs[n-1] out: every song behind it moves one place up
    for (int i = n; i < songCount; i++)
        songs[i - 1] = songs[i];

    songCount--;
    saveFile();
    return true;
}


int TrackList::getCount() const
{
    return songCount;
}


std::string TrackList::getTrack(int n) const
{
    if (n < 1 || n > songCount)
        return "";

    return songs[n - 1].path;
}


std::string TrackList::getTitle(int n) const
{
    if (n < 1 || n > songCount)
        return "";

    return songs[n - 1].title;
}


std::string TrackList::getArtist(int n) const
{
    if (n < 1 || n > songCount)
        return "";

    return songs[n - 1].artist;
}


std::string TrackList::getAlbum(int n) const
{
    if (n < 1 || n > songCount)
        return "";

    return songs[n - 1].album;
}


// When song number n was added (seconds since 1970). 0 if there is no such song.
// The Recordings / library tables show this as the "Date added" column.
long long TrackList::getAdded(int n) const
{
    if (n < 1 || n > songCount)
        return 0;

    return songs[n - 1].added;
}


int TrackList::getId(int n) const
{
    if (n < 1 || n > songCount)
        return 0;

    return songs[n - 1].id;
}


// Looks for a path in the list. Gives the song number, or 0 if it is not there.
int TrackList::find(const std::string& path) const
{
    for (int i = 0; i < songCount; i++) {
        if (songs[i].path == path)
            return i + 1;
    }

    return 0;
}


// Looks for an id in the list. Gives the song number, or 0.
int TrackList::positionOfId(int id) const
{
    for (int i = 0; i < songCount; i++) {
        if (songs[i].id == id)
            return i + 1;
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
    if (n < 1 || n > songCount)
        return false;

    if (query == "")
        return true;

    std::string wanted = lowerCase(query);

    return lowerCase(songs[n - 1].title).find(wanted) != std::string::npos ||
        lowerCase(songs[n - 1].artist).find(wanted) != std::string::npos ||
        lowerCase(songs[n - 1].album).find(wanted) != std::string::npos;
}


// The folder that was scanned last ("" if none yet).
std::string TrackList::getFolder() const
{
    return folder;
}
