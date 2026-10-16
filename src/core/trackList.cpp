#include "trackList.h"
#include "appPaths.h"
#include "textUtil.h"
#include "pathUtil.h"
#include "../audio/AudioInfo.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <unordered_set>

namespace fs = std::filesystem;

// The index file is a list of lines. A newer layout is marked with this number, so the songs of an
// older index get their names and lengths looked at again once.
static const int FORMAT_VERSION = 2;


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


static std::string extensionOf(const std::string& path)
{
    size_t dot = path.find_last_of('.');
    size_t slash = path.find_last_of("/\\");

    if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
        return "";

    return TextUtil::lower(path.substr(dot));
}


// Small letters and "/" slashes only, so two ways of writing the same address compare equal.
static std::string plainAddress(const std::string& text)
{
    return PathUtil::plain(text);
}


// Says yes when the file is a microphone recording: it lives in the Windows recordings folder
// (Documents\Sound Recordings) or inside one of the profile folders (where Luvista saves its own).
// Recordings have their own screen and must NEVER show up as music - even when the music
// folder covers them.
static bool isRecordingFile(const std::string& path)
{
    static const std::string profiles = plainAddress(AppPaths::profilesFolder()) + "/";
    std::string where = plainAddress(path);

    return where.find("sound recordings") != std::string::npos || where.rfind(profiles, 0) == 0;
}


// The time right now, in seconds since 1970.
static long long nowSeconds()
{
    return (long long)std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}


// One line of the index split at its tabs: 5 <tab> C:/music/Adele - Hello.mp3 <tab> Hello <tab> ...
static std::vector<std::string> splitTabs(const std::string& line)
{
    std::vector<std::string> parts(1);

    for (char c : line) {
        if (c == '\t')
            parts.emplace_back();
        else
            parts.back() += c;
    }

    return parts;
}


// ===========================================================================
// TAG READING  (the real title / artist / album inside a music file)
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


// Plain bytes of a tag (Latin-1 or UTF-8) as UTF-8. Many taggers write UTF-8 while the tag says Latin-1,
// so bytes that ARE valid UTF-8 are taken as UTF-8, the rest as Windows-1252.
static std::string bytesToUtf8(const std::string& raw)
{
    std::string out;

    if (TextUtil::isUtf8(raw)) {
        out = raw;
    }
    else {
        for (unsigned char c : raw)
            TextUtil::appendCp1252(out, c);
    }

    return TextUtil::repairMojibake(out);
}


// Turns the text of an ID3 frame into UTF-8. The first byte says how the text is written:
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
        std::string raw;

        for (unsigned int i = 0; i < size; i++) {
            if (data[i] == 0)
                break;

            raw += (char)data[i];
        }

        out = bytesToUtf8(raw);
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


// ID3v2 at the start of an mp3
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


// ID3v1: the last 128 bytes of old mp3 files
static bool readId3v1(const std::string& path, Tags& out)
{
    std::ifstream file(path, std::ios::binary);

    if (!file)
        return false;

    file.seekg(-128, std::ios::end);

    if (!file)
        return false;

    unsigned char block[128];
    file.read((char*)block, 128);

    if (file.gcount() != 128 || block[0] != 'T' || block[1] != 'A' || block[2] != 'G')
        return false;

    auto field = [&](int from, int length) {
        std::string raw;

        for (int i = 0; i < length && block[from + i] != 0; i++)
            raw += (char)block[from + i];

        return TextUtil::trim(bytesToUtf8(raw));
    };

    out.title = field(3, 30);
    out.artist = field(33, 30);
    out.album = field(63, 30);
    return true;
}


// m4a: the tags are at moov > udta > meta > ilst. A box is 4 bytes of size, 4 letters of name,
// then its content (which can hold more boxes).
static bool readBoxHeader(std::ifstream& f, uint64_t pos, uint64_t end, std::string& type,
    uint64_t& headLength, uint64_t& size)
{
    unsigned char h[16];

    f.clear();
    f.seekg((std::streamoff)pos);
    f.read((char*)h, 8);

    if (f.gcount() != 8)
        return false;

    size = bigEndian(h, 4);
    type.assign((const char*)h + 4, 4);
    headLength = 8;

    if (size == 1) {                            // the real size is in 8 more bytes
        f.read((char*)h + 8, 8);

        if (f.gcount() != 8)
            return false;

        size = ((uint64_t)bigEndian(h + 8, 4) << 32) | bigEndian(h + 12, 4);
        headLength = 16;
    }
    else if (size == 0) {                       // runs to the end
        size = end - pos;
    }

    return size >= headLength && pos + size <= end;
}


