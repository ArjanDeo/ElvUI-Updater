#pragma once

#include <string>

class Settings
{
public:
    Settings();

    void load();
    void save() const;

    const std::string& getWowDirectory() const;
    void setWowDirectory(const std::string& path);

    static bool isValidWowDirectory(const std::string& path);

private:
    std::string settingsPath;
    std::string wowDirectory;
};