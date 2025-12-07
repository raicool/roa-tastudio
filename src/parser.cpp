#include "pch.h"

#include "game_hook/expert_mode.h"
#include "parser.h"

void input_parser::get_button(std::string* str, int* mask, uint8_t input, bool inverse)
{
	uint32_t val_upper = str->find_first_of(toupper(entry_dict[input].indicator[0]));
	uint32_t val_lower = str->find_first_of(tolower(entry_dict[input].indicator[0]));

	(val_upper != -1) ? *mask |= entry_dict[input].mask : 0;
	(val_lower != -1) ? *mask &= ~entry_dict[input].mask : 0;

	// used for stick neutral state (for some reason 'Z' and 'z' are switched?)
	if (inverse && (val_upper ^ val_lower) != 0)
	{
		*mask ^= entry_dict[input].mask;
	}
}

void input_parser::set_input_bool(uint32_t frame, uint8_t input_id, bool value)
{
	controller& ctx = _map[frame];

	ctx.unique = true;

	if (!_map.contains(frame + 1))
	{
		_map[frame + 1] = ctx;
	}

	switch (input_id)
	{
	case e_input::NEUTRAL: ctx.neutral = value; break;
	case e_input::ANALOG_MODIFIER: ctx.analog_modifier = value; break;
	case e_input::ANALOG_HARD_UP: ctx.analog_hup = value; break;
	case e_input::ANALOG_HARD_RIGHT: ctx.analog_hright = value; break;
	case e_input::ANALOG_HARD_LEFT: ctx.analog_hleft = value; break;
	case e_input::ANALOG_HARD_DOWN: ctx.analog_hdown = value; break;
	case e_input::ANALOG_LIGHT_UP: ctx.analog_lup = value; break;
	case e_input::ANALOG_LIGHT_RIGHT: ctx.analog_lright = value; break;
	case e_input::ANALOG_LIGHT_LEFT: ctx.analog_lleft = value; break;
	case e_input::ANALOG_LIGHT_DOWN: ctx.analog_ldown = value; break;
	case e_input::ATTACK: ctx.attack = value; break;
	case e_input::SPECIAL: ctx.special = value; break;
	case e_input::STRONG: ctx.strong = value; break;
	case e_input::PARRY: ctx.parry = value; break;
	case e_input::JUMP: ctx.jump = value; break;
	case e_input::TAUNT: ctx.taunt = value; break;
	case e_input::CSTICK_UP: ctx.cstick_up = value; break;
	case e_input::CSTICK_RIGHT: ctx.cstick_right = value; break;
	case e_input::CSTICK_LEFT: ctx.cstick_left = value; break;
	case e_input::CSTICK_DOWN: ctx.cstick_down = value; break;
	case e_input::TAP_JUMP: ctx.tap_jump = value; break;
	default: LOG("passed invalid enum parameter into set_input_bool(). input_id = %i\n", input_id);
	}
}

bool input_parser::get_input_bool(uint32_t frame, uint8_t input_id)
{
	controller& ctx = _map[frame];

	switch (input_id)
	{
	case e_input::NEUTRAL: return ctx.neutral;
	case e_input::ANALOG_MODIFIER: return ctx.analog_modifier;
	case e_input::ANALOG_HARD_UP: return ctx.analog_hup;
	case e_input::ANALOG_HARD_RIGHT: return ctx.analog_hright;
	case e_input::ANALOG_HARD_LEFT: return ctx.analog_hleft;
	case e_input::ANALOG_HARD_DOWN: return ctx.analog_hdown;
	case e_input::ANALOG_LIGHT_UP: return ctx.analog_lup;
	case e_input::ANALOG_LIGHT_RIGHT: return ctx.analog_lright;
	case e_input::ANALOG_LIGHT_LEFT: return ctx.analog_lleft;
	case e_input::ANALOG_LIGHT_DOWN: return ctx.analog_ldown;
	case e_input::ATTACK: return ctx.attack;
	case e_input::SPECIAL: return ctx.special;
	case e_input::STRONG: return ctx.strong;
	case e_input::PARRY: return ctx.parry;
	case e_input::JUMP: return ctx.jump;
	case e_input::TAUNT: return ctx.taunt;
	case e_input::CSTICK_UP: return ctx.cstick_up;
	case e_input::CSTICK_RIGHT: return ctx.cstick_right;
	case e_input::CSTICK_LEFT: return ctx.cstick_left;
	case e_input::CSTICK_DOWN: return ctx.cstick_down;
	case e_input::TAP_JUMP: return ctx.tap_jump;
	default: LOG("passed invalid enum parameter into get_input_bool(). input_id = %i\n", input_id);
	}
}

