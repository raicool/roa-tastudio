#include "pch.h"

#include "parser.h"

#include <optional>

bool InputParser::get_button(char indicator, int* mask, uint8_t input, bool inverse)
{
	bool val_upper = indicator == toupper(input_entry_table[input].indicator[0]);
	bool val_lower = indicator == tolower(input_entry_table[input].indicator[0]);

	val_upper ? *mask |= input_entry_table[input].mask : 0;
	val_lower ? *mask &= ~input_entry_table[input].mask : 0;

	// used for stick neutral state (for some reason 'Z' and 'z' are switched?)
	if (inverse && (val_upper ^ val_lower) != 0)
	{
		*mask ^= input_entry_table[input].mask;
	}

	return val_upper || val_lower;
}

void InputParser::set_input_bool(uint32_t frame, uint8_t input_id, bool value)
{
	Controller& ctx = _map[frame];

	ctx.unique = true;

	if (!_map.contains(frame + 1))
	{
		_map[frame + 1] = ctx;
	}

	switch (input_id)
	{
	case InputTypeEnum::NEUTRAL: ctx.neutral = value; break;
	case InputTypeEnum::ANALOG_MODIFIER: ctx.analog_modifier = value; break;
	case InputTypeEnum::ANALOG_HARD_UP: ctx.analog_hup = value; break;
	case InputTypeEnum::ANALOG_HARD_RIGHT: ctx.analog_hright = value; break;
	case InputTypeEnum::ANALOG_HARD_LEFT: ctx.analog_hleft = value; break;
	case InputTypeEnum::ANALOG_HARD_DOWN: ctx.analog_hdown = value; break;
	case InputTypeEnum::ANALOG_LIGHT_UP: ctx.analog_lup = value; break;
	case InputTypeEnum::ANALOG_LIGHT_RIGHT: ctx.analog_lright = value; break;
	case InputTypeEnum::ANALOG_LIGHT_LEFT: ctx.analog_lleft = value; break;
	case InputTypeEnum::ANALOG_LIGHT_DOWN: ctx.analog_ldown = value; break;
	case InputTypeEnum::ATTACK: ctx.attack = value; break;
	case InputTypeEnum::SPECIAL: ctx.special = value; break;
	case InputTypeEnum::STRONG: ctx.strong = value; break;
	case InputTypeEnum::PARRY: ctx.parry = value; break;
	case InputTypeEnum::JUMP: ctx.jump = value; break;
	case InputTypeEnum::TAUNT: ctx.taunt = value; break;
	case InputTypeEnum::CSTICK_UP: ctx.cstick_up = value; break;
	case InputTypeEnum::CSTICK_RIGHT: ctx.cstick_right = value; break;
	case InputTypeEnum::CSTICK_LEFT: ctx.cstick_left = value; break;
	case InputTypeEnum::CSTICK_DOWN: ctx.cstick_down = value; break;
	case InputTypeEnum::TAP_JUMP: ctx.tap_jump = value; break;
	default: loader_log_warn("passed invalid enum parameter into set_input_bool(). input_id = {}", input_id);
	}

	serialize();
}

bool InputParser::get_input_bool(uint32_t frame, const uint8_t input_id)
{
	Controller& ctx = _map[frame];

	switch (input_id)
	{
	case InputTypeEnum::NEUTRAL: return ctx.neutral;
	case InputTypeEnum::ANALOG_MODIFIER: return ctx.analog_modifier;
	case InputTypeEnum::ANALOG_HARD_UP: return ctx.analog_hup;
	case InputTypeEnum::ANALOG_HARD_RIGHT: return ctx.analog_hright;
	case InputTypeEnum::ANALOG_HARD_LEFT: return ctx.analog_hleft;
	case InputTypeEnum::ANALOG_HARD_DOWN: return ctx.analog_hdown;
	case InputTypeEnum::ANALOG_LIGHT_UP: return ctx.analog_lup;
	case InputTypeEnum::ANALOG_LIGHT_RIGHT: return ctx.analog_lright;
	case InputTypeEnum::ANALOG_LIGHT_LEFT: return ctx.analog_lleft;
	case InputTypeEnum::ANALOG_LIGHT_DOWN: return ctx.analog_ldown;
	case InputTypeEnum::ATTACK: return ctx.attack;
	case InputTypeEnum::SPECIAL: return ctx.special;
	case InputTypeEnum::STRONG: return ctx.strong;
	case InputTypeEnum::PARRY: return ctx.parry;
	case InputTypeEnum::JUMP: return ctx.jump;
	case InputTypeEnum::TAUNT: return ctx.taunt;
	case InputTypeEnum::CSTICK_UP: return ctx.cstick_up;
	case InputTypeEnum::CSTICK_RIGHT: return ctx.cstick_right;
	case InputTypeEnum::CSTICK_LEFT: return ctx.cstick_left;
	case InputTypeEnum::CSTICK_DOWN: return ctx.cstick_down;
	case InputTypeEnum::TAP_JUMP: return ctx.tap_jump;
	default: loader_log_warn("passed invalid enum parameter into get_input_bool(). input_id = {}", input_id);
	}
}

void InputParser::set_input_u32(uint32_t frame, const uint8_t input_id, const uint32_t value)
{
	Controller& ctx = _map[frame];

	ctx.unique = true;

	switch (input_id)
	{
	case InputTypeEnum::ANGLE: ctx.angle = value; break;
	default: loader_log_warn("passed invalid enum parameter into set_input_u32(). input_id = {}", input_id);
	}
}

