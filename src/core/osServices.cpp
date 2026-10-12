#ifdef _WIN32
#define NOMINMAX                          // keeps windows.h from breaking std::min and std::max
#endif
#include "osServices.h"
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <commdlg.h>
#endif

namespace fs = std::filesystem;

std::string Os::chooseFolder()
{
#ifdef _WIN32
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);    // fine if it was already done

    BROWSEINFOW info = {};
    info.lpszTitle = L"Choose a folder with music";
    info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    PIDLIST_ABSOLUTE chosen = SHBrowseForFolderW(&info);
    if (chosen == nullptr)
        return "";

    wchar_t path[MAX_PATH] = {};
    bool ok = SHGetPathFromIDListW(chosen, path) != FALSE;
    CoTaskMemFree(chosen);

    return ok ? fs::path(path).string() : "";
#else
    return "";
#endif
}

std::string Os::chooseImage()
{
#ifdef _WIN32
    wchar_t file[MAX_PATH] = {};

    OPENFILENAMEW dialog = {};
    dialog.lStructSize = sizeof dialog;
    dialog.lpstrFilter = L"Pictures (*.jpg;*.jpeg;*.png)\0*.jpg;*.jpeg;*.png\0";
    dialog.lpstrFile = file;
    dialog.nMaxFile = MAX_PATH;
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    return GetOpenFileNameW(&dialog) ? fs::path(file).string() : "";
#else
    return "";
#endif
}

void Os::showInFolder(const std::string& path)
{
#ifdef _WIN32
    std::wstring arg = L"/select,\"" + fs::path(path).wstring() + L"\"";
    ShellExecuteW(nullptr, L"open", L"explorer.exe", arg.c_str(), nullptr, SW_SHOWNORMAL);
#else
    (void)path;
#endif
}
