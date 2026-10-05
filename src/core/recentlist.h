#pragma once

#include <string>

class RecentList
{
public:
    void add(const std::string& path);
    bool contains(const std::string& path) const;
    int count() const;
    std::string get(int n) const;
};