void input_parser::set_input_u32(uint32_t frame, uint8_t input_id, uint32_t value)
{
	controller& ctx = _map[frame];

	ctx.unique = true;

	switch (input_id)
	{
	case e_input::ANGLE: ctx.angle = value; break;
	default: LOG("passed invalid enum parameter into set_input_bool(). input_id = %i\n", input_id);
	}
}

void input_parser::deserialize(std::fstream* file)
{
	if (file->bad())
	{
		LOG("[!] error processing recording input file! (file->bad() == true)");
		return;
	}

	if (!_map.empty()) _map.clear();

	file->seekg(0, std::ios::end);
	size_t size = file->tellg();
	std::string s(size, ' ');
	file->seekg(0);
	file->read(&s[0], size);

	unique_frames = 0;
	controller ctx{ 0 };
	ctx.unique = true;

	uint32_t begin = s.find_first_of(':', 0);

	if (begin == -1)
	{
		LOG("[!] error processing recording input file! is this a valid input file? (_file_buffer.find_first_of(':', 0) == -1)\n");
		return;
	}

	s.erase(0, begin + 1);
	_offset = std::stoi(s);

	bool finished = false;
	while (finished == false)
	{
		unique_frames++;

		std::string frame_str = s.substr(0, s.find_first_of(':'));
		uint32_t frame = std::stoi(frame_str);

		s.erase(0, frame_str.size() + 1);

		ctx.frame = std::stoi(frame_str);

		ctx.has_angle = false;
		ctx.tap_jump = false;
		ctx.neutral = false;
		ctx.analog_modifier = false;
		ctx.analog_hup = false;
		ctx.analog_hright = false;
		ctx.analog_hleft = false;
		ctx.analog_hdown = false;

		uint32_t indicator = frame_str.find_first_of('y');
		if (indicator != -1)
		{
			ctx.has_angle = true;
			std::string angle_str = frame_str.substr(indicator + 1, indicator + 4);
			ctx.angle = std::stoi(angle_str);
		}

		for (uint8_t i = ANALOG_MODIFIER; i < E_INPUT_MAX; i++)
		{
			get_button(&frame_str, &ctx.raw, i, i == NEUTRAL);
		}

		_map[frame] = ctx;

		if (s.size() <= 1) finished = true;
	}

	ready = true;
}

