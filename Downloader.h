#pragma once
#include <string>

// T�l�charge tous les fichiers list�s sans supprimer les anciens
void downloadFromFileList(const std::string& fileListUrl, const std::string& targetDirectory);

// Supprime les fichiers existants avant de les re-t�l�charger
void updateFromFileList(const std::string& fileListUrl, const std::string& targetDirectory);

// V�rifie la version locale et met � jour les fichiers si une nouvelle version est disponible
void CheckAndUpdate(const std::string& targetDirectory);  // FULL USE

// G�n�re une URL GitHub "raw" pour un fichier donn�
std::string makeGitHubRawUrl(const std::string& fileName);

// R�cup�re la version distante � partir d�une URL
std::string getRemoteVersion(const std::string& url);

// R�cup�re la version locale � partir d�un fichier
std::string getLocalVersion(const std::string& filePath);

// �crit une version locale dans un fichier
void setLocalVersion(const std::string& filePath, const std::string& version);
