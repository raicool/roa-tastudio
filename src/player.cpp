#include "player.h"

std::array<InputParser, 10> parsers;
std::unordered_map<size_t, uint8_t> player_frame_states[10];

PFUNC_YYGMLScript state_update_orig = nullptr;

RValue& state_update_detour(
	CInstance* self,
	CInstance* other,
	RValue* out,
	int argc,
	RValue** argv
)
{
	RValue* replay_mode = loader_get_global_variable("replay_mode");
	RValue* replay_string = loader_get_global_variable("replay_string");
	RValue* gameplay_time = loader_get_global_variable("gameplay_time");
	RValue* player_id = loader_get_instance_variable(self, "player");
	RValue* player_state = loader_get_instance_variable(self, "state");

	int32_t game_elapsed = gameplay_time->get_s32();
	uint8_t id_int = player_id->get_s32();

	if (id_int > parsers.size())
	{
		return state_update_orig(self, other, out, argc, argv);
	}

	DynamicArrayOfRValue* dyn_array = (reinterpret_cast<RefDynamicArrayOfRValue*>(replay_string->value_array))->pArray;
	InputParser& my_parser = parsers[id_int];
	auto& my_frame_state = player_frame_states[id_int];

	if (replay_mode->truthy() == false)
	{
		const GMLStringRef* string_ref = dyn_array->arr[id_int].value_stringref;

		my_parser.deserialize(const_cast<char*>(string_ref->string));
		my_parser.player = id_int;
	}

	// add empty frames to follow gameplay_time
	if (game_elapsed > my_parser.last_frame_number())
	{
		my_parser.add_frame(my_parser.last_frame_number(), game_elapsed - my_parser.last_frame_number());
	}

	my_frame_state[gameplay_time->get_s32() - 1] = player_state->get_s32();

	my_parser.ready = true;

	return state_update_orig(self, other, out, argc, argv);
}

void Player::init_oplayer_hook()
{
	auto state_update = loader_get_script("gml_Script_state_update");

	loader_hook_create(
		state_update,
		&state_update_detour,
		reinterpret_cast<void**>(&state_update_orig)
	);
	loader_hook_enable(state_update);
}