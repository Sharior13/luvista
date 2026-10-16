#include "profiles.h"
#include "appPaths.h"
#include "credentials.h"
#include <fstream>
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


// ---- accounts -------------------------------------------------------------------------------------
// <profile folder>/account.dat holds lines like "email=a@b.c", "password=pbkdf2$...", "remember=0"

namespace {

struct Account {
    std::string email;
    std::string password;       // the fingerprint, never the password itself
    bool remember = true;       // a profile without account.dat is shown on the login screen
};

fs::path accountFile(const std::string& profile)
{
    return fs::path(AppPaths::profileFolder(profile)) / "account.dat";
}

Account readAccount(const std::string& profile)
{
    Account account;
    std::ifstream in(accountFile(profile));
    std::string line;

    while (std::getline(in, line)) {
        size_t eq = line.find('=');

        if (eq == std::string::npos)
            continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        if (key == "email") account.email = value;
        else if (key == "password") account.password = value;
        else if (key == "remember") account.remember = (value != "0");
    }

    return account;
}

bool writeAccount(const std::string& profile, const Account& account)
{
    std::ofstream out(accountFile(profile), std::ios::trunc);

    out << "email=" << account.email << "\n";
    out << "password=" << account.password << "\n";
    out << "remember=" << (account.remember ? "1" : "0") << "\n";
    return (bool)out;
}

std::string trimmed(const std::string& text)
{
    size_t a = text.find_first_not_of(" \t");

    if (a == std::string::npos)
        return "";

    return text.substr(a, text.find_last_not_of(" \t") - a + 1);
}

bool emailLooksOk(const std::string& email)
{
    size_t at = email.find('@');

    if (at == std::string::npos || at == 0 || email.find('@', at + 1) != std::string::npos)
        return false;

    size_t dot = email.find('.', at);

    return dot != std::string::npos && dot > at + 1 && dot + 1 < email.size()
        && email.find(' ') == std::string::npos;
}

}   // namespace


// Every profile gets a card. A profile that did not want to be remembered still shows up, but its card asks for the password.
std::vector<std::string> Profiles::shown() const
{
    return list();
}


bool Profiles::needsPassword(const std::string& name) const
{
    std::string actual = find(name);

    if (actual == "")
        return false;

    Account account = readAccount(actual);
    return !account.remember && !account.password.empty();
}


std::string Profiles::email(const std::string& name) const
{
    std::string actual = find(name);

    return actual == "" ? "" : readAccount(actual).email;
}


bool Profiles::hasPassword(const std::string& name) const
{
    std::string actual = find(name);

    return actual != "" && !readAccount(actual).password.empty();
}


// Who owns this email? ("" = nobody). `except` is a profile that does not count (the one being edited).
static std::string ownerOfEmail(const Profiles& profiles, const std::string& email, const std::string& except)
{
    for (const std::string& name : profiles.list()) {
        if (name != except && !email.empty() && lowerCase(profiles.email(name)) == lowerCase(email))
            return name;
    }

    return "";
}


bool Profiles::createAccount(const std::string& rawName, const std::string& rawEmail, const std::string& password,
                             bool remember, std::string& error)
{
    std::string name = trimmed(rawName);
    std::string email = trimmed(rawEmail);

    if (!nameIsOk(name)) {
        error = "That username cannot be used";
        return false;
    }
    if (exists(name)) {
        error = "That username is already used";
        return false;
    }
    if (!emailLooksOk(email)) {
        error = "That email address does not look right";
        return false;
    }
    if (ownerOfEmail(*this, email, "") != "") {
        error = "That email is already used";
        return false;
    }
    if (password.size() < 4) {
        error = "The password needs at least 4 characters";
        return false;
    }

    if (!create(name)) {
        error = "The profile could not be made";
        return false;
    }

    Account account;
    account.email = email;
    account.password = Credentials::makeFingerprint(password);
    account.remember = remember;

    if (!writeAccount(find(name), account)) {
        error = "The profile could not be saved";
        return false;
    }

    return true;
}


std::string Profiles::signIn(const std::string& userOrEmail, const std::string& password, std::string& error) const
{
    std::string who = trimmed(userOrEmail);
    std::string name = find(who);

    if (name == "")
        name = ownerOfEmail(*this, who, "");

    Account account = name == "" ? Account() : readAccount(name);

    // the same sentence for "no such profile" and "wrong password", so nobody can find out which names exist
    if (name == "" || account.password.empty() || !Credentials::matches(password, account.password)) {
        error = "Wrong username or password";
        return "";
    }

    return name;
}


bool Profiles::updateAccount(std::string& name, const std::string& rawNewName, const std::string& rawEmail,
                             const std::string& currentPassword, const std::string& newPassword, std::string& error)
{
    std::string oldName = find(name);

    if (oldName == "") {
        error = "This profile cannot be edited";
        return false;
    }

    std::string newName = trimmed(rawNewName);
    std::string email = trimmed(rawEmail);
    Account account = readAccount(oldName);

    if (!nameIsOk(newName)) {
        error = "That username cannot be used";
        return false;
    }
    if (lowerCase(newName) != lowerCase(oldName) && exists(newName)) {
        error = "That username is already used";
        return false;
    }
    if (!email.empty() && !emailLooksOk(email)) {
        error = "That email address does not look right";
        return false;
    }
    if (ownerOfEmail(*this, email, oldName) != "") {
        error = "That email is already used";
        return false;
    }

    if (!newPassword.empty()) {
        if (!account.password.empty() && !Credentials::matches(currentPassword, account.password)) {
            error = "The current password is wrong";
            return false;
        }
        if (newPassword.size() < 4) {
            error = "The password needs at least 4 characters";
            return false;
        }
        account.password = Credentials::makeFingerprint(newPassword);
    }

    account.email = email;

    if (newName != oldName) {                               // the folder carries the name
        std::error_code problem;
        fs::path from = AppPaths::profileFolder(oldName);
        fs::path to = fs::path(AppPaths::profilesFolder()) / newName;

        fs::rename(from, to, problem);

        if (problem) {
            error = "The username could not be changed";
            return false;
        }
    }

    if (!writeAccount(newName, account)) {
        error = "The changes could not be saved";
        return false;
    }

    name = newName;
    return true;
}
