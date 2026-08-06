#include "Settings.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <zip.h>

namespace fs = std::filesystem;

class TukUIApiResponse
{
    public:
        std::string url;
        std::string version;
        std::string slug;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TukUIApiResponse, url, version, slug)
};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* user)
{
    size_t total = size * nmemb;

    auto* response = static_cast<std::string*>(user);

    response->append( static_cast<char*>(contents), total);

    return total;
}

std::string CleanVersion(std::string version)
{
    size_t start = version.find_first_not_of(" \t\r\n");
    size_t end = version.find_last_not_of(" \t\r\n");

    if (start == std::string::npos)
        return "";

    version = version.substr(start, end - start + 1);

    if (!version.empty() && version[0] == 'v')
        version.erase(0, 1);

    return version;
}

bool HttpGet(const std::string& url, std::string& output)
{
    CURL* raw = curl_easy_init();

    if (!raw)
        return false;

    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(raw, curl_easy_cleanup);

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &output);
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "ElvUI-Updater/1.0");

    CURLcode result = curl_easy_perform(curl.get());

    if (result != CURLE_OK)
    {
        std::cerr << curl_easy_strerror(result) << '\n';
        return false;
    }

    long status = 0;

    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status);

    if (status < 200 || status >= 300)
    {
        std::cerr << "HTTP error: " << status << '\n';
        return false;
    }

    return true;
}
bool DownloadFile(const std::string& url, const fs::path& destination)
{
    CURL* raw = curl_easy_init();

    if (!raw)
        return false;

    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(raw, curl_easy_cleanup);

    FILE* file = fopen(destination.string().c_str(), "wb");

    if (!file)
        return false;

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 60L);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "ElvUI-Updater/1.0");

    CURLcode result = curl_easy_perform(curl.get());

    fclose(file);

    if (result != CURLE_OK)
    {
        fs::remove(destination);
        return false;
    }

    long status = 0;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status);

    if (status < 200 || status >= 300)
    {
        fs::remove(destination);
        return false;
    }

    return true;
}


bool RemoveOldElvUI(const std::string& wowDir)
{
    fs::path addons = fs::path(wowDir) / "_retail_" / "Interface" / "AddOns";

    std::vector<std::string> folders =
    {
        "ElvUI",
        "ElvUI_Options",
        "ElvUI_Libraries"
    };

    try
    {
        for (const auto& folder : folders)
        {
            fs::path target = addons / folder;

            if (fs::exists(target))
            {
                fs::remove_all(target);
                std::cout << "Removed: " << target << '\n';
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}


bool ExtractZip(const std::string& zipPath, const std::string& destination)
{
    int error = 0;

    zip_t* archive = zip_open(zipPath.c_str(), ZIP_RDONLY, &error);

    if (!archive)
        return false;

    try
    {
        fs::path base = fs::weakly_canonical(destination);

        zip_int64_t count = zip_get_num_entries(archive, 0);

        for (zip_int64_t i = 0; i < count; i++)
        {
            const char* rawName = zip_get_name(archive, i, 0);

            if (!rawName)
                continue;

            fs::path output = fs::path(destination)/ rawName;

            fs::path resolved = fs::weakly_canonical(output.parent_path());

            if (resolved.string().rfind(base.string(), 0) != 0)
            {
                std::cerr << "Blocked zip entry: " << rawName << '\n';

                continue;
            }

            if (rawName[strlen(rawName) - 1] == '/')
            {
                fs::create_directories(output);
                continue;
            }

            fs::create_directories(output.parent_path());

            zip_file_t* file = zip_fopen_index(archive, i, 0);

            if (!file)
                continue;

            std::ofstream out(output, std::ios::binary);

            char buffer[4096];
            zip_int64_t bytes;

            while ((bytes = zip_fread(file, buffer, sizeof(buffer))) > 0)
            {
                out.write(buffer, bytes);
            }

            zip_fclose(file);
        }

        zip_close(archive);
    }
    catch (...)
    {
        zip_close(archive);
        return false;
    }

    fs::remove(zipPath);

    return true;
}
bool InstallElvUI(
    const std::string& url,
    const std::string& wowDir)
{
    fs::path addons =
        fs::path(wowDir)
        / "_retail_"
        / "Interface"
        / "AddOns";

    fs::create_directories(addons);

    fs::path zip = addons / "ElvUI.zip";

    std::cout << "Downloading ElvUI...\n";

    if (!DownloadFile(url, zip))
    {
        std::cerr << "Download failed\n";

        return false;
    }

    if (!RemoveOldElvUI(wowDir))
        return false;

    if (!ExtractZip(zip.string(), addons.string()))
    {
        std::cerr << "Extraction failed\n";

        return false;
    }

    std::cout << "ElvUI installed successfully\n";

    return true;
}


int main()
{
    Settings settings;

    settings.load();

    if (!Settings::isValidWowDirectory(settings.getWowDirectory()))
    {
        std::string input;

        while (true)
        {
            std::cout << "Enter WoW directory: ";

            std::getline(std::cin, input);

            if (Settings::isValidWowDirectory(input))
            {
                settings.setWowDirectory(input);
                settings.save();
                break;
            }
        }
    }

    std::string response;

    if (!HttpGet("https://api.tukui.org/v1/addon/elvui", response))
    {
        return 1;
    }

    TukUIApiResponse api;

    try
    {
        api = nlohmann::json::parse(response).get<TukUIApiResponse>();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Invalid API response: " << e.what() << '\n';
        return 1;
    }

    api.version = CleanVersion(api.version);

    std::cout << "Latest ElvUI: " << api.version << '\n';


    fs::path toc =
        fs::path(settings.getWowDirectory())
        / "_retail_"
        / "Interface"
        / "AddOns"
        / "ElvUI"
        / "ElvUI_Mainline.toc";


    std::ifstream file(toc);

    if (!file)
    {
        std::cout << "ElvUI not installed\n";

        return InstallElvUI(api.url, settings.getWowDirectory())
        ? 0
        : 1;
    }

    std::string current;
    bool found = false;

    while (std::getline(file, current))
    {
        if (current.find("## Version:") != std::string::npos)
        {
            current = current.substr(current.find(":") + 1);
            current = CleanVersion(current);

            found = true;
            break;
        }
    }


    if (!found)
    {
        std::cerr << "Version missing from TOC\n";

        return 1;
    }


    std::cout
        << "Installed: " << current << "\n";

    std::cout << "API: " << api.version << "\n";


    if (current == api.version)
    {
        std::cout << "ElvUI is up to date\n";
        return 0;
    }


    std::cout << "Updating ElvUI...\n";


    return InstallElvUI( api.url, settings.getWowDirectory()) ? 0 : 1;
}