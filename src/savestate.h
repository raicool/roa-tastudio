#pragma once

namespace SavestateManager
{
	void init_advance_frame_hook();

	inline std::map<uint32_t, RValue> _savestates;
	static uint32_t save_interval = 30;
	static uint32_t save_timer = 0;
	static uint8_t count = 0;

	static std::optional<RValue> last_savestate(uint32_t frame)
	{
		for (auto it = _savestates.rbegin(); it != _savestates.rend(); ++it)
		{
			if (it->first < frame)
			{
				return it->second;
			}
		}
		return {};
	}

	static uint32_t last_savestate_frame(uint32_t frame)
	{
		for (auto it = _savestates.rbegin(); it != _savestates.rend(); ++it)
		{
			if (it->first < frame)
			{
				return it->first;
			}
		}
		return 0;
	}
	
	static bool is_savestate_frame(uint32_t frame)
	{
		return _savestates.contains(frame);
	}

	static void delete_oldest_savestate()
	{
		_savestates.erase(_savestates.begin());
	}
}