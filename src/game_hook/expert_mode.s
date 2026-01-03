
global detour_expert_mode_playback_update
global detour_expert_mode_playback_read_frame

#
# hooks into a subroutine that is called to read from the input playback buffer
#
# eax = playback buffer string ptr
# ebx = some scoped variable gamemaker uses?
# ecx =
# edx = playback buffer size?
# esi = current playback buffer offset
# edi = playback buffer size
#
_playback_read_frame_enter_detour:
	push edx #< clear registers to stack
	push ebx #

	cmp esi, 0
	jne _playback_read_frame_enter_detour_continue
	mov[expert_mode::current_frame], 0

_playback_read_frame_enter_detour_continue:
	mov ebx, eax
	mov eax, DWORD PTR [expert_mode::custom_playback_inputs_data]
	test eax, eax
	je _playback_read_frame_restore_original
	cmp edi, 0x40000 # edi is set to 0x40000 when playback is started from a state load?
	je _playback_read_frame_restore_original
	mov dl, expert_mode::overwrite #< set to 1 if the playback file was just overwritten from editor
	cmp edx, 0
	jne _playback_read_frame_reset_offset
	jmp _playback_read_frame_overwrite

_playback_read_frame_exit_detour:
	pop ebx #< restore from stack
	pop edx #
	jmp expert_mode_playback_read_frame #< trampoline func

_playback_read_frame_reset_offset:
	xor esi, esi
	mov expert_mode::overwrite, 0x00
	jmp _playback_read_frame_overwrite

_playback_read_frame_overwrite:
	mov[ebp + 0x04], eax #< overwrite playback buffer ptr at ebp offset
	mov[ebp + 0x0c], eax #<
	mov ebx, expert_mode::custom_playback_inputs_size
	mov[ebp + 0x08], ebx #< overwrite playback buffer size at ebp offset
	mov[ebp + 0x10], ebx #<

	add eax, esi
	mov[esp + 0x08], eax #< overwrite playback buffer ptr in stack
	mov[esp + 0x18], eax #<
			
	mov edi, expert_mode::custom_playback_inputs_size
	jne _playback_read_frame_exit_detour

_playback_read_frame_restore_original:
	mov eax, ebx
	jmp _playback_read_frame_exit_detour

_playback_update_enter_detour:
	push esi
	mov esi, expert_mode::current_frame
	inc esi
	mov[expert_mode::current_frame], esi
	jmp _playback_update_exit_detour

_playback_update_exit_detour:
	pop esi
	jmp expert_mode_playback_update