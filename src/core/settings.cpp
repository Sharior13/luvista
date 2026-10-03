#include "settings.h"
#include <fstream>

using namespace std;

static const char* SETTINGS_FILE = "settings.txt";

string loadFolderSetting()
{
    ifstream in(SETTINGS_FILE);
    string line;

    getline(in, line);
    return line;
}

void saveFolderSetting(const string& folder)
{
    ofstream out(SETTINGS_FILE);
    out << folder << "\n";
}
