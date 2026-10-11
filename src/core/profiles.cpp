#include "profiles.h"
#include "appPaths.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

const char* Profiles::GUEST = "guest";


static std::string lowerCase(std::string s)
{
    for (char& c : s)
        c = (char)std::tolower((unsigned char)c);

    return s;
}


// A profile name is also a folder name, so it may not hold \ / : * ? " < > |
// and may not start or end with a space or a dot (windows does not allow that).
static bool nameIsOk(const std::string& name)
{
    if (name.empty() || name.size() > 40)
        return false;

    if (name.front() == ' ' || name.back() == ' ' || name.back() == '.')
        return false;

    const std::string bad = "\\/:*?\"<>|";

    for (char c : name) {
        if ((unsigned char)c < 32 || bad.find(c) != std::string::npos)
            return false;
    }

    return lowerCase(name) != Profiles::GUEST;      // that name is taken by guest mode
}


std::vector<std::string> Profiles::list() const
{
    std::vector<std::string> names;
    std::error_code problem;

    for (fs::directory_iterator it(AppPaths::profilesFolder(), problem);
        !problem && it != fs::directory_iterator(); it.increment(problem)) {

        std::error_code e;

        if (!it->is_directory(e))
            continue;

        std::string name;

        try {
            name = it->path().filename().string();
        }
        catch (const std::system_error&) {
            continue;       // a folder name that plain text cannot hold
        }

        if (lowerCase(name) != GUEST)
            names.push_back(name);
    }

    std::sort(names.begin(), names.end(), [](const std::string& a, const std::string& b) {
        return lowerCase(a) < lowerCase(b);
        });

    return names;
}


std::string Profiles::find(const std::string& name) const
{
    std::string wanted = lowerCase(name);

    for (const std::string& existing : list()) {
        if (lowerCase(existing) == wanted)
            return existing;
    }

    return "";
}


bool Profiles::exists(const std::string& name) const
{
    return find(name) != "";
}


bool Profiles::create(const std::string& name)
{
    if (!nameIsOk(name) || exists(name))
        return false;

    AppPaths::profileFolder(name);                  // this makes the folder
    return exists(name);
}


std::string Profiles::nextName() const
{
    for (int number = 1; ; number++) {
        std::string name = "Profile " + std::to_string(number);

        if (!exists(name))
            return name;
    }
}


std::string Profiles::picture(const std::string& name) const
{
    const char* endings[] = { ".png", ".jpg", ".jpeg" };
    std::string actual = find(name);

    if (actual == "")
        return "";

    for (const char* ending : endings) {
        fs::path p = fs::path(AppPaths::profileFolder(actual)) / (std::string("picture") + ending);
        std::error_code problem;

        if (fs::exists(p, problem))
            return p.string();
    }

    return "";
}


bool Profiles::setPicture(const std::string& name, const std::string& imagePath)
{
    std::string actual = find(name);
    std::error_code problem;

    if (actual == "" || !fs::is_regular_file(imagePath, problem))
        return false;

    std::string ending = lowerCase(fs::path(imagePath).extension().string());

    if (ending == ".jpeg")
        ending = ".jpg";

    if (ending != ".png" && ending != ".jpg")
        return false;               // slint shows png and jpg

    fs::path folder = AppPaths::profileFolder(actual);

    // an older picture of the other kind would win over the new one, so remove them all first
    for (const char* old : { ".png", ".jpg", ".jpeg" })
        fs::remove(folder / (std::string("picture") + old), problem);

    fs::copy_file(imagePath, folder / (std::string("picture") + ending),
        fs::copy_options::overwrite_existing, problem);

    return !problem;
}


void Profiles::resetGuest()
{
    std::error_code problem;

    fs::remove_all(fs::path(AppPaths::profilesFolder()) / GUEST, problem);
    AppPaths::profileFolder(GUEST);                 // a fresh, empty folder
}


// The first letter, in capitals. A letter outside plain English (é, Ж, 李) is
// several bytes long, so the whole letter is copied and only plain letters are made big.
std::string Profiles::initial(const std::string& name)
{
    if (name.empty())
        return "";

    unsigned char first = (unsigned char)name[0];
    size_t length = 1;

    if (first >= 0xF0) length = 4;
    else if (first >= 0xE0) length = 3;
    else if (first >= 0xC0) length = 2;

    std::string letter = name.substr(0, length);

    if (length == 1)
        letter[0] = (char)std::toupper(first);

    return letter;
}
