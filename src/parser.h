#pragma once

#include <map>
#include <string>
#include <loader/yoyo.h>

enum InputTypeEnum
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

	InputTypeEnum_MAX
};

enum ParserTypeEnum
{
	TYPE_U32,
	TYPE_BOOLEAN,
};

struct InputEntry
{
	const char* name;
	const char* indicator;
	int mask;
	char id; //< enum e_input
	char type; //< enum e_type
};

constexpr InputEntry input_entry_table[] =
{
	{"Frame",              "",  0,           InputTypeEnum::FRAME,              ParserTypeEnum::TYPE_U32    },
	{"Angle",              "y", 0,           InputTypeEnum::ANGLE,              ParserTypeEnum::TYPE_U32    },
	{"Neutral",            "Z", (1 << (0)),  InputTypeEnum::NEUTRAL,            ParserTypeEnum::TYPE_BOOLEAN},
	{"Analog Modifier",    "N", (1 << (1)),  InputTypeEnum::ANALOG_MODIFIER,    ParserTypeEnum::TYPE_BOOLEAN},
	{"Analog Hard Up",     "P", (1 << (2)),  InputTypeEnum::ANALOG_HARD_UP,     ParserTypeEnum::TYPE_BOOLEAN},
	{"Analog Hard Right",  "I", (1 << (3)),  InputTypeEnum::ANALOG_HARD_RIGHT,  ParserTypeEnum::TYPE_BOOLEAN},
	{"Analog Hard Left",   "E", (1 << (4)),  InputTypeEnum::ANALOG_HARD_LEFT,   ParserTypeEnum::TYPE_BOOLEAN},
	{"Analog Hard Down",   "O", (1 << (5)),  InputTypeEnum::ANALOG_HARD_DOWN,   ParserTypeEnum::TYPE_BOOLEAN},
	{"Analog Light Up",    "U", (1 << (6)),  InputTypeEnum::ANALOG_LIGHT_UP,    ParserTypeEnum::TYPE_BOOLEAN},
	{"Analog Light Right", "R", (1 << (7)),  InputTypeEnum::ANALOG_LIGHT_RIGHT, ParserTypeEnum::TYPE_BOOLEAN},
	{"Analog Light Left",  "L", (1 << (8)),  InputTypeEnum::ANALOG_LIGHT_LEFT,  ParserTypeEnum::TYPE_BOOLEAN},
	{"Analog Light Down",  "D", (1 << (9)),  InputTypeEnum::ANALOG_LIGHT_DOWN,  ParserTypeEnum::TYPE_BOOLEAN},
	{"Attack",             "A", (1 << (10)), InputTypeEnum::ATTACK,             ParserTypeEnum::TYPE_BOOLEAN},
	{"Special",            "B", (1 << (11)), InputTypeEnum::SPECIAL,            ParserTypeEnum::TYPE_BOOLEAN},
	{"Strong",             "C", (1 << (12)), InputTypeEnum::STRONG,             ParserTypeEnum::TYPE_BOOLEAN},
	{"Parry",              "S", (1 << (13)), InputTypeEnum::PARRY,              ParserTypeEnum::TYPE_BOOLEAN},
	{"Jump",               "J", (1 << (14)), InputTypeEnum::JUMP,               ParserTypeEnum::TYPE_BOOLEAN},
	{"Taunt",              "T", (1 << (15)), InputTypeEnum::TAUNT,              ParserTypeEnum::TYPE_BOOLEAN},
	{"C-Stick Up",         "X", (1 << (16)), InputTypeEnum::CSTICK_UP,          ParserTypeEnum::TYPE_BOOLEAN},
	{"C-Stick Right",      "G", (1 << (17)), InputTypeEnum::CSTICK_RIGHT,       ParserTypeEnum::TYPE_BOOLEAN},
	{"C-Stick Left",       "F", (1 << (18)), InputTypeEnum::CSTICK_LEFT,        ParserTypeEnum::TYPE_BOOLEAN},
	{"C-Stick Down",       "W", (1 << (19)), InputTypeEnum::CSTICK_DOWN,        ParserTypeEnum::TYPE_BOOLEAN},
	{"Tap Jump?",          "M", (1 << (20)), InputTypeEnum::TAP_JUMP,           ParserTypeEnum::TYPE_BOOLEAN},
};

