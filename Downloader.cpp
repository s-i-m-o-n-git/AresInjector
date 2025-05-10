#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <urlmon.h>
#include <filesystem>
#include <wininet.h>
#include "skStr.h"
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")

const std::string githubUser = "s-i-m-o-n-git"; // CHANGE
const std::string githubRepo = "D"; // CHANGE
const std::string githubBranch = "main"; // CHANGE

std::string makeGitHubRawUrl(const std::string& fileName) {
    return "https://raw.githubusercontent.com/" + githubUser + "/" + githubRepo + "/" + githubBranch + "/" + fileName;
}

std::string getRemoteVersion(const std::string& url) {
    HINTERNET hInternet = InternetOpenA("VersionCheck", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return "";

    HINTERNET hFile = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hFile) {
        InternetCloseHandle(hInternet);
        return "";
    }

    char buffer[64];
    DWORD bytesRead;
    std::string version;

    while (InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        version.append(buffer, bytesRead);
    }

    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);

    version.erase(version.find_last_not_of(" \n\r\t") + 1);
    return version;
}

std::string getLocalVersion(const std::string& filePath) {
    std::ifstream in(filePath);
    std::string version;
    if (in.is_open()) {
        std::getline(in, version);
        in.close();
    }
    return version;
}

void setLocalVersion(const std::string& filePath, const std::string& version) {
    std::ofstream out(filePath, std::ios::trunc);
    if (out.is_open()) {
        out << version;
        out.close();
    }
}

void processFileList(const std::string& fileListUrl, const std::string& localBasePath, bool update) {
    HINTERNET hInternet = InternetOpenA(skCrypt("GitHubDownloader").decrypt(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) {
        std::cerr << skCrypt("Erreur: InternetOpenA a échoué.").decrypt() << std::endl;
        return;
    }

    HINTERNET hFile = InternetOpenUrlA(hInternet, fileListUrl.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hFile) {
        std::cerr << skCrypt("Erreur: InternetOpenUrlA a échoué.").decrypt() << std::endl;
        InternetCloseHandle(hInternet);
        return;
    }

    char buffer[4096];
    DWORD bytesRead;
    std::string content;

    while (InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        content.append(buffer, bytesRead);
    }

    std::istringstream iss(content);
    std::string fileName;

    while (std::getline(iss, fileName)) {
        fileName.erase(fileName.find_last_not_of(" \n\r\t") + 1);
        if (fileName.empty()) continue;

        std::string localPath = localBasePath + "\\" + fileName;
        std::string remoteURL = makeGitHubRawUrl(fileName);

        if (update && std::filesystem::exists(localPath)) {
            std::cout << skCrypt("Suppression de ").decrypt() << fileName << skCrypt("...").decrypt() << std::endl;
            std::filesystem::remove(localPath);
        }

        if (!std::filesystem::exists(localPath)) {
            std::cout << skCrypt("Téléchargement de ").decrypt() << fileName << skCrypt("...").decrypt() << std::endl;
            std::filesystem::create_directories(std::filesystem::path(localPath).parent_path());

            HRESULT res = URLDownloadToFileA(NULL, remoteURL.c_str(), localPath.c_str(), 0, NULL);

            if (SUCCEEDED(res)) {
                std::cout << skCrypt(" -> Téléchargement réussi !").decrypt() << std::endl;
            }
            else {
                std::cerr << skCrypt(" -> Échec du téléchargement (HRESULT: ").decrypt() << res << ")" << std::endl;
            }
        }
        else {
            std::cout << fileName << skCrypt(" existe déjà, téléchargement ignoré.").decrypt() << std::endl;
        }
    }

    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);
}

void downloadFromFileList(const std::string& fileListUrl, const std::string& localBasePath) {
    processFileList(fileListUrl, localBasePath, false);
}

void updateFromFileList(const std::string& fileListUrl, const std::string& localBasePath) {
    processFileList(fileListUrl, localBasePath, true);
}

void CheckAndUpdate(const std::string& targetDirectory) {
    const std::string versionFile = targetDirectory + "\\current_dll_version.txt"; // CHANGE
    const std::string versionUrl = makeGitHubRawUrl("dll_version.txt"); // CHANGE
    const std::string fileListUrl = makeGitHubRawUrl("filelist.txt"); // CHANGE

    std::string remoteVersion = getRemoteVersion(versionUrl);
    if (remoteVersion.empty()) {
        std::cerr << skCrypt("Erreur : impossible de récupérer la version distante.").decrypt() << std::endl;
        return;
    }

    std::string localVersion = getLocalVersion(versionFile);

    if (remoteVersion != localVersion) {
        std::cout << skCrypt("Mise à jour détectée.").decrypt() << std::endl;
        updateFromFileList(fileListUrl, targetDirectory);
        setLocalVersion(versionFile, remoteVersion);
        std::cout << skCrypt("Mise à jour vers la version ").decrypt() << remoteVersion << std::endl;
    }
    else {
        std::cout << skCrypt("Aucune mise à jour nécessaire.").decrypt() << std::endl;
    }
}
