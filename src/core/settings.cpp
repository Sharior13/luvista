#include "settings.h"
#include "appPaths.h"
#include <cstdlib>
#include <fstream>

static std::string settingsFile()
{
    return AppPaths::activeFolder() + "/settings.dat";
}


std::vector<std::string> Settings::languages()
{
    return { "English" };
}


std::vector<std::string> Settings::themes()
{
    return { "Default" };
}


void Settings::load()
{
    autoplayOn = true;
    languageIndex = 0;
    themeIndex = 0;

    std::ifstream in(settingsFile());
    std::string line;

    while (std::getline(in, line)) {
        size_t eq = line.find('=');

        if (eq == std::string::npos)
            continue;

        std::string key = line.substr(0, eq);
        int value = std::atoi(line.substr(eq + 1).c_str());

        if (key == "autoplay") autoplayOn = (value != 0);
        else if (key == "language") languageIndex = value;
        else if (key == "theme") themeIndex = value;
    }

    if (languageIndex < 0 || languageIndex >= (int)languages().size()) languageIndex = 0;
    if (themeIndex < 0 || themeIndex >= (int)themes().size()) themeIndex = 0;
}


void Settings::save() const
{
    std::ofstream out(settingsFile(), std::ios::trunc);

    out << "autoplay=" << (autoplayOn ? 1 : 0) << "\n";
    out << "language=" << languageIndex << "\n";
    out << "theme=" << themeIndex << "\n";
}


bool Settings::autoplay() const { return autoplayOn; }
int Settings::language() const { return languageIndex; }
int Settings::theme() const { return themeIndex; }


void Settings::setAutoplay(bool on)
{
    autoplayOn = on;
    save();
}


void Settings::setLanguage(int index)
{
    if (index >= 0 && index < (int)languages().size()) {
        languageIndex = index;
        save();
    }
}


void Settings::setTheme(int index)
{
    if (index >= 0 && index < (int)themes().size()) {
        themeIndex = index;
        save();
    }
}
