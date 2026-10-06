#include "favorites.h"
#include <fstream>
#include <string>

static const char* FAVORITES_FILE = "favorite-index.dat";

bool Favorite::contains(const std::string& path) const {
    std::ifstream File(FAVORITES_FILE); // Opens the file automatically
    std::string line;
    
    while (std::getline(File, line)) {
        if (line == path) 
            return true;
    }
    return false;
}

void Favorite::add(const std::string& path) {

    if (contains(path))  // stops duplicates
        return;

    std::ofstream File(FAVORITES_FILE, std::ios::app);   // append mode: adds at the end
    File << path << "\n";
}

void Favorite::remove(const std::string& path) {
    std::ifstream File(FAVORITES_FILE);   // open to read
    std::string line;
    std::string kept = "";

    while (std::getline(File, line)) {
        if (line != path)
            kept += line + "\n";          // keep every line except the song
    }
    File.close();                         // finish reading first

    std::ofstream outFile(FAVORITES_FILE);   // now open to write (this erases the old content)
    outFile << kept;
}

int Favorite::count() const {
    std::ifstream File(FAVORITES_FILE);   // open the file to read
    std::string line;                     // holds one line at a time
    int lines = 0;                        // the counter

    while (std::getline(File, line)) {
        lines++;
    }

    return lines;
}

std::string Favorite::get(int n) const {
    std::ifstream File(FAVORITES_FILE);   // open the file to read
    std::string line;                     // holds one line at a time
    int lineNumber = 0;                   // which line we are on

    while (std::getline(File, line)) {    // read the text into line
        lineNumber++;                     // count it

        if (lineNumber == n)
            return line;                  // this is the line we wanted
    }

    return "";                            // there is no line number n
}