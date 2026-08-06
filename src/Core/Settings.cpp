#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Settings
{
private:
    std::filesystem::path configFile;
    json data;

public:
    Settings(std::filesystem::path path)
        : configFile(path)
    {
        Load();
    }

    void Load()
    {
        if (std::filesystem::exists(configFile))
        {
            std::ifstream file(configFile);
            file >> data;
        }
    }

    void Save()
    {
        std::filesystem::create_directories(configFile.parent_path());

        std::ofstream file(configFile);
        file << data.dump(4);
    }

    std::filesystem::path GetAddonDirectory()
    {
        return data.value("directory", "");
    }

    void SetAddonDirectory(std::filesystem::path path)
    {
        data["directory"] = path.string();
        Save();
    }
};