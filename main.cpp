#include "pch.h"
#include <iostream>
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <urlmon.h>
#include "auth.hpp"
#include <string>
#include "skStr.h"
#include <ctime>
#include "protection.h"
#include "Logs.h"

#include "Downloader.h"
#include "main_version.h" // inclure le fichier d'en-tête
#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma comment(lib, "urlmon.lib")
#include "UpdaterExe.h"
using namespace KeyAuth;

// 🔧 Activer ou désactiver KeyAuth ici
bool TekzaMode = false; // false pour ignorer toute sécurité
bool MichMode = false;
std::string versionapp = skCrypt("0.0.4").decrypt(); // CHANGE
std::string dll_version = skCrypt("0.0.0").decrypt(); // NOT CHANGE 
std::string name = skCrypt("NAME KEYAUTH").decrypt(); // CHANGE
std::string ownerid = skCrypt("ownerID").decrypt();  // CHANGE 
std::string version = skCrypt("1.0").decrypt();  // CHANGE
std::string url = skCrypt("https://keyauth.win/api/1.3/").decrypt();
std::string path = skCrypt("").decrypt();

api KeyAuthApp(name, ownerid, version, url, path);

std::string tm_to_readable_time(tm ctx) {
    char buffer[80];
    strftime(buffer, sizeof(buffer), skCrypt("%a %m/%d/%y %H:%M:%S %Z").decrypt(), &ctx);
    return std::string(buffer);
}

static std::time_t string_to_timet(std::string timestamp) {
    auto cv = strtol(timestamp.c_str(), NULL, 10);
    return (time_t)cv;
}

static std::tm timet_to_tm(time_t timestamp) {
    std::tm context;
    localtime_s(&context, &timestamp);
    return context;
}

std::string getSavedKey(const std::string& filePath) {
    std::ifstream keyFile(filePath);
    std::string key;
    if (keyFile.is_open()) {
        std::getline(keyFile, key);
        keyFile.close();
    }
    return key;
}

void saveKey(const std::string& filePath, const std::string& key) {
    std::ofstream keyFile(filePath);
    if (keyFile.is_open()) {
        keyFile << key;
        keyFile.close();
    }
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    if (!TekzaMode)
    {
        char appDataPath2[MAX_PATH];
        SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath2);
        std::string dirPath2 = std::string(appDataPath2) + skCrypt("\\Arès").decrypt(); 
        // CheckAndUpdateExe();
        CheckAndUpdate(dirPath2);
        FreeConsole();
        g_menu->initialize();
        g_menu->loop();
        return 0;
    }

  //  startSecurityThread();
    SetConsoleTitleA(skCrypt("AresLoader").decrypt()); 
    std::cout << skCrypt("\n\n Connecting...").decrypt() << std::endl;
    std::cout << "\n\n Version de l'injecteur : " + versionapp << std::endl;

    // 📁 Récupérer chemin AppData
    char appDataPath[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath);
    std::string dirPath = std::string(appDataPath) + skCrypt("\\Arès").decrypt();
    std::string keyFilePath = dirPath + skCrypt("\\license.key").decrypt();
    std::string uncryptPath = dirPath + skCrypt("\\.uncrypt").decrypt();

    if (!std::filesystem::exists(dirPath))
        std::filesystem::create_directory(dirPath);

    // 📂 Vérification override par fichier .uncrypt
    if (std::filesystem::exists(uncryptPath)) { // REMOVE FOR SAFETY
        std::ifstream uncryptFile(uncryptPath);
        std::string line;
        while (std::getline(uncryptFile, line)) {
            if (line.find(skCrypt("DisableKeyAuth").decrypt()) != std::string::npos) {
                std::cout << skCrypt("\n\n ⚠ KeyAuth disabled by .uncrypt override.\n").decrypt() << std::endl;
                MichMode = false;
            }
        }
    }

    if (MichMode) {
        KeyAuthApp.init();
        if (!KeyAuthApp.response.success)
        {
            std::cout << skCrypt("\n Status: ").decrypt() << KeyAuthApp.response.message << std::endl;
            Sleep(1500);
            return 1;
        }

        std::string key = getSavedKey(keyFilePath);
        if (key.empty()) {
            std::cout << skCrypt("\n Enter license key: ").decrypt() << std::endl;
            std::cin >> key;
            saveKey(keyFilePath, key);
        }
        else {
            std::cout << skCrypt("\n Using saved license key.").decrypt() << std::endl;
        }

        KeyAuthApp.license(key);

        if (!KeyAuthApp.response.success)
        {
            std::cout << skCrypt("\n Status: ").decrypt() << KeyAuthApp.response.message << std::endl;
            std::filesystem::remove(keyFilePath);
            Sleep(1500);
            return 1;
        }

        std::cout << skCrypt("\n User data:").decrypt();
        std::cout << skCrypt("\n Username: ").decrypt() << KeyAuthApp.user_data.username;
        std::cout << skCrypt("\n IP address: ").decrypt() << KeyAuthApp.user_data.ip;
        std::cout << skCrypt("\n Hardware-Id: ").decrypt() << KeyAuthApp.user_data.hwid;
        std::cout << skCrypt("\n Create date: ").decrypt() << tm_to_readable_time(timet_to_tm(string_to_timet(KeyAuthApp.user_data.createdate)));
        std::cout << skCrypt("\n Last login: ").decrypt() << tm_to_readable_time(timet_to_tm(string_to_timet(KeyAuthApp.user_data.lastlogin)));
        std::cout << skCrypt("\n Subscription(s): ").decrypt();

        for (int i = 0; i < KeyAuthApp.user_data.subscriptions.size(); i++) {
            auto sub = KeyAuthApp.user_data.subscriptions.at(i);
            std::cout << skCrypt("\n name: ").decrypt() << sub.name;
            std::cout << skCrypt(" : expiry: ").decrypt() << tm_to_readable_time(timet_to_tm(string_to_timet(sub.expiry)));
        }

        std::cout << skCrypt("\n\n Status: ").decrypt() << KeyAuthApp.response.message << std::endl;
    }
   // CheckAndUpdateExe();
    CheckAndUpdate(dirPath);

    FreeConsole();
    g_menu->initialize();
    g_menu->loop();

    return 0;
}