static bool findBox(std::ifstream& f, uint64_t start, uint64_t end, const std::vector<std::string>& path,
    size_t depth, uint64_t& bodyStart, uint64_t& bodyEnd)
{
    uint64_t pos = start;

    while (pos + 8 <= end) {
        std::string type;
        uint64_t head = 0, size = 0;

        if (!readBoxHeader(f, pos, end, type, head, size))
            return false;

        if (type == path[depth]) {
            uint64_t from = pos + head;
            uint64_t to = pos + size;

            if (depth + 1 == path.size()) {
                bodyStart = from;
                bodyEnd = to;
                return true;
            }

            if (type == "meta")
                from += 4;                      // "meta" has 4 bytes of version and flags first

            if (findBox(f, from, to, path, depth + 1, bodyStart, bodyEnd))
                return true;
        }

        pos += size;
    }

    return false;
}


static bool readM4aTags(const std::string& songPath, Tags& out)
{
    std::ifstream file(songPath, std::ios::binary);

    if (!file)
        return false;

    file.seekg(0, std::ios::end);
    uint64_t fileSize = (uint64_t)file.tellg();

    uint64_t bodyStart = 0, bodyEnd = 0;
    std::vector<std::string> path = { "moov", "udta", "meta", "ilst" };

    if (!findBox(file, 0, fileSize, path, 0, bodyStart, bodyEnd))
        return false;

    uint64_t pos = bodyStart;

    while (pos + 8 <= bodyEnd) {
        std::string type;
        uint64_t head = 0, size = 0;

        if (!readBoxHeader(file, pos, bodyEnd, type, head, size))
            break;

        // 0xA9 is the "(c)" sign in front of nam / ART / alb
        std::string* target = nullptr;

        if (type == "\xA9" "nam") target = &out.title;
        else if (type == "\xA9" "ART") target = &out.artist;
        else if (type == "\xA9" "alb") target = &out.album;

        if (target) {
            // inside the item: a "data" box = box header, 4 bytes of type, 4 of language, then the text
            std::string innerType;
            uint64_t innerHead = 0, innerSize = 0;

            if (readBoxHeader(file, pos + head, pos + size, innerType, innerHead, innerSize) &&
                innerType == "data" && innerSize > innerHead + 8) {
                uint64_t length = innerSize - innerHead - 8;

                if (length > 0 && length <= 2048) {
                    std::string text((size_t)length, '\0');
                    file.clear();
                    file.seekg((std::streamoff)(pos + head + innerHead + 8));
                    file.read(&text[0], (std::streamsize)length);

                    if ((uint64_t)file.gcount() == length)
                        *target = bytesToUtf8(text);
                }
            }
        }

        pos += size;
    }

    return true;
}


