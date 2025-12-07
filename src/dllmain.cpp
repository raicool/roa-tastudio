#include "pch.h"

#include "loader/log.h"

#include "utils/utils.hpp"
#include "hooks/hooks.hpp"
#include "game_hook/expert_mode.h"

#define DEBUG 1

DWORD WINAPI dll(LPVOID hModule)
{
#if DEBUG
	AllocConsole();

	SetConsoleTitleA("roa-mod-loader");

	freopen_s(reinterpret_cast<FILE**>(stdin), "conin$", "r", stdin);
	freopen_s(reinterpret_cast<FILE**>(stdout), "conout$", "w", stdout);

	::ShowWindow(GetConsoleWindow(), SW_SHOW);
#endif
	HWND proc_window = Utils::GetProcessWindow();
	HMODULE base = GetModuleHandle(0);

	loader_log_debug("Rendering backend: DirectX11");

	add_panel<tastudio>();

	Hooks::Init();

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

#ifndef DISABLE_LOGGING_CONSOLE
		fclose(stdin);
		fclose(stdout);

		if (H::bShuttingDown) {
			::ShowWindow(GetConsoleWindow(), SW_HIDE);
		}
		else {
			FreeConsole();
		}
#endif

		FreeLibraryAndExitThread(hModule, 0);
	}

	return TRUE;
}