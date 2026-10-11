#include "CoverArt.h"
#include "../core/appPaths.h"
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

typedef std::vector<unsigned char> Bytes;


static std::string lowerCase(std::string s)
{
    for (char& c : s)
        c = (char)std::tolower((unsigned char)c);

    return s;
}


// "jpg" or "png" if the bytes start like such a picture, "" if not.
static std::string pictureKind(const unsigned char* b, size_t size)
{
    if (size >= 3 && b[0] == 0xFF && b[1] == 0xD8 && b[2] == 0xFF)
        return "jpg";

    if (size >= 8 && b[0] == 0x89 && b[1] == 'P' && b[2] == 'N' && b[3] == 'G')
        return "png";

    return "";
}


// Looks for the start of a picture in the first bytes of a frame. The bytes in front of it
// (text encoding, mime type, picture type, description) are in the way and differ per tag version,
// so instead of reading each of them we just search for the first byte that starts a picture.
static bool findPicture(const unsigned char* data, size_t size, size_t& start, std::string& kind)
{
    size_t looking = size < 1024 ? size : 1024;

    for (size_t i = 0; i < looking; i++) {
        std::string k = pictureKind(data + i, size - i);

        if (k != "") {
            start = i;
            kind = k;
            return true;
        }
    }

    return false;
}


static bool writeFile(const std::string& path, const unsigned char* data, size_t size)
{
    std::ofstream out(path, std::ios::binary);

    if (!out)
        return false;

    out.write((const char*)data, (std::streamsize)size);
    out.close();
    return !out.fail();
}


// mp3

static unsigned int syncsafe(const unsigned char* b)
{
    return ((unsigned int)(b[0] & 0x7F) << 21) | ((unsigned int)(b[1] & 0x7F) << 14) |
        ((unsigned int)(b[2] & 0x7F) << 7) | (unsigned int)(b[3] & 0x7F);
}


static unsigned int bigEndian(const unsigned char* b, int bytes)
{
    unsigned int n = 0;

    for (int i = 0; i < bytes; i++)
        n = (n << 8) | b[i];

    return n;
}


// "unsynchronisation" put a zero after every 0xFF byte. Takes those zeros out again.
static Bytes removeUnsync(const unsigned char* data, size_t size)
{
    Bytes out;
    out.reserve(size);

    for (size_t i = 0; i < size; i++) {
        out.push_back(data[i]);

        if (data[i] == 0xFF && i + 1 < size && data[i + 1] == 0x00)
            i++;
    }

    return out;
}


static std::string extractFromMp3(const std::string& songPath, const std::string& outBase)
{
    std::ifstream file(songPath, std::ios::binary);

    if (!file)
        return "";

    unsigned char header[10];
    file.read((char*)header, 10);

    if (file.gcount() != 10 || header[0] != 'I' || header[1] != 'D' || header[2] != '3')
        return "";

    int version = header[3];

    if (version < 2 || version > 4)
        return "";

    unsigned int tagSize = syncsafe(header + 6);

    if (tagSize == 0 || tagSize > 32 * 1024 * 1024)
        return "";

    Bytes tag(tagSize);
    file.read((char*)tag.data(), tagSize);
    tag.resize((size_t)file.gcount());

    // older tags (2.2 and 2.3) can be unsynchronised as a whole. 2.4 does it frame by frame
    if ((header[5] & 0x80) && version < 4)
        tag = removeUnsync(tag.data(), tag.size());

    const unsigned char* t = tag.data();
    unsigned int end = (unsigned int)tag.size();
    unsigned int pos = 0;

    if (header[5] & 0x40) {                     // skip the extended header
        if (version == 4 && end >= 4)
            pos = syncsafe(t);
        else if (version == 3 && end >= 4)
            pos = bigEndian(t, 4) + 4;
    }

    int idLength = (version == 2) ? 3 : 4;
    int headLength = (version == 2) ? 6 : 10;

    while (pos + headLength <= end) {
        if (t[pos] == 0)
            break;

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
            break;

        if (id == "APIC" || id == "PIC") {
            const unsigned char* data = t + dataStart;
            Bytes cleaned;

            // 2.4 frame flags: the second flag byte, bit 0x02, means this frame is unsynchronised
            if (version == 4 && (t[pos + 9] & 0x02)) {
                cleaned = removeUnsync(data, size);
                data = cleaned.data();
                size = (unsigned int)cleaned.size();
            }

            size_t start = 0;
            std::string kind;

            if (findPicture(data, size, start, kind)) {
                std::string out = outBase + "." + kind;

                if (writeFile(out, data + start, size - start))
                    return out;
            }
        }

        pos = dataStart + size;
    }

    return "";
}


