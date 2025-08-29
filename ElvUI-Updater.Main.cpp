#define CURL_STATICLIB
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

// Filesystem handling
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <sstream>
#include <regex>
#include <cstdlib>
#include <windows.h>

using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace std;

struct Settings {
    string addonFolderPath;
};

struct ApiResponse {
    string slug;
    string url;
};

const string battleNetAggregateFilePath = "C:\\ProgramData\\Battle.net\\agent\\aggregate.json";

void from_json(const json& j, ApiResponse& s) {
    j.at("url").get_to(s.url);
    j.at("slug").get_to(s.slug);
}

void to_json(json& j, const Settings& s) {
    j = json{ {"addonFolderPath", s.addonFolderPath} };
}

bool ReadSettings(Settings& settings) {
    ifstream inFile("settings.json");
    if (!inFile.is_open()) {
        cerr << "Failed to open settings.json" << endl;
        return false;
    }

    try {
        json settingsJson;
        inFile >> settingsJson;
        settings.addonFolderPath = settingsJson["addonFolderPath"].get<string>();
        return true;
    }
    catch (const exception& e) {
        cerr << "Error reading settings: " << e.what() << endl;
        return false;
    }
}

void InitSettings() {
    ifstream aggregateFile(battleNetAggregateFilePath);
    if (!aggregateFile.is_open()) {
        cerr << "Failed to open file: " << battleNetAggregateFilePath << endl;
        return;
    }

    try {
        json aggregateJson = json::parse(aggregateFile);
        aggregateFile.close();

        string wowFolder;
        for (const auto& game : aggregateJson["installed"]) {
            if (game.contains("product_id") && game["product_id"] == "wow") {
                string iconPath = game["icon_path"];
                fs::path p(iconPath);
                p = p.parent_path();
                wowFolder = p.string() + "\\_retail_\\Interface\\AddOns";
                break;
            }
        }

        if (wowFolder.empty()) {
            cerr << "World of Warcraft installation not found." << endl;
            return;
        }

        Settings settings;
        settings.addonFolderPath = wowFolder;

        json settingsJson = settings;
        ofstream settingsFile("settings.json");
        if (!settingsFile.is_open()) {
            cerr << "Failed to create settings.json file." << endl;
            return;
        }

        settingsFile << settingsJson.dump(4);
        settingsFile.close();

        cout << "Settings created with addon folder path: " << settings.addonFolderPath << endl;
    }
    catch (const exception& e) {
        cerr << "Error processing settings: " << e.what() << endl;
    }
}

bool DownloadFile(const string& url, const string& outputPath) {
    try {
        curlpp::Easy request;
        request.setOpt<cURLpp::Options::Url>(url);

        ofstream outFile(outputPath, ios::binary);
        if (!outFile.is_open()) {
            cerr << "Failed to create output file: " << outputPath << endl;
            return false;
        }

        curlpp::options::WriteStream ws(&outFile);
        request.setOpt(ws);
        request.perform();

        return true;
    }
    catch (const exception& e) {
        cerr << "Download failed: " << e.what() << endl;
        return false;
    }
}

void UpdateAddon() {
    cout << "Updating ElvUI..." << endl;

    Settings settings;
    if (!ReadSettings(settings)) {
        return;
    }

    string downloadUrl = "https://api.tukui.org/v1/addon/elvui";

    try {
        // Get download URL from API
        curlpp::Easy request;
        request.setOpt<cURLpp::Options::Url>(downloadUrl);

        ostringstream os;
        curlpp::options::WriteStream ws(&os);
        request.setOpt(ws);
        request.perform();

        json j = json::parse(os.str());
        ApiResponse response = j.get<ApiResponse>();

        string zipFilePath = "ElvUI.zip";
        if (!DownloadFile(response.url, zipFilePath)) {
            return;
        }

        cout << "ElvUI downloaded to " << zipFilePath << endl;

        string command = "powershell -command \"Expand-Archive -Path '" +
            zipFilePath + "' -DestinationPath '.' -Force\"";
        int result = system(command.c_str());

        if (result != 0) {
            cerr << "Extraction failed with code: " << result << endl;
            return;
        }

        string elvUIFolderPath = settings.addonFolderPath + "\\ElvUI";
        fs::create_directories(elvUIFolderPath);

        fs::path sourceDir = fs::current_path() / "ElvUI";
        if (fs::exists(sourceDir)) {
            fs::copy(sourceDir, elvUIFolderPath,
                fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            cout << "ElvUI updated successfully." << endl;

            fs::remove_all(sourceDir);
            fs::remove(zipFilePath);
        }
        else {
            cerr << "Extracted ElvUI folder not found." << endl;
        }

    }
    catch (const exception& e) {
        cerr << "Error during update: " << e.what() << endl;
    }
}

bool ElvUIIsUpToDate() {
    Settings settings;
    if (!ReadSettings(settings)) {
        return false;
    }

    string changelog = "https://raw.githubusercontent.com/tukui-org/ElvUI/refs/heads/main/CHANGELOG.md";

    try {
        curlpp::Easy request;
        request.setOpt<cURLpp::Options::Url>(changelog);

        ostringstream responseStream;
        request.setOpt(cURLpp::options::WriteStream(&responseStream));
        request.perform();

        string response = responseStream.str();
        istringstream iss(response);
        string firstLine;
        getline(iss, firstLine);

        regex versionRegex(R"(### Version ([0-9]+\.[0-9]+))");
        smatch match;
        string apiVersion;

        if (regex_search(firstLine, match, versionRegex)) {
            apiVersion = match[1];
        }
        else {
            cout << "Version not found in changelog." << endl;
            return false;
        }

        string tocPath = settings.addonFolderPath + "\\ElvUI\\ElvUI_Mainline.toc";
        ifstream versionFile(tocPath);

        if (!versionFile.is_open()) {
            cerr << "Failed to open ElvUI toc file: " << tocPath << endl;
            return false;
        }

        string line;
        string localVersion;
        while (getline(versionFile, line)) {
            if (line.find("## Version:") != string::npos) {
                size_t versionPos = line.find("v");
                if (versionPos != string::npos) {
                    localVersion = line.substr(versionPos + 1);
                    localVersion.erase(0, localVersion.find_first_not_of(" \t"));
                    localVersion.erase(localVersion.find_last_not_of(" \t") + 1);
                    break;
                }
            }
        }

        if (localVersion.empty()) {
            cerr << "Could not find version in toc file." << endl;
            return false;
        }

        return localVersion == apiVersion;

    }
    catch (const exception& e) {
        cerr << "Error checking version: " << e.what() << endl;
        return false;
    }
}

int main() {
    curlpp::initialize();

    try {
        if (!fs::exists("settings.json")) {
            InitSettings();
        }

        if (!ElvUIIsUpToDate()) {
            UpdateAddon();
        }
        else {
            cout << "ElvUI is already up to date." << endl;
        }
    }
    catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }

    curlpp::terminate();
    system("pause");
    return 0;
}