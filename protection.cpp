#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <sstream>

#include "Discord.h"
#include "CheckVM.h"
#include "skStr.h"

#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

void showReasonAndExit(const std::string& reason, const std::string& detectedProcess = "") {
    std::ostringstream oss;
    oss << skCrypt("Raison de l'arrêt : ").decrypt() << reason;

    if (!detectedProcess.empty()) {
        oss << "\n" << skCrypt("Processus détecté : ").decrypt() << detectedProcess;
    }

    //MessageBoxA(NULL, oss.str().c_str(), "Sécurité - Programme arrêté", MB_ICONERROR | MB_OK);
    ExitProcess(0);
}

bool checkDebugger() {
    return IsDebuggerPresent();
}

bool checkRemoteDebugger() {
    BOOL debuggerPresent = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &debuggerPresent);
    return debuggerPresent;
}

std::string checkSuspiciousProcesses() {
    const char* suspicious[] = {
        "x64dbg.exe", "x32dbg.exe", "ida.exe", "ida64.exe",
        "ollydbg.exe", "processhacker.exe", "tcpview.exe",
        "fiddler.exe", "wireshark.exe", "httpanalyzer.exe",
        "vboxservice.exe", "vboxtray.exe", "vmtoolsd.exe",
        "sandboxie.exe"
    };

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
        return "";

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe)) {
        do {
            char processName[MAX_PATH] = { 0 };
            WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, -1, processName, MAX_PATH, NULL, NULL);

            for (const auto& name : suspicious) {
                if (_stricmp(processName, name) == 0) {
                    CloseHandle(hSnap);
                    return std::string(processName);
                }
            }
        } while (Process32Next(hSnap, &pe));
    }

    CloseHandle(hSnap);
    return "";
}

std::string GetPublicIP() {
    std::string ip = "Unknown";

    HINTERNET hSession = WinHttpOpen(L"IP Checker/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, 0);
    if (!hSession) return ip;

    HINTERNET hConnect = WinHttpConnect(hSession, L"api.ipify.org", INTERNET_DEFAULT_HTTP_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return ip; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return ip; }

    if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
        WinHttpReceiveResponse(hRequest, NULL)) {
        DWORD dwSize = 0;
        WinHttpQueryDataAvailable(hRequest, &dwSize);
        if (dwSize > 0) {
            char* buffer = new char[dwSize + 1];
            ZeroMemory(buffer, dwSize + 1);
            DWORD dwDownloaded = 0;
            WinHttpReadData(hRequest, (LPVOID)buffer, dwSize, &dwDownloaded);
            if (dwDownloaded > 0) ip = std::string(buffer);
            delete[] buffer;
        }
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return ip;
}

std::string GetMachineName() {
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);
    if (GetComputerNameA(computerName, &size)) {
        return std::string(computerName);
    }
    return "UnknownPC";
}

void runChecksLoop() {
    const std::string webhook_url = skCrypt("WEBHOOK").decrypt(); // CHANGE

    while (true) {
        std::string detectedProc = checkSuspiciousProcesses();
        std::string ip = GetPublicIP();
        std::string pcName = GetMachineName();

        if (checkDebugger()) {
            DiscordWebhook webhook(webhook_url);
            webhook.setContent("");
            webhook.setUsername(pcName);
            webhook.setAvatarUrl(skCrypt("https://example.com/avatar.png").decrypt());

            DiscordEmbed embed(ip, skCrypt("# **checkDebugger**: **ExitProcess(0)**").decrypt());
            embed.setColor(0x00FF00);
            embed.setTimestamp();

            webhook.addEmbed(embed);
            webhook.execute();

            showReasonAndExit(skCrypt("Debuggeur local détecté").decrypt());
        }
        else if (checkRemoteDebugger()) {
            DiscordWebhook webhook(webhook_url);
            webhook.setContent("");
            webhook.setUsername(pcName);
            webhook.setAvatarUrl(skCrypt("https://example.com/avatar.png").decrypt());

            DiscordEmbed embed(ip, skCrypt("# **checkRemoteDebugger**: **ExitProcess(0)**").decrypt());
            embed.setColor(0xFF0000);
            embed.setTimestamp();

            webhook.addEmbed(embed);
            webhook.execute();

            showReasonAndExit(skCrypt("Remote Debug détecté").decrypt());
        }
        else if (!detectedProc.empty()) {
            DiscordWebhook webhook(webhook_url);
            webhook.setContent(detectedProc);
            webhook.setUsername(pcName);
            webhook.setAvatarUrl(skCrypt("https://example.com/avatar.png").decrypt());

            DiscordEmbed embed(ip, skCrypt("# **checkSuspiciousProcesses**: **ExitProcess(0)**").decrypt());
            embed.setColor(0xFFFF00);
            embed.setTimestamp();

            webhook.addEmbed(embed);
            webhook.execute();

            showReasonAndExit(skCrypt("Processus suspect détecté").decrypt(), detectedProc);
        }
        else if (checkVM()) {
            DiscordWebhook webhook(webhook_url);
            webhook.setContent(getLastVMReason());
            webhook.setUsername(pcName);
            webhook.setAvatarUrl(skCrypt("https://example.com/avatar.png").decrypt());

            DiscordEmbed embed(ip, skCrypt("# **checkVM**: **ExitProcess(0)**").decrypt());
            embed.setColor(0x0000FF);
            embed.setTimestamp();

            webhook.addEmbed(embed);
            webhook.execute();

            showReasonAndExit(skCrypt("VM Detecté").decrypt(), getLastVMReason());
        }

        Sleep(700 + (rand() % 500));
    }
}

void startSecurityThread() {
    std::thread(runChecksLoop).detach();
}