// m4a: the picture is at moov > udta > meta > ilst > covr > data. A box is
// 4 bytes of size, 4 letters of name, then its content (which can hold more boxes).

static bool readHeader(std::ifstream& f, uint64_t pos, uint64_t end, std::string& type,
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

        if (!readHeader(f, pos, end, type, head, size))
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


static std::string extractFromM4a(const std::string& songPath, const std::string& outBase)
{
    std::ifstream file(songPath, std::ios::binary);

    if (!file)
        return "";

    file.seekg(0, std::ios::end);
    uint64_t fileSize = (uint64_t)file.tellg();

    uint64_t bodyStart = 0, bodyEnd = 0;
    std::vector<std::string> path = { "moov", "udta", "meta", "ilst", "covr" };

    if (!findBox(file, 0, fileSize, path, 0, bodyStart, bodyEnd))
        return "";

    // inside covr: a "data" box = 8 bytes of box header, 4 of type, 4 of language, then the picture
    std::string type;
    uint64_t head = 0, size = 0;

    if (!readHeader(file, bodyStart, bodyEnd, type, head, size) || type != "data" || size < head + 8)
        return "";

    uint64_t length = size - head - 8;

    if (length == 0 || length > 32 * 1024 * 1024)
        return "";

    Bytes picture((size_t)length);
    file.clear();
    file.seekg((std::streamoff)(bodyStart + head + 8));
    file.read((char*)picture.data(), (std::streamsize)length);

    if ((uint64_t)file.gcount() != length)
        return "";

    std::string kind = pictureKind(picture.data(), picture.size());

    if (kind == "")
        return "";

    std::string out = outBase + "." + kind;
    return writeFile(out, picture.data(), picture.size()) ? out : "";
}


// a picture lying in the song's folder
static std::string findInFolder(const std::string& songPath)
{
    const char* names[] = { "cover", "folder", "front", "album", "albumart" };
    const char* endings[] = { ".jpg", ".jpeg", ".png" };

    std::error_code problem;
    fs::path folder = fs::path(songPath).parent_path();

    for (const char* name : names) {
        for (const char* ending : endings) {
            fs::path candidate = folder / (std::string(name) + ending);

            if (fs::is_regular_file(candidate, problem))
                return candidate.string();
        }
    }

    return "";
}


namespace CoverArt
{
    std::string extract(const std::string& songPath, const std::string& outBase)
    {
        std::string ending = lowerCase(fs::path(songPath).extension().string());

        if (ending == ".mp3")
            return extractFromMp3(songPath, outBase);

        if (ending == ".m4a")
            return extractFromM4a(songPath, outBase);

        return "";
    }


    std::string forSong(int songId, const std::string& songPath)
    {
        if (songId <= 0)
            return "";

        std::error_code problem;
        fs::path base = fs::path(AppPaths::coversFolder()) / std::to_string(songId);

        // already cut out on an earlier run
        for (const char* ending : { ".jpg", ".png" }) {
            fs::path cached = base.string() + ending;

            if (fs::is_regular_file(cached, problem))
                return cached.string();
        }

        // ".none" is a note to self: this song has no picture inside, do not read its tags again
        fs::path none = base.string() + ".none";

        if (!fs::exists(none, problem)) {
            std::string saved = extract(songPath, base.string());

            if (saved != "")
                return saved;

            std::ofstream mark(none);
        }

        return findInFolder(songPath);
    }
}
