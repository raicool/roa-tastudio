#pragma once

#include <map>
#include <string>

enum e_input
{
	FRAME, //< editor only, not saved to replay/input file

	ANGLE,
	NEUTRAL,

	ANALOG_MODIFIER,
	ANALOG_HARD_UP,
	ANALOG_HARD_RIGHT,
	ANALOG_HARD_LEFT,
	ANALOG_HARD_DOWN,
	ANALOG_LIGHT_UP,
	ANALOG_LIGHT_RIGHT,
	ANALOG_LIGHT_LEFT,
	ANALOG_LIGHT_DOWN,

	ATTACK,
	SPECIAL,
	STRONG,
	PARRY,
	JUMP,
	TAUNT,
	CSTICK_UP,
	CSTICK_RIGHT,
	CSTICK_LEFT,
	CSTICK_DOWN,
	TAP_JUMP,

	E_INPUT_MAX
};

enum e_type
{
	TYPE_U32,
	TYPE_BOOLEAN,
};

struct input_entry
{
	const char* name;
	const char* indicator;
	int mask;
	char id; //< enum e_input
	char type; //< enum e_type
};

const input_entry entry_dict[] =
{
	{"Frame", "", 0, e_input::FRAME, e_type::TYPE_U32},
	{"Angle", "y", 0, e_input::ANGLE, e_type::TYPE_U32},
	{"Neutral", "Z", (1 << (0)), e_input::NEUTRAL, e_type::TYPE_BOOLEAN},
	{"Analog Modifier", "N", (1 << (1)), e_input::ANALOG_MODIFIER, e_type::TYPE_BOOLEAN},
	{"Analog Hard Up", "P", (1 << (2)), e_input::ANALOG_HARD_UP, e_type::TYPE_BOOLEAN},
	{"Analog Hard Right", "I", (1 << (3)), e_input::ANALOG_HARD_RIGHT, e_type::TYPE_BOOLEAN},
	{"Analog Hard Left", "E", (1 << (4)),e_input::ANALOG_HARD_LEFT, e_type::TYPE_BOOLEAN},
	{"Analog Hard Down", "O", (1 << (5)), e_input::ANALOG_HARD_DOWN, e_type::TYPE_BOOLEAN},
	{"Analog Light Up", "U", (1 << (6)), e_input::ANALOG_LIGHT_UP, e_type::TYPE_BOOLEAN},
	{"Analog Light Right", "R", (1 << (7)), e_input::ANALOG_LIGHT_RIGHT, e_type::TYPE_BOOLEAN},
	{"Analog Light Left", "L", (1 << (8)), e_input::ANALOG_LIGHT_LEFT, e_type::TYPE_BOOLEAN},
	{"Analog Light Down", "D", (1 << (9)), e_input::ANALOG_LIGHT_DOWN, e_type::TYPE_BOOLEAN},
	{"Attack", "A", (1 << (10)), e_input::ATTACK, e_type::TYPE_BOOLEAN},
	{"Special", "B", (1 << (11)), e_input::SPECIAL, e_type::TYPE_BOOLEAN},
	{"Strong", "C", (1 << (12)), e_input::STRONG, e_type::TYPE_BOOLEAN},
	{"Parry", "S", (1 << (13)), e_input::PARRY, e_type::TYPE_BOOLEAN},
	{"Jump", "J", (1 << (14)), e_input::JUMP, e_type::TYPE_BOOLEAN},
	{"Taunt", "T", (1 << (15)), e_input::TAUNT, e_type::TYPE_BOOLEAN},
	{"C-Stick Up", "X", (1 << (16)), e_input::CSTICK_UP, e_type::TYPE_BOOLEAN},
	{"C-Stick Right", "G", (1 << (17)), e_input::CSTICK_RIGHT, e_type::TYPE_BOOLEAN},
	{"C-Stick Left", "F", (1 << (18)), e_input::CSTICK_LEFT, e_type::TYPE_BOOLEAN},
	{"C-Stick Down", "W", (1 << (19)), e_input::CSTICK_DOWN, e_type::TYPE_BOOLEAN},
	{"Tap Jump?", "M", (1 << (20)), e_input::TAP_JUMP, e_type::TYPE_BOOLEAN},
};