// ===========================================================================
// WHAT TO CALL A SONG
// ===========================================================================
namespace {

typedef std::unordered_set<std::string> NameSet;

struct Names {
    std::string title, artist, album;
};


bool startsWith(const std::string& text, const std::string& start)
{
    return text.size() >= start.size() && text.compare(0, start.size(), start) == 0;
}


bool endsWith(const std::string& text, const std::string& end)
{
    return text.size() >= end.size() && text.compare(text.size() - end.size(), end.size(), end) == 0;
}


// "Hello (Official Video)" looks like a title, "Adele" does not
bool looksLikeTitle(const std::string& text)
{
    std::string low = TextUtil::lower(text);

    static const char* words[] = {
        "(", "[", "feat", " ft.", " ft ", "official", "lyric", "remix", "audio", "video", "live", "version", "remaster", "cover", "acoustic"
    };

    for (const char* word : words) {
        if (low.find(word) != std::string::npos)
            return true;
    }

    return false;
}


// A file name "A - B" cut in two. false if there is no " - " with something on both sides.
bool cutAtDash(const std::string& text, std::string& left, std::string& right)
{
    size_t place = text.find(" - ");

    if (place == std::string::npos || place == 0 || place + 3 >= text.size())
        return false;

    left = TextUtil::trim(text.substr(0, place));
    right = TextUtil::trim(text.substr(place + 3));
    return !left.empty() && !right.empty();
}


std::string cleanOrEmpty(const std::string& text)
{
    std::string cleaned = TextUtil::cleanTag(text);
    return TextUtil::looksGarbled(cleaned) ? std::string() : cleaned;
}


// Works out title, artist and album from what the file says about itself, then from its name.
// When neither gives a trustworthy answer the whole file name becomes the title, so the song is never shown
// with the artist and the title the wrong way round.
Names analyse(const std::string& path, const std::string& stem, const NameSet& knownArtists)
{
    Tags t;
    std::string ext = extensionOf(path);

    if (ext == ".mp3") {
        bool v2 = readTags(path, t);

        if (!v2 || (t.title.empty() && t.artist.empty())) {
            Tags v1;

            if (readId3v1(path, v1)) {
                if (t.title.empty()) t.title = v1.title;
                if (t.artist.empty()) t.artist = v1.artist;
                if (t.album.empty()) t.album = v1.album;
            }
        }
    }
    else if (ext == ".m4a") {
        readM4aTags(path, t);
    }

    std::string title = cleanOrEmpty(t.title);
    std::string artist = cleanOrEmpty(t.artist);
    std::string album = cleanOrEmpty(t.album);
    std::string name = TextUtil::cleanTag(stem);        // the file name, kept as it is
    std::string left, right;
    bool dashed = cutAtDash(name, left, right);

    auto known = [&](const std::string& who) { return knownArtists.count(TextUtil::lower(who)) > 0; };

    if (title.empty()) {
        // no usable title in the file: read it from the file name
        if (dashed) {
            if (!artist.empty()) {
                std::string a = TextUtil::lower(artist);

                if (TextUtil::lower(left) == a)         title = right;
                else if (TextUtil::lower(right) == a)   title = left;
                else                                    title = name;
            }
            else if (known(left) && !known(right)) { artist = left; title = right; }
            else if (known(right) && !known(left)) { artist = right; title = left; }
            else if (looksLikeTitle(right) && !looksLikeTitle(left)) { artist = left; title = right; }
            else if (looksLikeTitle(left) && !looksLikeTitle(right)) { artist = right; title = left; }
            else title = name;                          // cannot tell which side is which: show the file name
        }
        else {
            title = name;
        }
    }
    else if (artist.empty()) {
        // a title but no artist: the title may be "Artist - Title", or the file name tells
        std::string a, b;

        if (cutAtDash(title, a, b)) {
            if (known(a) && !known(b)) { artist = a; title = b; }
            else if (known(b) && !known(a)) { artist = b; title = a; }
        }
        else if (dashed) {
            std::string low = TextUtil::lower(title);

            if (TextUtil::lower(right) == low) artist = left;
            else if (TextUtil::lower(left) == low) artist = right;
        }
    }
    else {
        // both are there. An artist that is also written in the title ("Adele - Hello") is taken out of it
        std::string a = TextUtil::lower(artist);
        std::string low = TextUtil::lower(title);

        if (startsWith(low, a + " - ") && title.size() > artist.size() + 3)
            title = TextUtil::trim(title.substr(artist.size() + 3));
        else if (endsWith(low, " - " + a) && title.size() > artist.size() + 3)
            title = TextUtil::trim(title.substr(0, title.size() - artist.size() - 3));
        // tags written the wrong way round: the "title" is a known artist, the "artist" is not, and the file name agrees
        else if (dashed && known(title) && !known(artist) &&
                 TextUtil::lower(left) == low && TextUtil::lower(right) == a)
            std::swap(title, artist);
    }

    if (title.empty())
        title = name.empty() ? std::string("Unknown") : name;

    if (artist.empty())
        artist = "Unknown artist";

    if (album.empty())
        album = "Unknown album";

    return { title, artist, album };
}


NameSet artistsOf(const std::vector<std::string>& lowerNames)
{
    return NameSet(lowerNames.begin(), lowerNames.end());
}

}   // namespace


// ===========================================================================
// TrackList
// ===========================================================================