static std::optional<size_t> strnum(char*& str)
{
	const size_t length = strlen(str);
	// empty, return
	if (length <= 0 || isdigit(str[0]) == false)
	{
		return {};
	}

	size_t r = 0;

	for (size_t i = 0; i < length; i++)
	{
		if (isdigit(str[0]) == 0)
		{
			break;
		}

		r = r * 10 + (str[0] ^ '0');
		str++;
	}

	return r;
}

void InputParser::deserialize(char* replay_string)
{
	if (!_map.empty()) _map.clear();

	const size_t size = strlen(replay_string);

	if (size == 0)
	{
		return;
	}

	unique_frames = 0;
	Controller ctx{ 0 };
	ctx.unique = true;

	char* read = replay_string;

	bool finished = false;
	while (finished == false)
	{
		unique_frames++;

		auto frame_string = strnum(read);

		if (frame_string != std::nullopt)
		{
			ctx.frame = frame_string.value();
		}

		if (_map.contains(ctx.frame))
		{
			ctx = _map[ctx.frame];
		}
		else
		{
			ctx.has_angle = false;
			ctx.tap_jump = false;
			ctx.analog_modifier = false;
			ctx.analog_hup = false;
			ctx.analog_hright = false;
			ctx.analog_hleft = false;
			ctx.analog_hdown = false;
		}


		if (read[0] == 'y')
		{
			read++;

			ctx.has_angle = true;
			std::string angle_str = std::string(read, 3);
			ctx.angle = std::stoi(angle_str);

			read += 3;
		}

		for (uint8_t i = NEUTRAL; i < InputTypeEnum_MAX; i++)
		{
			if (get_button(read[0], &ctx.raw, i, i == NEUTRAL) == true)
			{
				read++;
				break;
			}
		}

		_map[ctx.frame] = ctx;

		if ((read - replay_string) >= size)
		{
			finished = true;
		}
	}

	ready = true;
}

void InputParser::serialize()
{
	s_buffer.clear();
	Controller input_xor{ 0 };
	uint32_t angle_xor{ 0 };
	uint32_t has_angle_xor{ 0 };

	int32_t last_frame = -1;
	bool analog_toggle = false;

	for (auto& _frame : _map)
	{
		Controller& _entry = _frame.second;

		if (_frame.first > 0)
		{
			input_xor ^= _entry;
			angle_xor ^= _entry.angle;
			has_angle_xor ^= _entry.has_angle;

			if (input_xor.is_zero() && angle_xor == 0 && last_frame != -1)
			{
				// no differences, no point in adding it to the file
				input_xor = _entry;
				continue;
			}

			last_frame = _frame.first;

			s_buffer.append(std::to_string(_frame.first));

			// "240y 90zUPMX242uxZ"

			if (input_xor.analog_modifier)
			{
				s_buffer.push_back(_entry.neutral ? 'N' : 'n');
			}

			if (input_xor.neutral)
			{
				s_buffer.push_back(_entry.neutral ? 'z' : 'Z');
			}

			if (angle_xor)
			{
				// padding format e.g. "y  2" "y 24" "y240"
				s_buffer.append(std::format("y{:3}", _entry.angle));
			}

			if (input_xor.analog_lleft)
			{
				s_buffer.push_back(_entry.analog_lleft ? 'L' : 'l');
			}

			if (input_xor.analog_lright)
			{
				s_buffer.push_back(_entry.analog_lright ? 'R' : 'r');
			}

			if (input_xor.analog_lup)
			{
				s_buffer.push_back(_entry.analog_lup ? 'U' : 'u');
			}

			if (input_xor.analog_ldown)
			{
				s_buffer.push_back(_entry.analog_ldown ? 'D' : 'd');
			}

			if (input_xor.cstick_left)
			{
				s_buffer.push_back(_entry.cstick_left ? 'F' : 'f');
			}

			if (input_xor.cstick_right)
			{
				s_buffer.push_back(_entry.cstick_right ? 'G' : 'g');
			}

			if (input_xor.cstick_up)
			{
				s_buffer.push_back(_entry.cstick_up ? 'X' : 'x');
			}

			if (input_xor.cstick_down)
			{
				s_buffer.push_back(_entry.cstick_down ? 'W' : 'w');
			}

			if (_entry.analog_hleft)
			{
				s_buffer.push_back('E');
			}

			if (_entry.analog_hright)
			{
				s_buffer.push_back('I');
			}

			if (_entry.analog_hup)
			{
				s_buffer.push_back('P');
			}

			if (_entry.analog_hdown)
			{
				s_buffer.push_back('O');
			}

			if (input_xor.jump)
			{
				s_buffer.push_back(_entry.jump ? 'J' : 'j');
			}

			if (input_xor.attack)
			{
				s_buffer.push_back(_entry.attack ? 'A' : 'a');
			}

			if (input_xor.special)
			{
				s_buffer.push_back(_entry.special ? 'B' : 'b');
			}

			if (input_xor.parry)
			{
				s_buffer.push_back(_entry.parry ? 'S' : 's');
			}

			if (input_xor.strong)
			{
				s_buffer.push_back(_entry.strong ? 'C' : 'c');
			}

			if (_entry.tap_jump)
			{
				s_buffer.push_back('M');
			}

			input_xor = _entry;
			angle_xor = _entry.angle;
			has_angle_xor = _entry.has_angle;
		}
	}

	// overwrite player replay inputs
	RValue* replay_string = loader_get_global_variable("replay_string");
	DynamicArrayOfRValue* dyn_array = (reinterpret_cast<RefDynamicArrayOfRValue*>(replay_string->value_array))->pArray;

	loader_set_string(&dyn_array->arr[player], s_buffer.data());
}