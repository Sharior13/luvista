#include "favorites.h"
#include "appPaths.h"
#include <fstream>
#include <cstdlib>

// Where the favorites notebook lives
static std::string favoritesFile()
{
    return AppPaths::profileFolder("default") + "/favorites.dat";
}

bool Favorite::contains(int id) const {
    std::ifstream File(favoritesFile());
    std::string line;

    while (std::getline(File, line)) {
        if (std::atoi(line.c_str()) == id)
            return true;
    }
    return false;
}

void Favorite::add(int id) {

    if (contains(id))  // stops duplicates
        return;

    std::ofstream File(favoritesFile(), std::ios::app);   // append mode: adds at the end
    File << id << "\n";
}

void Favorite::remove(int id) {
    std::ifstream File(favoritesFile());   // open to read
    std::string line;
    std::string kept = "";

    while (std::getline(File, line)) {
        if (std::atoi(line.c_str()) != id)
            kept += line + "\n";          // keep every line except the song
    }
    File.close();                         // finish reading first

    std::ofstream outFile(favoritesFile());   // now open to write (this erases the old content)
    outFile << kept;
}

void Favorite::clear() {
    std::ofstream File(favoritesFile());   // opening to write erases everything
}

int Favorite::count() const {
    std::ifstream File(favoritesFile());
    std::string line;
    int lines = 0;

    while (std::getline(File, line)) {
        lines++;
    }

    return lines;
}

int Favorite::get(int n) const {
    std::ifstream File(favoritesFile());
    std::string line;
    int lineNumber = 0;

    while (std::getline(File, line)) {
        lineNumber++;

        if (lineNumber == n)
            return std::atoi(line.c_str());   // this is the line we wanted
    }

    return 0;                                 // there is no line number n
}
