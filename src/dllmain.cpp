#include "pch.h"

#include "log.h"

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

	loader_log_debug("[+] Rendering backend: DirectX11\n");

	if (Utils::GetRenderingBackend() == NONE)
	{
		LOG("[!] Looks like you forgot to set a backend. Will unload after pressing enter...");
		std::cin.get();

		FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(hModule), 0);
		return 0;
	}

	HMODULE base = GetModuleHandle(0);

	if (MH_Initialize() == MH_OK)
		LOG("Minhook OK\n");
	else
		LOG("Minhook could not initialize...\n");

	add_panel<tastudio>();

	Hooks::Init();

	expert_mode::init_hooks((uint32_t)base);
	MH_EnableHook(MH_ALL_HOOKS);

	return TRUE;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		Utils::SetRenderingBackend(DIRECTX11);

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