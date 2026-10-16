#include "launchSettings.h"
#include "appPaths.h"
#include <fstream>

LaunchSettings::LaunchSettings()
{
    load();
}


// launch.dat: lines like "open=1", "guest=1" or "last=Anna"
void LaunchSettings::load()
{
    std::ifstream file(AppPaths::launchFile());
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        size_t equals = line.find('=');

        if (equals == std::string::npos)
            continue;

        std::string key = line.substr(0, equals);
        std::string value = line.substr(equals + 1);

        if (key == "open") open = (value != "0");
        else if (key == "guest") { guest = (value == "1"); any = any || guest; }
        else if (key == "last" && !value.empty()) { last = value; any = true; }
    }
}


void LaunchSettings::save() const
{
    std::ofstream file(AppPaths::launchFile(), std::ios::trunc);

    file << "open=" << (open ? "1" : "0") << "\n";

    if (guest)
        file << "guest=1\n";
    else if (!last.empty())
        file << "last=" << last << "\n";
}


bool LaunchSettings::openLast() const { return open; }
bool LaunchSettings::hasLast() const { return any; }
bool LaunchSettings::lastWasGuest() const { return guest; }
std::string LaunchSettings::lastProfile() const { return last; }


void LaunchSettings::setOpenLast(bool on)
{
    open = on;
    save();
}


void LaunchSettings::remember(const std::string& profile, bool isGuest)
{
    guest = isGuest;
    last = isGuest ? "" : profile;
    any = isGuest || !profile.empty();
    save();
}


void LaunchSettings::forget()
{
    guest = false;
    last = "";
    any = false;
    save();
}