TrackList::TrackList()
{
    folder = "";
    nextId = 1;
    formatVersion = FORMAT_VERSION;
    loadFile();
    loadRemoved();
}


// Reads the notebook file one time and puts every song into the list.
void TrackList::loadFile()
{
    songs.clear();
    nextId = 1;
    formatVersion = FORMAT_VERSION;

    std::ifstream notebook(AppPaths::indexFile());
    std::string line;
    bool exists = (bool)notebook;
    bool versionSeen = false;
    int savedNext = 1;
    long long now = nowSeconds();

    while (std::getline(notebook, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line == "")
            continue;

        if (line[0] == '#') {                       // "#v=2" and "#next=57"
            if (line.compare(0, 3, "#v=") == 0) {
                formatVersion = std::atoi(line.c_str() + 3);
                versionSeen = true;
            }
            else if (line.compare(0, 6, "#next=") == 0) {
                savedNext = std::atoi(line.c_str() + 6);
            }

            continue;
        }

        std::vector<std::string> parts = splitTabs(line);

        if (parts.size() < 2)
            continue;

        Song s;
        s.id = std::atoi(parts[0].c_str());
        s.path = parts[1];
        s.title = parts.size() > 2 ? parts[2] : "";
        s.artist = parts.size() > 3 ? parts[3] : "";
        s.album = parts.size() > 4 ? parts[4] : "";
        s.added = parts.size() > 5 ? std::atoll(parts[5].c_str()) : 0;
        s.seconds = parts.size() > 6 ? std::atof(parts[6].c_str()) : 0;

        if (s.id <= 0 || s.path.empty() || isRecordingFile(s.path))
            continue;                               // an old recording in the index: drop it, music library only

        if ((int)songs.size() >= capacity)          // the list is full: stop reading
            break;

        if (s.added == 0)
            s.added = now;

        if (s.id >= nextId)                         // so a new id is never the same as an old one
            nextId = s.id + 1;

        songs.push_back(s);
    }

    // ids are never handed out twice, not even for a song that was removed and the program restarted
    if (savedNext > nextId)
        nextId = savedNext;

    // a file from before the version line existed is version 1. No file at all has nothing to look at again.
    if (exists && !versionSeen)
        formatVersion = 1;

    std::stable_sort(songs.begin(), songs.end(), [](const Song& a, const Song& b) { return a.added > b.added; });
    rebuildIndexes();
}


// Writes the whole list into the notebook file (a new file first, then it replaces the old one).
void TrackList::saveFile()
{
    std::string target = AppPaths::indexFile();
    std::string temporary = target + ".tmp";

    {
        std::ofstream notebook(temporary, std::ios::trunc);

        notebook << "#v=" << formatVersion << '\n';
        notebook << "#next=" << nextId << '\n';

        for (const Song& s : songs) {
            notebook << s.id << '\t' << s.path << '\t' << s.title << '\t'
                << s.artist << '\t' << s.album << '\t' << s.added << '\t' << s.seconds << '\n';
        }

        if (!notebook)
            return;                                 // keep the old file rather than a half-written one
    }

    std::error_code problem;
    fs::rename(temporary, target, problem);

    if (problem) {                                  // some systems do not replace a file: copy over it instead
        fs::copy_file(temporary, target, fs::copy_options::overwrite_existing, problem);
        fs::remove(temporary, problem);
    }
}


void TrackList::rebuildIndexes()
{
    pathIndex.clear();
    idIndex.clear();
    pathIndex.reserve(songs.size() * 2);
    idIndex.reserve(songs.size() * 2);

    for (size_t i = 0; i < songs.size(); i++) {
        pathIndex[plainAddress(songs[i].path)] = (int)i + 1;
        idIndex[songs[i].id] = (int)i + 1;
    }
}


TrackList::Snapshot TrackList::snapshot() const
{
    Snapshot shot;
    NameSet artists;

    shot.paths.reserve(songs.size());

    for (const Song& s : songs) {
        shot.paths.push_back(s.path);

        std::string who = TextUtil::lower(s.artist);

        if (!who.empty() && who != "unknown artist" && who != "unknown")
            artists.insert(who);
    }

    shot.artists.assign(artists.begin(), artists.end());

    for (const RemovedSong& r : removedList)
        shot.removed.push_back(r.path);

    return shot;
}


