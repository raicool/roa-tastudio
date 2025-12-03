#include "pch.h"

#include "console/console.hpp"
#include "utils/utils.hpp"
#include "hooks/hooks.hpp"
#include "game_hook/expert_mode.h"

#define DEBUG 1

DWORD WINAPI dll(LPVOID hModule)
{
#if DEBUG
	Console::Alloc();
#endif

	LOG("[+] Rendering backend: %s\n", Utils::RenderingBackendToStr());
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

//	this apparently decides if the cursor shows or not?
//	TODO: please replace this at some point im begging
// 
// 	*(bool*)0x05ec1d5c = true;
// 	*(bool*)(base + 0x05b71d9a) = true;

	return TRUE;

#if DEBUG
	Console::Free();
#endif
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
		Console::Free();

		FreeLibraryAndExitThread(hModule, 0);
	}

	return TRUE;
}