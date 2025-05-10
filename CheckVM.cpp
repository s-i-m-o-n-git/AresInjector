#include "CheckVM.h"
#include <Windows.h>
#include <intrin.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#include "skStr.h"

static std::string lastVMReason = "";

std::string getLastVMReason() {
    return lastVMReason;
}

bool containsAny(const std::string& src, const std::vector<std::string>& patterns) {
    for (const auto& pat : patterns) {
        if (src.find(pat) != std::string::npos) return true;
    }
    return false;
}

bool checkWMIProperty(const std::wstring& wqlClass, const std::wstring& property, const std::vector<std::string>& vmHints) {
    HRESULT hres;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) return false;

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);

    if (FAILED(hres)) {
        CoUninitialize();
        return false;
    }

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hres)) {
        CoUninitialize();
        return false;
    }

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),
        NULL, NULL, 0, NULL, 0, 0, &pSvc);
    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return false;
    }

    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE);
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return false;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t((L"SELECT " + property + L" FROM " + wqlClass).c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return false;
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) break;

        VARIANT vtProp;
        hr = pclsObj->Get(property.c_str(), 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
            std::wstring wstr = vtProp.bstrVal;
            std::string str(wstr.begin(), wstr.end());

            if (containsAny(str, vmHints)) {
                lastVMReason = str;
                VariantClear(&vtProp);
                pclsObj->Release();
                pSvc->Release();
                pLoc->Release();
                pEnumerator->Release();
                CoUninitialize();
                return true;
            }
        }
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
    return false;
}

bool checkVM() {
    lastVMReason.clear();
    std::vector<std::string> vmHints = {
        "VBOX", "VMWARE", "VIRTUALBOX", "QEMU", "XEN", "KVM", "PARALLELS", "HYPER-V", "HYPERV"
    };

    std::vector<std::pair<std::wstring, std::wstring>> props = {
        {L"Win32_BIOS", L"Manufacturer"},
        {L"Win32_ComputerSystem", L"Manufacturer"},
        {L"Win32_BaseBoard", L"Product"},
        {L"Win32_DiskDrive", L"Model"},
        {L"Win32_VideoController", L"Name"}
    };

    for (auto& [cls, prop] : props) {
        if (checkWMIProperty(cls, prop, vmHints)) {
            return true;
        }
    }

    // CPU Check (Hypervisor CPUID)
    int cpuInfo[4] = { -1 };
    __cpuid(cpuInfo, 0x40000000);
    char hyperVendor[13];
    memcpy(hyperVendor + 0, &cpuInfo[1], 4);
    memcpy(hyperVendor + 4, &cpuInfo[2], 4);
    memcpy(hyperVendor + 8, &cpuInfo[3], 4);
    hyperVendor[12] = '\0';

    std::string hvStr(hyperVendor);
    if (containsAny(hvStr, vmHints)) {
        lastVMReason = "Hyperviseur: " + hvStr;
        return true;
    }

    return false;
}