// Walks the folder (and the folders inside it). A song that is already known is skipped without looking inside
// its file, so a folder that was scanned before costs one walk and nothing more.
ScanResult TrackList::discover(const std::string& folderPath, const Snapshot& known, const std::atomic<bool>* cancel)
{
    ScanResult result;
    result.folder = folderPath;

    std::error_code problem;

    if (!fs::is_directory(folderPath, problem))
        return result;                              // the folder is not there

    result.ok = true;

    // songs whose file is not there any more
    for (const std::string& path : known.paths) {
        if (cancel && cancel->load()) {
            result.ok = false;
            return result;
        }

        // only songs inside the folder being scanned are checked: another music folder may be on a drive
        // that is not plugged in, and its songs must stay
        if (!PathUtil::isInside(path, folderPath))
            continue;

        std::error_code e;

        if (!fs::exists(path, e))
            result.missing.push_back(path);
    }

    NameSet have;
    have.reserve(known.paths.size() * 2);

    for (const std::string& path : known.paths)
        have.insert(plainAddress(path));

    NameSet artists = artistsOf(known.artists);

    // songs removed on purpose are not found again
    NameSet removedKeys;

    for (const std::string& path : known.removed)
        removedKeys.insert(plainAddress(path));

    fs::recursive_directory_iterator it(folderPath, fs::directory_options::skip_permission_denied, problem);

    for (; !problem && it != fs::recursive_directory_iterator(); it.increment(problem)) {
        if (cancel && cancel->load()) {
            result.ok = false;
            return result;
        }

        std::error_code e;

        if (!it->is_regular_file(e) || !isMusic(it->path()))
            continue;

        std::string path;
        std::string fileStem;

        try {
            path = it->path().string();
            fileStem = it->path().stem().string();
        }
        catch (const std::system_error&) {
            continue;       // a file name plain text cannot hold (emoji, other scripts ...): skip this file
        }

        // recordings belong to the Recordings page, never to the music library
        if (isRecordingFile(path))
            continue;

        if (removedKeys.count(plainAddress(path)))
            continue;

        // the same file found by another spelling of its address (c:\Music vs C:/music), or reached twice: only once
        if (!have.insert(plainAddress(path)).second)
            continue;

        Song s;
        Names names = analyse(path, fileStem, artists);
        s.path = path;
        s.title = names.title;
        s.artist = names.artist;
        s.album = names.album;
        s.added = nowSeconds();
        result.found.push_back(s);

        if ((int)(known.paths.size() + result.found.size()) >= capacity)
            break;

        // artists of the songs found so far help to read the next file names
        std::string who = TextUtil::lower(s.artist);

        if (who != "unknown artist" && !names.title.empty() && names.title != TextUtil::cleanTag(fileStem))
            artists.insert(who);
    }

    return result;
}


int TrackList::apply(const ScanResult& result)
{
    if (!result.ok)
        return 0;

    folder = result.folder;
    int changed = 0;

    if (!result.missing.empty()) {
        NameSet gone;

        for (const std::string& path : result.missing)
            gone.insert(plainAddress(path));

        size_t before = songs.size();
        songs.erase(std::remove_if(songs.begin(), songs.end(),
            [&](const Song& s) { return gone.count(plainAddress(s.path)) > 0; }), songs.end());

        changed += (int)(before - songs.size());
        rebuildIndexes();
    }

    for (const Song& found : result.found) {
        if ((int)songs.size() >= capacity)
            break;

        std::string key = plainAddress(found.path);

        if (pathIndex.count(key) || isRemoved(found.path))
            continue;                               // it joined (or was removed) while the folder was being walked

        Song s = found;
        s.id = nextId++;
        songs.push_back(s);
        pathIndex[key] = (int)songs.size();
        idIndex[s.id] = (int)songs.size();
        changed++;
    }

    if (changed > 0) {
        // newest song first. Songs added at the same second keep the order they were found in
        std::stable_sort(songs.begin(), songs.end(), [](const Song& a, const Song& b) { return a.added > b.added; });
        rebuildIndexes();
        saveFile();
    }

    return changed;
}


