#pragma once
#include <string>

namespace vars
{
	inline std::wstring_view str_steam_process_name{ L"steam.exe" };
	inline std::wstring_view str_game_process_name{ L"cs2.exe" };
	inline std::wstring_view str_dll_name{ L"cheat.dll" };
	inline std::wstring_view str_steam_mod_name{ L"tier0_s.dll" };
	inline std::wstring_view str_game_mod_name{ L"matchmaking.dll" };
	inline std::wstring_view str_d3d11_mod_name{ L"d3d11.dll" };
	inline uint32_t game_appid{ 730 };
	inline std::wstring str_game_launch_opts{ L"-console -worldwide -novid" };
	inline std::string GetAppDataPath()
	{
		char* appdata = nullptr;
		size_t len = 0;
		_dupenv_s(&appdata, &len, "APPDATA");
		std::string path = appdata ? std::string(appdata) + "\\Ar�s" : ".\\Ar�s";
		free(appdata);
		return path;
	}

	inline std::string str_dll_dir_path = GetAppDataPath();
}