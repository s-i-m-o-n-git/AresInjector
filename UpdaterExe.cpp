#include "UpdaterExe.h"
#include <windows.h>
#include <wininet.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "skStr.h"
#include "main_version.h" // contient extern std::string versionapp;

#pragma comment(lib, "wininet.lib")

const std::string githubUser = "s-i-m-o-n-git"; // CHANGE
const std::string githubRepo = "D"; // CHANGE 
const std::string githubBranch = "main"; // CHANGE

std::string makeGitHubRawUrl2(const std::string& fileName) {
    return "https://raw.githubusercontent.com/" + githubUser + "/" + githubRepo + "/" + githubBranch + "/" + fileName;
}

std::string getRemoteTextFile(const std::string& url) {
    HINTERNET hInternet = InternetOpenA("UpdateCheck", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return "";

    HINTERNET hFile = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hFile) {
        InternetCloseHandle(hInternet);
        return "";
    }

    char buffer[1024];
    DWORD bytesRead;
    std::string content;

    while (InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        content.append(buffer, bytesRead);
    }

    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);

    content.erase(content.find_last_not_of(" \n\r\t") + 1);
    return content;
}

bool DownloadFileDirect(const std::string& url, const std::string& outputPath) {
    HINTERNET hInternet = InternetOpenA("Updater", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hFile = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hFile) {
        InternetCloseHandle(hInternet);
        return false;
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) {
        InternetCloseHandle(hFile);
        InternetCloseHandle(hInternet);
        return false;
    }

    char buffer[4096];
    DWORD bytesRead;

    while (InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        outFile.write(buffer, bytesRead);
    }

    outFile.close();
    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);

    return true;
}

void CheckAndUpdateExe() {
    const std::string versionUrl = makeGitHubRawUrl2("exe_version.txt"); // CHANGE
    const std::string exeNameUrl = makeGitHubRawUrl2("exe_file.txt"); // CHANGE

    std::string remoteVersion = getRemoteTextFile(versionUrl);
    if (remoteVersion.empty()) {
        std::cerr << skCrypt("Erreur : impossible de récupérer la version distante de l'exe.").decrypt() << std::endl;
        return;
    }

    if (remoteVersion != versionapp) {
        std::cout << skCrypt("Nouvelle version détectée : ").decrypt() << remoteVersion << std::endl;

        std::string remoteExeName = getRemoteTextFile(exeNameUrl);
        if (remoteExeName.empty()) {
            std::cerr << skCrypt("Erreur : nom de fichier .exe manquant.").decrypt() << std::endl;
            return;
        }

        char currentPath[MAX_PATH];
        GetModuleFileNameA(NULL, currentPath, MAX_PATH);
        std::string currentDir = std::filesystem::path(currentPath).parent_path().string();
        std::string downloadPath = currentDir + "\\" + remoteExeName;

        if (std::filesystem::exists(downloadPath)) {
            std::cout << skCrypt("Le fichier de mise à jour est déjà présent dans le dossier.").decrypt() << std::endl;
            Sleep(5000);
            exit(0);
        }

        std::string remoteExeUrl = makeGitHubRawUrl2(remoteExeName);

        std::cout << skCrypt("Téléchargement de la nouvelle version dans le répertoire courant...").decrypt() << std::endl;

        if (!DownloadFileDirect(remoteExeUrl, downloadPath)) {
            std::cerr << skCrypt("Échec du téléchargement de la nouvelle version.").decrypt() << std::endl;
            return;
        }

        std::cout << skCrypt("Téléchargement terminé. Le fichier a été placé à côté de l'exécutable actuel.").decrypt() << std::endl;
        std::cout << skCrypt("Pensez à créer une exception WindowsDefender pour le répertoire ou est stocké le .exe. Sinon il vous sera impossible de télécharger la nouvelle version").decrypt() << std::endl;
        std::cout << skCrypt("Fermeture du programme actuel dans 20 secondes.").decrypt() << std::endl;
        Sleep(20000);
        exit(0);
    }
    else {
        std::cout << skCrypt("Aucune mise à jour de l'exe disponible.").decrypt() << std::endl;
    }
}