// Old songs keep their id. New songs get a new id.
bool TrackList::scan(const std::string& folderPath)
{
    ScanResult result = discover(folderPath, snapshot());

    if (!result.ok)
        return false;

    apply(result);
    return true;
}


std::vector<Song> TrackList::needsWork() const
{
    std::vector<Song> work;

    for (const Song& s : songs) {
        if (formatVersion < FORMAT_VERSION || s.seconds == 0)
            work.push_back(s);
    }

    return work;
}


std::vector<SongFix> TrackList::repair(const std::vector<Song>& work, const Snapshot& known, const std::atomic<bool>* cancel)
{
    std::vector<SongFix> fixes;
    NameSet artists = artistsOf(known.artists);

    fixes.reserve(work.size());

    for (const Song& s : work) {
        if (cancel && cancel->load())
            return std::vector<SongFix>();

        SongFix fix;
        fix.id = s.id;
        fix.seconds = s.seconds;

        if (fix.seconds == 0) {
            double length = AudioInfo::seconds(s.path);
            fix.seconds = length > 0 ? length : -1;     // -1: do not try this file again next time
        }

        std::string stem = fs::path(s.path).stem().string();
        Names names = analyse(s.path, stem, artists);
        fix.title = names.title;
        fix.artist = names.artist;
        fix.album = names.album;
        fix.names = true;
        fixes.push_back(fix);
    }

    return fixes;
}


void TrackList::applyFixes(const std::vector<SongFix>& fixes)
{
    for (const SongFix& fix : fixes) {
        auto at = idIndex.find(fix.id);

        if (at == idIndex.end())
            continue;                               // the song was removed in the meantime

        Song& s = songs[at->second - 1];

        if (fix.names) {
            s.title = fix.title;
            s.artist = fix.artist;
            s.album = fix.album;
        }

        s.seconds = fix.seconds;
    }

    formatVersion = FORMAT_VERSION;
    saveFile();
}


// Takes song number n out of the list. The music file itself stays on disk.
// The song is written down as removed, so the next scan of its folder does not bring it back.
bool TrackList::removeSong(int n, bool hide)
{
    if (n < 1 || n > (int)songs.size())
        return false;

    const Song& s = songs[n - 1];

    if (hide && !isRemoved(s.path)) {
        RemovedSong r;
        r.path = s.path;
        r.title = s.title;
        r.artist = s.artist;
        r.album = s.album;
        r.added = s.added;
        r.seconds = s.seconds;
        removedList.push_back(r);
        removedKeys.push_back(plainAddress(r.path));
        saveRemoved();
    }

    songs.erase(songs.begin() + (n - 1));
    rebuildIndexes();
    saveFile();
    return true;
}


bool TrackList::isRemoved(const std::string& path) const
{
    return std::find(removedKeys.begin(), removedKeys.end(), plainAddress(path)) != removedKeys.end();
}


void TrackList::loadRemoved()
{
    removedList.clear();
    removedKeys.clear();

    std::ifstream file(AppPaths::removedFile());
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        std::vector<std::string> parts = splitTabs(line);

        if (parts[0].empty())
            continue;

        RemovedSong r;
        r.path = parts[0];
        // a list of plain paths (an older layout of this file) has no names: the file name stands in for the title
        r.title = parts.size() > 1 ? parts[1] : fs::path(r.path).stem().string();
        r.artist = parts.size() > 2 ? parts[2] : "";
        r.album = parts.size() > 3 ? parts[3] : "";
        r.added = parts.size() > 4 ? std::atoll(parts[4].c_str()) : 0;
        r.seconds = parts.size() > 5 ? std::atof(parts[5].c_str()) : 0;

        removedKeys.push_back(plainAddress(r.path));
        removedList.push_back(r);
    }
}


void TrackList::saveRemoved()
{
    std::string target = AppPaths::removedFile();
    std::string temporary = target + ".tmp";

    {
        std::ofstream file(temporary, std::ios::trunc);

        for (const RemovedSong& r : removedList)
            file << r.path << '\t' << r.title << '\t' << r.artist << '\t' << r.album << '\t' << r.added << '\t' << r.seconds << '\n';

        if (!file)
            return;
    }

    std::error_code problem;
    fs::rename(temporary, target, problem);

    if (problem) {
        fs::copy_file(temporary, target, fs::copy_options::overwrite_existing, problem);
        fs::remove(temporary, problem);
    }
}


