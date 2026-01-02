#include "pch.h"

#include "loader/log.h"
#include "loader/load.h"
#include "loader/yyc.h"
#include "loader/modpanel.h"
#include "loader/d3d11_hook.h"

#include "utils/utils.hpp"
#include "game_hook/expert_mode.h"

#include "GMLScriptEnv/gml.h"

#define DEBUG 1

DWORD WINAPI dll(LPVOID hModule)
{
	HWND proc_window = Utils::GetProcessWindow();
	HMODULE base = GetModuleHandle(0);

	ImGui::SetCurrentContext(loader_get_imgui_context());

	add_panel(new tastudio());
	add_panel(new roomview());

	CRoom* room = get_room_by_index(0);
	if (room->m_Caption)
	{
		loader_log_info(room->m_Caption);
	}

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
		FreeLibraryAndExitThread(hModule, 0);
	}

	return TRUE;
}