void input_parser::serialize(std::string& path, bool debug)
{
	std::string bak_path = std::format("{}.bak", path);
	// backup input file if it already exists
	if (std::filesystem::exists(path))
	{
		std::filesystem::copy(path, bak_path, std::filesystem::copy_options::overwrite_existing);
	}

	//	check if backup file copied correctly
	//	if (std::filesystem::exists(bak_path))
	// 	{
	// 		LOG("could not create backup file, stopping serialization.");
	// 		return;
	// 	}

	std::ofstream file(std::format("{}", path), std::ios::trunc | std::ios::out);

	std::string s_buffer;

	controller input_xor{ 0 };
	int32_t last_frame = -1;
	bool analog_toggle = false;

	for (auto& _frame : _map)
	{
		controller& _entry = _frame.second;

		if (_frame.first > 0)
		{
			input_xor ^= _entry;

			if (input_xor.is_zero())
			{
				// no differences, no point in adding it to the file
				input_xor = _entry;
				continue;
			}

			// start of frame data indicator
			s_buffer.append(":");

			s_buffer.append(std::to_string(_frame.first));
			s_buffer.push_back('\x00');

			// "240y 90zUPMX242uxZ"

			if (input_xor.neutral)
			{
				s_buffer.push_back(_entry.neutral ? 'z' : 'Z');
				s_buffer.push_back('\x00');
			}

			// padding format e.g. "y  2" "y 24" "y240"
			s_buffer.append(std::format("y{:3}", _entry.angle));
			s_buffer.push_back('\x00');

			if (input_xor.analog_lleft)
			{
				s_buffer.push_back(_entry.analog_lleft ? 'L' : 'l');
				s_buffer.push_back('\x00');
			}

			if (input_xor.analog_lright)
			{
				s_buffer.push_back(_entry.analog_lright ? 'R' : 'r');
				s_buffer.push_back('\x00');
			}

			if (input_xor.analog_lup)
			{
				s_buffer.push_back(_entry.analog_lup ? 'U' : 'u');
				s_buffer.push_back('\x00');
			}

			if (input_xor.analog_ldown)
			{
				s_buffer.push_back(_entry.analog_ldown ? 'D' : 'd');
				s_buffer.push_back('\x00');
			}

			if (input_xor.cstick_left)
			{
				s_buffer.push_back(_entry.cstick_left ? 'F' : 'f');
				s_buffer.push_back('\x00');
			}

			if (input_xor.cstick_right)
			{
				s_buffer.push_back(_entry.cstick_right ? 'G' : 'g');
				s_buffer.push_back('\x00');
			}

			if (input_xor.cstick_up)
			{
				s_buffer.push_back(_entry.cstick_up ? 'X' : 'x');
				s_buffer.push_back('\x00');
			}

			if (input_xor.cstick_down)
			{
				s_buffer.push_back(_entry.cstick_down ? 'W' : 'w');
				s_buffer.push_back('\x00');
			}

			if (_entry.analog_hleft)
			{
				s_buffer.push_back('E');
				s_buffer.push_back('\x00');
			}

			if (_entry.analog_hright)
			{
				s_buffer.push_back('I');
				s_buffer.push_back('\x00');
			}

			if (_entry.analog_hup)
			{
				s_buffer.push_back('P');
				s_buffer.push_back('\x00');
			}

			if (_entry.analog_hdown)
			{
				s_buffer.push_back('O');
				s_buffer.push_back('\x00');
			}

			if (input_xor.jump)
			{
				s_buffer.push_back(_entry.jump ? 'J' : 'j');
				s_buffer.push_back('\x00');
			}

			if (input_xor.attack)
			{
				s_buffer.push_back(_entry.attack ? 'A' : 'a');
				s_buffer.push_back('\x00');
			}

			if (input_xor.special)
			{
				s_buffer.push_back(_entry.special ? 'B' : 'b');
				s_buffer.push_back('\x00');
			}

			if (input_xor.parry)
			{
				s_buffer.push_back(_entry.parry ? 'S' : 's');
				s_buffer.push_back('\x00');
			}

			if (input_xor.strong)
			{
				s_buffer.push_back(_entry.strong ? 'C' : 'c');
				s_buffer.push_back('\x00');
			}

			if (_entry.tap_jump)
			{
				s_buffer.push_back('M');
				s_buffer.push_back('\x00');
			}

			input_xor = _entry;
		}
	}

	// eof indicator
	s_buffer.append(";\00");

	file.seekp(0, std::ofstream::beg);
	file.write(s_buffer.c_str(), s_buffer.size());
	file.close();

	// overwrite custom playback buffer with data we just made
	if (expert_mode::custom_playback_inputs_data)
	{
		delete[] expert_mode::custom_playback_inputs_data;
	}

	expert_mode::custom_playback_inputs_data = new char[s_buffer.size() + 0x73];
	expert_mode::custom_playback_inputs_size = s_buffer.size();
	for (uint32_t i = 0; i < s_buffer.size() + 0x73; i++)
	{
		expert_mode::custom_playback_inputs_data[i] = 0;
	}

	memcpy((void*)expert_mode::custom_playback_inputs_data, (void*)s_buffer.c_str(), s_buffer.size());
	expert_mode::overwrite = true;

	LOG(std::format("Saved to directory {}\n", std::format("{}", path)).c_str());
}