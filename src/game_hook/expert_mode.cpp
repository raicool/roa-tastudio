#include "pch.h"

#include "expert_mode.h"

namespace expert_mode
{
	void (*expert_mode_playback_read_frame)();
	void __declspec(naked) detour_expert_mode_playback_read_frame()
	{
		__asm
		{
			//
			// hooks into a subroutine that is called to read from the input playback buffer
			//
			// eax = playback buffer string ptr
			// ebx = some scoped variable gamemaker uses?
			// ecx =
			// edx = playback buffer size?
			// esi = current playback buffer offset
			// edi = playback buffer size
			//
_playback_read_frame_enter_detour:
			push edx //< clear registers to stack
			push ebx //

			cmp esi, 0
			jne _playback_read_frame_enter_detour_continue
			mov[expert_mode::current_frame], 0

_playback_read_frame_enter_detour_continue:
			mov ebx, eax
			mov eax, DWORD PTR [expert_mode::custom_playback_inputs_data]
			test eax, eax
			je _playback_read_frame_restore_original
			cmp edi, 0x40000 // edi is set to 0x40000 when playback is started from a state load?
			je _playback_read_frame_restore_original
			mov dl, expert_mode::overwrite //< set to 1 if the playback file was just overwritten from editor
			cmp edx, 0
			jne _playback_read_frame_reset_offset
			jmp _playback_read_frame_overwrite

_playback_read_frame_exit_detour:
			pop ebx //< restore from stack
			pop edx //
			jmp expert_mode_playback_read_frame //< trampoline func

_playback_read_frame_reset_offset:
			xor esi, esi
			mov expert_mode::overwrite, 0x00
			jmp _playback_read_frame_overwrite

_playback_read_frame_overwrite:
			mov[ebp + 0x04], eax //< overwrite playback buffer ptr at ebp offset
			mov[ebp + 0x0c], eax //<
			mov ebx, expert_mode::custom_playback_inputs_size
			mov[ebp + 0x08], ebx //< overwrite playback buffer size at ebp offset
			mov[ebp + 0x10], ebx //<

			add eax, esi
			mov[esp + 0x08], eax //< overwrite playback buffer ptr in stack
			mov[esp + 0x18], eax //<
			
			mov edi, expert_mode::custom_playback_inputs_size
			jne _playback_read_frame_exit_detour

_playback_read_frame_restore_original:
			mov eax, ebx
			jmp _playback_read_frame_exit_detour
		}
	}

	void (*expert_mode_playback_update)();
	void __declspec(naked) detour_expert_mode_playback_update()
	{
		__asm
		{
_playback_update_enter_detour:
			push esi
			mov esi, expert_mode::current_frame
			inc esi
			mov[expert_mode::current_frame], esi
			jmp _playback_update_exit_detour

_playback_update_exit_detour:
			pop esi
			jmp expert_mode_playback_update
		}
	}
	uint32_t playback_update_addr = 0;
	uint32_t playback_read_frame_addr = 0;

	void init_hooks(uint32_t base)
	{
		//
		// hook expert_mode_playback_update
		//

		// instructions near expert_mode update subroutine
		std::vector<uint8_t> update_subr_pat =
		{
			0x8b, 0x86, 0x6c, 0x28, 0x00, 0x00, // mov ebp, dword ptr [esi + 0x65b8]
			0x48,                               // dec eax
			0xa9, 0xfc, 0xff, 0xff, 0x00,        // test eax, 0x00fffffc
			0x75, 0x0f,
			0x8d, 0x86, '?', '?', '?', '?',
			0x50,
			0xe8, '?', '?', '?', '?',
			0x83, 0xc4, 0x04,
			0x83, 0xc4, 0x18
		};

		playback_update_addr = (uint32_t)loader_search_memory(update_subr_pat);
		if (!playback_update_addr)
		{
			loader_log_error("could not find the expert_mode update subroutine!");
		}
		else
		{
			playback_update_addr += 0x400000;
			playback_update_addr += 0x3;
			loader_hook_create(reinterpret_cast<void**>(playback_update_addr), &detour_expert_mode_playback_update, reinterpret_cast<void**>(&expert_mode_playback_update));
		}

		//
		// hook expert_mode_playback_read_frame
		//

		// instructions near expert_mode read frame subroutine
		std::vector<uint8_t> read_frame_subr_pat =
		{
			0xc7, 0x86, 0xe8, 0x08, 0x00, 0x00, 0xeb, 0x00, 0x00, 0x00, // mov dword ptr [esi + 0x????], 0xeb
			0x50,                                                       // push eax
			0xe8, '?', '?', '?', '?',                                   // call 0x00401680 (builtin gml function, func address may move)
			0x83, 0xc4, 0x04,                                           // add esp, 0x4
			0x8b, 0x86, 0x44                                            // mov eax, dword ptr [esi + 0x244] (this is a struct offset, prob will change /shrug)
		};
		
		playback_read_frame_addr = (uint32_t)loader_search_memory(read_frame_subr_pat);
		if (!playback_read_frame_addr)
		{
			loader_log_error("could not find the expert_mode read frame subroutine!");
		}
		else
		{
			playback_read_frame_addr += 0x400000;
			playback_read_frame_addr += 0x4;
			loader_hook_create(reinterpret_cast<void**>(playback_read_frame_addr), &detour_expert_mode_playback_read_frame, reinterpret_cast<void**>(&expert_mode_playback_read_frame));
		}
	}

	void enable_hooks()
	{
		if (playback_update_addr)
		{
			loader_hook_enable(reinterpret_cast<void**>(playback_update_addr));
		}

		if (playback_read_frame_addr)
		{
			loader_hook_enable(reinterpret_cast<void**>(playback_read_frame_addr));
		}
	}

	void disable_hooks()
	{
		if (playback_update_addr)
		{
			loader_hook_disable(reinterpret_cast<void**>(playback_update_addr));
		}

		if (playback_read_frame_addr)
		{
			loader_hook_disable(reinterpret_cast<void**>(playback_read_frame_addr));
		}
	}
}