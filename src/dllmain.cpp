#include "pch.h"

#include "loader/log.h"
#include "loader/load.h"

#include "utils/utils.hpp"
#include "hooks/hooks.hpp"
#include "game_hook/expert_mode.h"

#define DEBUG 1

DWORD WINAPI dll(LPVOID hModule)
{
	HWND proc_window = Utils::GetProcessWindow();
	HMODULE base = GetModuleHandle(0);

	add_panel<tastudio>();

	Hooks::Init();

	check_mod_repository("roa-hook");

	expert_mode::init_hooks((uint32_t)base);
	expert_mode::enable_hooks((uint32_t)base);

	return TRUE;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		HANDLE hHandle = CreateThread(NULL, 0, dll, hModule, 0, NULL);
		if (hHandle != NULL)
		{
			CloseHandle(hHandle);
		}
	}
	else if (fdwReason == DLL_PROCESS_DETACH && !lpReserved)
	{
		Hooks::Free();

		FreeLibraryAndExitThread(hModule, 0);
	}

	return TRUE;
}