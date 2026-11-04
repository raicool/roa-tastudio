#include "pch.h"

#include "mod.h"
#include "player.h"
#include "savestate.h"

#include "loader/log.h"
#include "loader/load.h"
#include "loader/yyc.h"
#include "loader/d3d11_hook.h"

LOADER_MOD_ENTRY initialize(loader_info* info)
{
	loader_mod& mod_instance = loader_mod::get();

	mod_instance.name = MOD_NAME;
	mod_instance.description = "rivals 1 mod that implements a tas tool inspired by BizHawk's TAStudio.";
	mod_instance.repository = "https://github.com/raicool/roa-tastudio";
	mod_instance.commit_hash = 0x00000000;

	imgui_init();

	Player::init_oplayer_hook();
	SavestateManager::init_advance_frame_hook();

	loader_add_present_callback(render_panels);

	loader_add_wndproc_callback(handle_wndproc);

	return mod_instance;
}