struct Controller
{
	bool unique; //< editor only, not saved to replay/input file
	bool has_angle;

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
	Controller& operator ^= (const Controller& rvalue)
	{
		raw ^= rvalue.raw;

		// these should be overwritten instead of xor'd
		analog_hdown = rvalue.analog_hdown;
		analog_hleft = rvalue.analog_hleft;
		analog_hright = rvalue.analog_hright;
		analog_hup = rvalue.analog_hup;

		return *this;
	}

	Controller& operator |= (const Controller& rvalue)
	{
		raw |= rvalue.raw;
		return *this;
	}

	bool is_zero() const
	{
		return raw == 0;
	}
};

class InputParser
{
public:
	Controller* get_frame(uint32_t frame_idx)
	{
		if (_map.contains(frame_idx) == false)
		{
			_map[frame_idx] = { 0 };
		}

		return &_map[frame_idx];
	}

	Controller* set_frame(uint32_t frame_idx, Controller frame, bool overwrite = false)
	{
		// set frame after to defaults if it doesnt exist already
		_init_frame(frame_idx + 1);

		if (_map.contains(frame_idx) && !overwrite)
		{
			_map[frame_idx].frame = frame_idx;
			return &_map[frame_idx];
		}

		_init_frame(frame_idx, frame, true);

		serialize();

		return &_map[frame_idx];
	}

	void _init_frame(uint32_t frame_idx, Controller frame = { 0 }, bool overwrite = false)
	{
		bool should_write = overwrite;

		if (_map.contains(frame_idx))
		{
			should_write |= _map[frame_idx].unique == false;
		}
		else
		{
			should_write = true;
		}

		if (should_write)
		{
			_map[frame_idx] = frame;
			_map[frame_idx].unique = true;
			_map[frame_idx].frame = frame_idx;
		}
	}

	void erase_frame(uint32_t frame_idx)
	{
		if (_map.contains(frame_idx))
		{
			_map[frame_idx] = { .neutral = true };
		}

		serialize();
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

		serialize();
	}

	Controller* add_frame(uint32_t frame_idx, uint32_t amount = 1, Controller default_input = { .neutral = true })
	{
		// initialize frame_idx if its not set in _map
		_init_frame(frame_idx);

		// set frame after to defaults if it doesnt exist already
		_init_frame(frame_idx + 1);

		_map.emplace(std::pair<uint32_t, Controller>(last_frame_number() + amount, default_input));

		for (auto it = _map.rbegin(); it != _map.rend(); it++)
		{
			if (it->first > frame_idx + amount)
			{
				_map[it->first] = _map[it->first - amount];
			}
			else
			{
				it->second = default_input;

				if (it->first <= frame_idx)
				{
					it->second.unique = false;
					return &it->second;
				}
			}
		}

		serialize();
	}

	bool is_unique(uint32_t frame)
	{
		return _map.contains(frame) ? _map[frame].unique : false;
	}

	uint32_t frame_count() const
	{
		return _map.size();
	}

	uint32_t last_frame_number() const
	{
		if (_map.empty() == false)
		{
			return _map.rbegin()->first;
		}
		return 0;
	}

	bool get_input_bool(uint32_t frame, uint8_t input_id);
	void set_input_bool(uint32_t frame, uint8_t input_id, bool value);
	void set_input_u32(uint32_t frame, uint8_t input_id, uint32_t value);

	void deserialize(char* replay_string);
	void serialize();

	std::string s_buffer;
	uint32_t unique_frames;
	uint8_t player;
	bool ready = false;

private:
	std::map<uint32_t, Controller> _map;
	bool get_button(char indicator, int* mask, uint8_t input, bool inverse);
};