#include "Settings.hpp"
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <stdexcept>

using json = nlohmann::json;

namespace {
    std::string getEnvOrThrow(const char* name)
    {
        const char* val = std::getenv(name);
        if (!val)
            throw std::runtime_error(std::string("Environment variable not set: ") + name);
        return val;
    }
}

Settings::Settings()
{
#ifdef _WIN32
    settingsPath = getEnvOrThrow("APPDATA") + "/ElvUI-Updater/settings.json";
#elif __APPLE__
    settingsPath = getEnvOrThrow("HOME") + "/Library/Application Support/ElvUI-Updater/settings.json";
#else
    settingsPath = getEnvOrThrow("HOME") + "/.config/ElvUI-Updater/settings.json";
#endif
}

void Settings::load()
{
    if (!std::filesystem::exists(settingsPath))
        return;

    std::ifstream file(settingsPath);
    if (!file)
        return;

    json data;
    try
    {
        file >> data;
    }
    catch (const nlohmann::json::parse_error&)
    {
        // Corrupt or empty settings file — fall back to defaults.
        return;
    }

    wowDirectory = data.value("wow_directory", "");
}

void Settings::save() const
{
    std::filesystem::path path(settingsPath);
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec)
        throw std::runtime_error(std::string("Failed to create settings directory: ") + ec.message());

    json data;
    data["wow_directory"] = wowDirectory;

    std::ofstream file(settingsPath);
    if (!file)
        throw std::runtime_error(std::string("Failed to open settings file for writing: ") + settingsPath);

    file << data.dump(4);
}

const std::string& Settings::getWowDirectory() const
{
    return wowDirectory;
}

void Settings::setWowDirectory(const std::string& path)
{
    wowDirectory = path;
}

bool Settings::isValidWowDirectory(const std::string& path)
{
    if (path.empty())
        return false;

    std::filesystem::path p(path);
    return std::filesystem::exists(p / "Interface" / "AddOns");
}