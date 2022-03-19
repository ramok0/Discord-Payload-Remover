#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include "nlohmann/json.hpp"

int get_directories_with_search_term(std::string path, std::vector<std::string>* buffer)
{
    for (auto& p : std::filesystem::directory_iterator(path)) {
        if (p.is_directory()) {
            std::string path = p.path().string();
            if (path.find("iscord") != std::string::npos) {
                buffer->push_back(path);
            }
        }

    }
    return 0;
}

int get_recurcive_directories_and_filter(std::string path, std::vector<std::string>* buffer) {
    for (auto& p : std::filesystem::recursive_directory_iterator(path)) {
        if (p.is_directory()) {
            if (p.path().string().find("discord_desktop_core") != std::string::npos) {
                if (p.path().parent_path().filename().string().find("discord_desktop_core") != std::string::npos) {
                    buffer->push_back(p.path().string());
                }
            }
        }
    }

    return 0;
}

std::vector<std::string> getDiscordPaths(char* localAppdata) {
    std::vector<std::string> buffer = std::vector<std::string>();

    std::vector<std::string> DiscordDesktopCore = std::vector<std::string>();
    get_directories_with_search_term(std::string(localAppdata), &buffer);
    for (std::string path : buffer) {
        get_recurcive_directories_and_filter(path, &DiscordDesktopCore);
    }
    return DiscordDesktopCore;
}

void analysePackageJson(std::filesystem::path packageJsonPath) {
    if (std::filesystem::exists(packageJsonPath)) {
        std::ifstream packageJsonReader(packageJsonPath);
        if (packageJsonReader.is_open()) {
            nlohmann::json json;
            packageJsonReader >> json;
            if (json["main"] != nlohmann::detail::value_t::null && json["main"] != "index.js") {
                printf("Detected something sus\n");
                std::filesystem::path susPath = std::filesystem::path(json["main"]);
                if (std::filesystem::exists(susPath)) {
                    std::filesystem::remove(susPath);
                }
                else {
                    json["main"] = "index.js";
                    packageJsonReader.close();
                    std::ofstream packageJsonWriter(packageJsonPath);
                    if (packageJsonWriter.is_open()) {
                        packageJsonWriter << json;
                        packageJsonWriter.close();
                    }
                }
            }
        }
    }
}

void anaylseIndexJs(std::filesystem::path indexJsPath) {

}

int main()
{
    char* buffer;
    size_t size = sizeof(buffer);
    _dupenv_s(&buffer, &size, "LOCALAPPDATA");

    printf("Found localappdata : %s\n", buffer);

    std::vector<std::string> paths = getDiscordPaths(buffer);
    for (auto path : paths) {
        printf("Found discord path : %s\n", path.c_str());
        printf("Analyzing folder...\n");
        std::filesystem::path discordPath = std::filesystem::path(path);
        std::filesystem::path packageJsonPath = std::filesystem::path(discordPath / "package.json");
        std::filesystem::path indexJsPath = std::filesystem::path(discordPath / "index.js");
        analysePackageJson(packageJsonPath);
        anaylseIndexJs(indexJsPath);
    }


}