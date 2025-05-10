#pragma once
#include <string>

// Télécharge tous les fichiers listés sans supprimer les anciens
void downloadFromFileList(const std::string& fileListUrl, const std::string& targetDirectory);

// Supprime les fichiers existants avant de les re-télécharger
void updateFromFileList(const std::string& fileListUrl, const std::string& targetDirectory);

// Vérifie la version locale et met à jour les fichiers si une nouvelle version est disponible
void CheckAndUpdate(const std::string& targetDirectory);  // FULL USE

// Génère une URL GitHub "raw" pour un fichier donné
std::string makeGitHubRawUrl(const std::string& fileName);

// Récupère la version distante à partir d’une URL
std::string getRemoteVersion(const std::string& url);

// Récupère la version locale à partir d’un fichier
std::string getLocalVersion(const std::string& filePath);

// Écrit une version locale dans un fichier
void setLocalVersion(const std::string& filePath, const std::string& version);
