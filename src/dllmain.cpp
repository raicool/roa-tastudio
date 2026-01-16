#include "pch.h"

#include "game_hook/expert_mode.h"
#include "mod.h"

#include "loader/log.h"
#include "loader/load.h"
#include "loader/yyc.h"
#include "loader/d3d11_hook.h"
#include "GMLScriptEnv/gml.h"

#define DEBUG 1

DWORD WINAPI dll(LPVOID hModule)
{
	HMODULE base = GetModuleHandle(0);
	loader_log_debug("base: {}", (void*)base);

	std::string mod_name_str = MOD_NAME;
	loader_fetch_mod_repository(mod_name_str);

	panel_init();

	add_panel<tastudio>();
	add_panel<roomview>();

	loader_add_present_callback(render_panels);
	loader_add_wndproc_callback(handle_wndproc);

	CRoom* room = loader_get_room_by_index(0);
	if (room->m_Caption)
	{
		loader_log_info(room->m_Caption);
	}

	expert_mode::init_hooks((uint32_t)base);
	expert_mode::enable_hooks();
	
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