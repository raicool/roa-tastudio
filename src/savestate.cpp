#include "savestate.h"

PFUNC_YYGMLScript AdvanceGameFrame_orig = nullptr;
PFUNC_YYGMLScript dp_handle_flow_state_save_orig = nullptr;
PFUNC_YYGMLScript dp_handle_flow_state_load_orig = nullptr;
PFUNC_YYGMLScript buffer_create_track_orig = nullptr;

RValue& AdvanceGameFrame_detour(
	CInstance* self,
	CInstance* other,
	RValue* out,
	int argc,
	RValue** argv
)
{
	RValue* g_dp_replay_frames = loader_get_global_variable("g_dp_replay_frames");

	if (g_dp_replay_frames->get_s32() < 2000000)
	{
		g_dp_replay_frames->set_real(2000000);
		loader_log_info("extended replay length to 2000000");
	}

// 	if (loader_get_object_instance_count("oPlayer") > 0)
// 	{
// 		RValue* gameplay_time = loader_get_global_variable("gameplay_time");
// 		const int32_t game_elapsed = gameplay_time->get_s32();
// 
// 		SavestateManager::save_timer++;
// 		
// 		if (SavestateManager::save_timer >= SavestateManager::save_interval)
// 		{
// 			auto NetworkManager = loader_get_object_instances("NetworkManager")[0];
// 
// 			//RValue new_buffer = loader_call_builtin_ex("buffer_create", nullptr, nullptr, { 0, YYBufferType::buffer_fast, 1 });
// 			RValue buffer_size = 300000.0;
// 			RValue arg2 = 0;
// 			RValue arg3 = 1;
// 
// 			auto new_savestate = loader_call_script("gml_Script_buffer_create_track", NetworkManager, NetworkManager, { &buffer_size, &arg2, &arg3 });
// 			loader_call_script("gml_Script_dp_handle_flow_state_save", NetworkManager, NetworkManager, { new_savestate });
// 
// 			SavestateManager::_savestates[game_elapsed] = *new_savestate;
// 			SavestateManager::save_timer = 0;
// 			SavestateManager::count++;
// 
// 			if (SavestateManager::count >= 20)
// 			{
// 				SavestateManager::delete_oldest_savestate();
// 				SavestateManager::count = 0;
// 			}
// 		}
// 	}
	return AdvanceGameFrame_orig(self, other, out, argc, argv);
}

RValue& dp_handle_flow_state_save_detour(
	CInstance* self,
	CInstance* other,
	RValue* out,
	int argc,
	RValue** argv
)
{
	return dp_handle_flow_state_save_orig(self, other, out, argc, argv);
}

RValue& dp_handle_flow_state_load_detour(
	CInstance* self,
	CInstance* other,
	RValue* out,
	int argc,
	RValue** argv
)
{
	return dp_handle_flow_state_load_orig(self, other, out, argc, argv);
}

RValue& buffer_create_track_detour(
	CInstance* self,
	CInstance* other,
	RValue* out,
	int argc,
	RValue** argv
)
{
	return buffer_create_track_orig(self, other, out, argc, argv);
}

void SavestateManager::init_advance_frame_hook()
{
	auto AdvanceGameFrame = loader_get_script("gml_Script_AdvanceGameFrame");
	auto dp_handle_flow_state_save = loader_get_script("gml_Script_dp_handle_flow_state_save");
	auto dp_handle_flow_state_load = loader_get_script("gml_Script_dp_handle_flow_state_load");
	auto buffer_create_track = loader_get_script("gml_Script_buffer_create_track");

	loader_hook_create(
		AdvanceGameFrame,
		&AdvanceGameFrame_detour,
		reinterpret_cast<void**>(&AdvanceGameFrame_orig)
	);

	loader_hook_create(
		dp_handle_flow_state_save,
		&dp_handle_flow_state_save_detour,
		reinterpret_cast<void**>(&dp_handle_flow_state_save_orig)
	);


	loader_hook_create(
		dp_handle_flow_state_load,
		&dp_handle_flow_state_load_detour,
		reinterpret_cast<void**>(&dp_handle_flow_state_load_orig)
	);

	loader_hook_create(
		buffer_create_track,
		&buffer_create_track_detour,
		reinterpret_cast<void**>(&buffer_create_track_orig)
	);

	loader_hook_enable(AdvanceGameFrame);
	loader_hook_enable(dp_handle_flow_state_save);
	loader_hook_enable(dp_handle_flow_state_load);
	loader_hook_enable(buffer_create_track);
}