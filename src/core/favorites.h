#pragma once
#include<string>

class Favorite {
public:
	bool contains(const std::string& path) const;
	void add(const std::string& path);
	void remove(const std::string& path);

	int count() const;
	std::string	get(int n) const;
};