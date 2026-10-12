#pragma once
#include <string>

// Things only the operating system can do. Windows today; on other systems they do nothing,
// so the rest of the program does not need to know which system it runs on.
namespace Os
{
    // the "pick a folder" window. "" if the user cancels
    std::string chooseFolder();

    // the "pick a picture" window (jpg / png). "" if the user cancels
    std::string chooseImage();

    // opens the file explorer with this file selected
    void showInFolder(const std::string& path);
}