struct controller
{
	bool unique; //< editor only, not saved to replay/input file
	bool has_angle; //< editor only, not saved to replay/input file

	uint32_t frame;
	uint32_t angle;

	union
	{
		struct
		{
			bool neutral : 1;
			bool analog_modifier : 1;
			bool analog_hup : 1;
			bool analog_hright : 1;
			bool analog_hleft : 1;
			bool analog_hdown : 1;
			bool analog_lup : 1;
			bool analog_lright : 1;
			bool analog_lleft : 1;
			bool analog_ldown : 1;

			bool attack : 1;
			bool special : 1;
			bool strong : 1;
			bool parry : 1;
			bool jump : 1;
			bool taunt : 1;

			bool cstick_up : 1;
			bool cstick_right : 1;
			bool cstick_left : 1;
			bool cstick_down : 1;

			bool tap_jump : 1; //< seemingly enables whenever a hard analog up input is registered
			// ^ is enabled regardless if player's control scheme has tap jump enabled, but won't do anything unless tap jump is enabled
		};

		int raw;
	};

	// xors all input toggles, useful during serialization
	controller& operator ^= (const controller& rvalue)
	{
		raw ^= rvalue.raw;
		return *this;
	}

	bool is_zero()
	{
		return raw == 0;
	}
};

class input_parser
{
public:
	controller* get_frame(uint32_t frame_idx)
	{
		if (frame_idx < last_frame_number())
		{
			return &_map[frame_idx];
		}
		return nullptr;
	}

	controller* set_frame(uint32_t frame_idx, controller frame, bool overwrite = false)
	{
		if (frame_idx < last_frame_number())
		{
			if (_map.contains(frame_idx) && !overwrite)
			{
				return &_map[frame_idx];
			}

			_map[frame_idx] = frame;
			_map[frame_idx].unique = true;
			_map[frame_idx].frame = frame_idx;
			return &_map[frame_idx];
		}
		return nullptr;
	}

	void erase_frame(uint32_t frame_idx)
	{
		if (_map.contains(frame_idx))
		{
			_map[frame_idx] = { 0 };
		}
	}

	void delete_frame(uint32_t frame_idx)
	{
		if (_map.contains(frame_idx))
		{
			for (auto it = _map.find(frame_idx); it != _map.end(); it++)
			{
				if (it->first > frame_idx)
				{
					_map[it->first - 1] = _map[it->first];
				}
			}

			_map.erase(std::prev(_map.end()));
		}
	}

	controller* add_frame(uint32_t frame_idx, uint32_t amount = 1, controller default_input = { 0 })
	{
		if (_map.contains(frame_idx))
		{
			_map.emplace(std::pair<uint32_t, controller>(last_frame_number() + 1, { 0 }));

			for (auto it = _map.rbegin(); it != _map.rend(); it++)
			{
				if (it->first > frame_idx + amount)
				{
					_map[it->first] = _map[it->first - amount];
				}
				else
				{
					//it->second = default_input;

					if (it->first <= frame_idx)
					{
						it->second.unique = true;
						it->second.has_angle = true;
						return &it->second;
					}
				}
			}
		}
	}

	bool is_unique(uint32_t frame)
	{
		return _map.contains(frame) ? _map[frame].unique : false;
	}

	uint32_t frame_offset()
	{
		return _offset;
	}

	uint32_t frame_count()
	{
		return _map.size();
	}

	uint32_t last_frame_number()
	{
		if (_map.empty() == false)
		{
			return _map.rbegin()->first;
		}
	}

	bool get_input_bool(uint32_t frame, uint8_t input_id);
	void set_input_bool(uint32_t frame, uint8_t input_id, bool value);
	void set_input_u32(uint32_t frame, uint8_t input_id, uint32_t value);

	void deserialize(std::fstream* file);
	void serialize(std::string& path, bool debug = false);

	uint32_t unique_frames;
	bool ready = false;

private:
	std::map<uint32_t, controller> _map;
	uint32_t _offset;
	void get_button(std::string* str, int* mask, uint8_t input, bool inverse);
};