const std::vector<RemovedSong>& TrackList::removedSongs() const
{
    return removedList;
}


bool TrackList::restoreRemoved(int index)
{
    if (index < 0 || index >= (int)removedList.size())
        return false;

    RemovedSong r = removedList[index];
    removedList.erase(removedList.begin() + index);
    removedKeys.erase(removedKeys.begin() + index);
    saveRemoved();

    std::error_code problem;

    if (!fs::exists(r.path, problem) || (int)songs.size() >= capacity)
        return false;                               // the file is gone: nothing to bring back

    if (pathIndex.count(plainAddress(r.path)))
        return true;                                // it is in the library already

    Song s;
    s.id = nextId++;
    s.path = r.path;
    s.title = r.title;
    s.artist = r.artist;
    s.album = r.album;
    s.added = r.added > 0 ? r.added : nowSeconds();
    s.seconds = r.seconds;
    songs.push_back(s);

    std::stable_sort(songs.begin(), songs.end(), [](const Song& a, const Song& b) { return a.added > b.added; });
    rebuildIndexes();
    saveFile();
    return true;
}


void TrackList::restoreRemovedInside(const std::string& folder)
{
    size_t before = removedList.size();
    std::vector<RemovedSong> kept;
    std::vector<std::string> keys;

    for (const RemovedSong& r : removedList) {
        if (!PathUtil::isInside(r.path, folder)) {
            kept.push_back(r);
            keys.push_back(plainAddress(r.path));
        }
    }

    removedList = kept;
    removedKeys = keys;

    if (removedList.size() != before)
        saveRemoved();
}


int TrackList::restoreAllRemoved()
{
    int back = 0;

    while (!removedList.empty()) {
        if (restoreRemoved((int)removedList.size() - 1))
            back++;
    }

    return back;
}


int TrackList::getCount() const
{
    return (int)songs.size();
}


std::string TrackList::getTrack(int n) const
{
    if (n < 1 || n > (int)songs.size())
        return "";

    return songs[n - 1].path;
}


std::string TrackList::getTitle(int n) const
{
    if (n < 1 || n > (int)songs.size())
        return "";

    return songs[n - 1].title;
}


std::string TrackList::getArtist(int n) const
{
    if (n < 1 || n > (int)songs.size())
        return "";

    return songs[n - 1].artist;
}


std::string TrackList::getAlbum(int n) const
{
    if (n < 1 || n > (int)songs.size())
        return "";

    return songs[n - 1].album;
}


// When song number n was added (seconds since 1970). 0 if there is no such song.
long long TrackList::getAdded(int n) const
{
    if (n < 1 || n > (int)songs.size())
        return 0;

    return songs[n - 1].added;
}


// How long song number n is (0 when that is not known yet or the file has no length we can read).
double TrackList::getSeconds(int n) const
{
    if (n < 1 || n > (int)songs.size())
        return 0;

    return songs[n - 1].seconds > 0 ? songs[n - 1].seconds : 0;
}


int TrackList::getId(int n) const
{
    if (n < 1 || n > (int)songs.size())
        return 0;

    return songs[n - 1].id;
}


// Looks for a path in the list. Gives the song number, or 0 if it is not there.
int TrackList::find(const std::string& path) const
{
    auto at = pathIndex.find(plainAddress(path));
    return at == pathIndex.end() ? 0 : at->second;
}


// Looks for an id in the list. Gives the song number, or 0.
int TrackList::positionOfId(int id) const
{
    auto at = idIndex.find(id);
    return at == idIndex.end() ? 0 : at->second;
}


bool TrackList::matches(int n, const std::string& query) const
{
    if (n < 1 || n > (int)songs.size())
        return false;

    if (query == "")
        return true;

    std::string wanted = TextUtil::lower(query);

    return TextUtil::lower(songs[n - 1].title).find(wanted) != std::string::npos ||
        TextUtil::lower(songs[n - 1].artist).find(wanted) != std::string::npos ||
        TextUtil::lower(songs[n - 1].album).find(wanted) != std::string::npos;
}


// The folder that was scanned last (provided a scan found it, "" if none yet).
std::string TrackList::getFolder() const
{
    return folder;
}
