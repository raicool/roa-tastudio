#pragma once

namespace expert_mode
{
	inline char* custom_playback_inputs_data;
	inline char* original_playback_inputs_data;
	inline size_t custom_playback_inputs_size;

	inline volatile bool overwrite;
	inline volatile uint32_t current_frame;

	void init_hooks(uint32_t base_ptr);
	void enable_hooks(uint32_t base_ptr);
	void disable_hooks(uint32_t base_ptr);
}