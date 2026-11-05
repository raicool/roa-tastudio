#include "pch.h"

#include "parser.h"
#include "panel.h"
#include "savestate.h"

#include <loader/yyc.h>
#include <loader/d3d11_hook.h>

#include <imgui_impl_win32.h>

#define IS_MOUSE_EVENT(x) (x >= WM_MOUSEFIRST && x <= WM_MOUSELAST)
#define TOOLTIP(x, ...) if (ImGui::IsItemHovered()) ImGui::SetTooltip(x, __VA_ARGS__)

constexpr uint8_t column_size = 20;
constexpr uint8_t column_count = 23;
constexpr uint32_t table_transparency = 0x20000000;

extern std::array<InputParser, 10> parsers;
extern std::unordered_map<size_t, uint8_t> player_frame_states[10];

bool g_uiflag = true;
bool g_replay_mode = false;
bool g_frame_advance = false;
float g_playback_speed = 1.0;
float g_window_background_alpha = 0.75;

bool g_should_open_add_frames_popup = false;
bool g_follow_cursor = true;

uint32_t current_frame;
Controller input_clipboard;

Controller* buffer;

struct InputContext
{
	InputParser* parser;
	Controller input;
	bool ready = false;
};

InputContext analog_ctx; // used for the analog input editor

enum table_pallette
{
	// last 2 least significant bits are omitted and overwritten by table_transparency
	// abgr8888
	REGULAR_FRAME = IM_COL32(0x52, 0xde, 0x2b, 0x20),
	UNIQUE_FRAME = IM_COL32(0x5f, 0xb0, 0x51, 0x40),
	CURRENT_FRAME = IM_COL32(0x5f, 0xb0, 0xf1, 0x60),

	ANALOG_HOVERED = IM_COL32(0x62, 0xfe, 0x3b, 0x20)
};

void imgui_init()
{
	ImGui::CreateContext();
	HWND window = loader_get_window();
	ImGui_ImplWin32_Init(window);

	ImGuiStyle& style = ImGui::GetStyle();
	style.AntiAliasedLines = false;
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.058f, 0.058f, 0.058f, g_window_background_alpha);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.03f, 0.03f, 0.03f, g_window_background_alpha * 1.1);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, g_window_background_alpha * 1.5);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void handle_wndproc(
	const HWND hWnd,
	UINT& uMsg,
	WPARAM& wParam,
	LPARAM& lParam
)
{
	LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	if (g_uiflag)
	{
		// show cursor when ui is enabled, some rooms hide cursor by default
		HCURSOR cursor = LoadCursorA(0, IDC_ARROW);
		SetCursor(cursor);
	}

	if (uMsg == WM_KEYDOWN)
	{
		if (wParam == VK_F3)
		{
			g_uiflag ^= 1;
		}

		// Clear wndproc message so rivals doesnt register key event
		if (ImGui::GetIO().WantCaptureKeyboard)
		{
			uMsg = 0;
		}
	}

	if (IS_MOUSE_EVENT(uMsg))
	{
		// Clear wndproc message so rivals doesnt register mouse event
		if (ImGui::GetIO().WantCaptureMouse)
		{
			uMsg = 0;
		}
	}
}

void imgui_render()
{
	ImGuiID dockspace_id = ImGui::GetID("Dockspace");
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

	if (ImGui::Begin("TAStudio"))
	{
		static int selected_file;

		RValue* replay_mode = loader_get_global_variable("replay_mode");
		RValue* was_replay_mode = loader_get_global_variable("was_replay_mode");
		RValue* training_speed = loader_get_global_variable("training_speed");
		RValue* training_frame_stopped = loader_get_global_variable("training_frame_stopped");

		g_replay_mode = replay_mode->truthy();
		g_playback_speed = training_speed->get_real();
		g_frame_advance = training_frame_stopped->truthy();

		if (ImGui::Checkbox("Replay Mode", &g_replay_mode))
		{
			replay_mode->value_real = g_replay_mode ? 1 : 0;
			was_replay_mode->value_real = g_replay_mode ? 1 : 0;

			std::vector<CInstance*> pause_obj = loader_get_object_instances("obj_pause");

			if (pause_obj.size() > 0)
			{
				auto pause_selection = loader_get_instance_variable(pause_obj[0], "select");
				pause_selection->set_real(1);
			}
		}

		if (ImGui::Checkbox("Frame Advance", &g_frame_advance))
		{
			training_frame_stopped->set_real(g_frame_advance);
		}

		ImGui::Checkbox("Follow Cursor", &g_follow_cursor);

		if (ImGui::InputFloat("Playback Speed", &g_playback_speed, 0.25, 0.5, "%.2f"))
		{
			training_speed->set_real(g_playback_speed);
		}

		if (ImGui::Button("Save Inputs"))
		{
			const char* file = create_file();

			if (file)
			{
				std::fstream replay_file(file, std::ios::out);

				replay_file << "RTAS";

				for (auto& p : parsers)
				{
					p.serialize();
					replay_file << p.s_buffer << '\n';
				}

				replay_file.close();
			}
			else
			{
				loader_log_warn("could not open file to save inputs");
			}

			RValue* mfx_star = loader_call_builtin_ex("asset_get_index", nullptr, nullptr, { "mfx_star" });
			loader_call_builtin_ex("audio_play_sound", nullptr, nullptr, { mfx_star->get_real(), 0, 0, 0.5 });
		}

		if (ImGui::Button("Load Inputs"))
		{
			std::fstream replay_file = open_file_s(std::ios::in);

			if (replay_file)
			{
				std::string this_line;
				std::getline(replay_file, this_line);

				if (this_line == "RTAS")
				{
					uint8_t player_idx = 1;
					while (std::getline(replay_file, this_line))
					{
						if (player_idx > parsers.size())
						{
							break;
						}

						parsers[player_idx].deserialize(this_line.data());
						player_idx++;
					}
				}
			}
			else
			{
				loader_log_warn("could not open file to save load inputs");
			}
		}
	}
	ImGui::End();

	for (auto& p : parsers)
	{
		if (p.ready)
		{
			ImGui::PushID(&p);
			render_parser(&p);
			ImGui::PopID();
		}
	}

	if (analog_ctx.ready)
	{
		render_analog_context();
	}
}

void render_panels(
	ID3D11RenderTargetView* render_target, 
	IDXGISwapChain* swapchain, 
	ID3D11Device* d3d_device, 
	ID3D11DeviceContext* d3d_device_context
)
{
	if (!ImGui::GetIO().BackendRendererUserData)
	{
		ImGui_ImplDX11_Init(d3d_device, d3d_device_context);
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (g_uiflag)
	{
		imgui_render();
	}

	ImGui::Render();

	d3d_device_context->OMSetRenderTargets(1, &render_target, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

static uint32_t cell;
void render_parser(InputParser* parser)
{
	std::string window_name = std::format("Player {}", parser->player);

	ImGui::SetNextWindowSize({ 500, 500 }, ImGuiCond_FirstUseEver);
	if (ImGui::Begin(window_name.data(), nullptr, ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Edit", 0, false, parser->ready))
			{
				ImGui::OpenPopup("FileEditMenu");
			}

			if (ImGui::BeginPopup("FileEditMenu"))
			{
				if (ImGui::MenuItem("Add # of frames"))
				{
					static int32_t _frames_popup_selection = 0;
					static uint32_t _frames_position = analog_ctx.ready ? analog_ctx.input.frame : 0;
					static uint32_t _frame_amount = 10;

					g_should_open_add_frames_popup = true;
				}

				ImVec2 center = ImGui::GetMainViewport()->GetCenter();
				ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

				ImGui::EndPopup();
			}

			ImGui::EndMenuBar();
		}

		if (g_should_open_add_frames_popup)
		{
			g_should_open_add_frames_popup = false;
			ImGui::OpenPopup("AddFramesPopup");
		}

		if (ImGui::BeginPopupModal("AddFramesPopup"))
		{
			const char* combo_text[] =
			{
				"Custom",
				"First frame",
				"Last frame"
			};

			static int32_t _frames_popup_selection;
			static uint32_t _frames_position = 0;
			static uint32_t _frame_amount;

			if (ImGui::Combo("Position", &_frames_popup_selection, combo_text, IM_ARRAYSIZE(combo_text)))
			{
				switch (_frames_popup_selection)
				{
				case 1: _frames_position = 0; break;
				case 2: _frames_position = parser->last_frame_number(); break;
				}
			}

			ImGui::InputScalar("Frame", ImGuiDataType_U32, &_frames_position, nullptr, nullptr, nullptr, _frames_popup_selection != 0 ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None);
			ImGui::InputScalar("Amount", ImGuiDataType_U32, &_frame_amount, nullptr, nullptr, nullptr, _frames_popup_selection != 0 ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None);

			if (ImGui::Button("Add"))
			{
				parser->add_frame(_frames_position, _frame_amount);
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | 
			ImGuiTableFlags_HighlightHoveredColumn | ImGuiTableFlags_NoKeepColumnsVisible | ImGuiTableFlags_NoHostExtendX;

		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, 0x00000000);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0x40ff7f00);

		if (ImGui::BeginTable("##", column_count, flags))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 50);
			ImGui::TableSetupColumn("Analog", ImGuiTableColumnFlags_WidthFixed, column_size + 40);


			//ImGui::TableSetupColumn("Neutral", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Modifier", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Hard Up", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Hard Right", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Hard Left", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Hard Down", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Light Up", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Light Right", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Light Left", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Light Down", ImGuiTableColumnFlags_WidthFixed, column_size);

			ImGui::TableSetupColumn("Attack", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Special", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Strong", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Parry", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Jump", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Taunt", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Cstick_up", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Cstick_right", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Cstick_left", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Cstick_down", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("Tap Jump", ImGuiTableColumnFlags_WidthFixed, column_size);
			ImGui::TableSetupColumn("State");
			ImGui::TableHeadersRow();

			cell = 0;
			uint32_t unique_frames = 0;
			uint32_t frame_color;

			RValue* gameplay_time = loader_get_global_variable("gameplay_time");
			const int32_t game_elapsed = gameplay_time->get_s32() - 1;

			Controller dummy = { 0x00 };
			buffer = &dummy;
			Controller buffer_xor = { 0x00 };

			ImGuiListClipper clipper;
			clipper.Begin(parser->last_frame_number(), column_size);

			if (g_follow_cursor)
			{
				ImGui::SetScrollY(game_elapsed * column_size);
			}

			while (clipper.Step())
			{
				for (current_frame = clipper.DisplayStart; current_frame < parser->last_frame_number(); current_frame++)
				{
					if (current_frame > clipper.DisplayEnd) break;

					if (unique_frames < parser->frame_count())
					{
						//if (buffer) buffer->unique = false;

						if (parser->is_unique(current_frame))
						{
							unique_frames++;

							// copy last frame
							if (buffer) buffer_xor = *buffer;

							buffer = parser->get_frame(current_frame);

							// get input differences from last frame
							buffer_xor ^= *buffer;

							// set row bg color to UNIQUE_FRAME
							frame_color = UNIQUE_FRAME;
						}
						else
						{
							buffer_xor = { 0 };

							/// set row bg color to REGULAR_FRAME
							frame_color = REGULAR_FRAME;
						}

						if (current_frame == game_elapsed)
						{
							frame_color = CURRENT_FRAME;
						}

						ImGui::PushStyleColor(ImGuiCol_TableRowBg, frame_color);
						ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, frame_color);

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);

						frame_number(parser, current_frame);
						input_analog(parser, buffer, current_frame);
						//input_button(parser, InputTypeEnum::NEUTRAL, buffer->neutral);
						input_button(parser, InputTypeEnum::ANALOG_MODIFIER, buffer->analog_modifier);
						input_button(parser, InputTypeEnum::ANALOG_HARD_UP, buffer->analog_hup);
						input_button(parser, InputTypeEnum::ANALOG_HARD_RIGHT, buffer->analog_hright);
						input_button(parser, InputTypeEnum::ANALOG_HARD_LEFT, buffer->analog_hleft);
						input_button(parser, InputTypeEnum::ANALOG_HARD_DOWN, buffer->analog_hdown);
						input_button(parser, InputTypeEnum::ANALOG_LIGHT_UP, buffer->analog_lup);
						input_button(parser, InputTypeEnum::ANALOG_LIGHT_RIGHT, buffer->analog_lright);
						input_button(parser, InputTypeEnum::ANALOG_LIGHT_LEFT, buffer->analog_lleft);
						input_button(parser, InputTypeEnum::ANALOG_LIGHT_DOWN, buffer->analog_ldown);

						input_button(parser, InputTypeEnum::ATTACK, buffer->attack);
						input_button(parser, InputTypeEnum::SPECIAL, buffer->special);
						input_button(parser, InputTypeEnum::STRONG, buffer->strong);
						input_button(parser, InputTypeEnum::PARRY, buffer->parry);
						input_button(parser, InputTypeEnum::JUMP, buffer->jump);
						input_button(parser, InputTypeEnum::TAUNT, buffer->taunt);
						input_button(parser, InputTypeEnum::CSTICK_UP, buffer->cstick_up);
						input_button(parser, InputTypeEnum::CSTICK_RIGHT, buffer->cstick_right);
						input_button(parser, InputTypeEnum::CSTICK_LEFT, buffer->cstick_left);
						input_button(parser, InputTypeEnum::CSTICK_DOWN, buffer->cstick_down);
						input_button(parser, InputTypeEnum::TAP_JUMP, buffer->tap_jump);
						state_string(parser->player, current_frame);

						ImGui::PopStyleColor(2);
					}
				}
			}

			ImGui::EndTable();
		}

		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();
	}
	ImGui::End();
}

void render_analog_context()
{
	if (ImGui::Begin("Analog"))
	{
		if (ImGui::BeginTable("##", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_HighlightHoveredColumn))
		{
			ImGui::TableNextColumn();
			ImGui::Text("Player: %i", analog_ctx.parser->player);
			ImGui::Text("Frame: %i", analog_ctx.input.frame);

			const float size = 96;

			ImGuiStyle& style = ImGui::GetStyle();
			ImGuiIO& io = ImGui::GetIO();

			// Control stick interaction
			ImGui::InvisibleButton("##", ImVec2(size * 2, size * 2));
			if (ImGui::IsItemHovered())
			{
				ImGui::GetForegroundDrawList()->AddCircle(io.MousePos, 8, 0xff007fff, 16, 2);
			}

			ImVec2 pos = ImGui::GetCursorScreenPos();
			pos.x += size;
			pos.y -= size;
			pos.y -= style.FramePadding.y;

			static bool should_snap;
			if (ImGui::IsItemActive())
			{
				analog_ctx.input.unique = true;
				analog_ctx.input.neutral = true;
				analog_ctx.input.has_angle = true;
				analog_ctx.input.angle = atan2(pos.y - io.MousePos.y, pos.x - io.MousePos.x) * -180 / M_PI;
				analog_ctx.input.angle = (analog_ctx.input.angle + 180) % 360;

				if (io.MouseDownDuration[1] == 0.0f)
				{
					// Toggle angle snapping mode
					should_snap ^= 1;
				}

				// If right click down, snap control stick angle to nearest 45 degree angle
				if (should_snap)
				{
					ImGui::Text("Snapping");
					uint32_t snapped = analog_ctx.input.angle % 45;
					snapped < 22.5 ? analog_ctx.input.angle -= snapped : analog_ctx.input.angle -= snapped - 45;

					analog_ctx.input.angle = snapped;
				}

				analog_ctx.parser->set_frame(analog_ctx.input.frame, analog_ctx.input, true);
			}
			else
			{
				should_snap = false;
			}

			ImGui::Text("Angle: %i", analog_ctx.input.angle);

			const float angle_rad = analog_ctx.input.angle * M_PI / -180;

			ImVec2 line_end = ImVec2(cosf(angle_rad) * size, sinf(angle_rad) * size);
			line_end.x += pos.x;
			line_end.y += pos.y;

			// Render control stick display
			ImGui::GetWindowDrawList()->AddCircle(pos, size, 0x7fff7f00, 16, 2);
			ImGui::GetWindowDrawList()->AddCircleFilled(pos, size, 0x40ff7f00, 16);
			ImGui::GetWindowDrawList()->AddCircle(pos, size * 0.3, 0x7ff00ff7, 16, 2);
			ImGui::GetWindowDrawList()->AddLine(pos, line_end, should_snap ? 0xffff7f00 : 0xffffffff, 5);

			ImGui::TableNextColumn();
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

void input_analog(InputParser* parser, Controller* input, uint32_t frame)
{
	cell++;
	ImGui::PushID(cell);
	ImGui::TableGetColumnIndex();

	// begin drawing analog stick
	ImGuiTable* table = ImGui::GetCurrentContext()->CurrentTable;
	ImRect rect = ImGui::TableGetCellBgRect(table, 1);

	const float size = rect.GetHeight() * 0.5;

	ImGui::GetWindowDrawList()->AddCircleFilled(rect.GetCenter(), size, 0x7fff7f00, 8);

	if (input->has_angle)
	{
		const float angle_rad = input->angle * M_PI / -180;
		ImVec2 line_end = ImVec2(cosf(angle_rad) * size, sinf(angle_rad) * size);
		line_end.x += rect.GetCenter().x;
		line_end.y += rect.GetCenter().y;

		ImGui::GetWindowDrawList()->AddLine(rect.GetCenter(), line_end, 0xffffffff);
	}

	if (ImGui::Button("##", ImVec2(-FLT_MIN, column_size)))
	{
		Controller* new_frame = parser->set_frame(frame, *input);
		if (new_frame)
		{
			new_frame->neutral = true;
			analog_ctx.parser = parser;
			analog_ctx.input = *new_frame;
			analog_ctx.ready = true;
			loader_log_trace(std::format("selected input at {}", analog_ctx.input.frame));
		}
		else
		{
			loader_log_warn("input_analog(): new input could not be created.");
		}
	}

	ImGui::TableNextColumn();
	ImGui::PopID();
}

void input_button(InputParser* parser, uint32_t input_id, bool down)
{
	cell++;
	ImGui::PushID(cell);

	const char* indicator = input_entry_table[input_id].indicator;
	const char* name = input_entry_table[input_id].name;

	if (ImGui::Button(down == true ? indicator : " ", ImVec2(-FLT_MIN, column_size)))
	{
		parser->set_frame(current_frame, *buffer);
		parser->set_input_bool(current_frame, input_id, down ^ 0x01);

// 		auto state = SavestateManager::last_savestate(current_frame);
// 
// 		if (state.has_value())
// 		{
// 			auto NetworkManager = loader_get_object_instances("NetworkManager")[0];
// 
// 			RValue state_index = state.value();
// 			loader_call_script("gml_Script_LoadRollbackState", NetworkManager, NetworkManager, { &state_index });
// 		}
	}
	TOOLTIP(name);

	ImGui::TableNextColumn();
	ImGui::PopID();
}

void frame_number(InputParser* parser, uint32_t frame)
{
	cell++;
	ImGui::PushID(cell);
	ImGui::Button(std::format("{}", frame).c_str(), ImVec2(-FLT_MIN, column_size));

	if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
	{
		ImGui::OpenPopup("FrameActionMenu");
	}

	if (ImGui::BeginPopup("FrameActionMenu"))
	{
		ImGui::MenuItem(std::format("Frame {}", frame).c_str(), 0, false, false);
		ImGui::Separator();

		static bool clipboard;
		if (ImGui::MenuItem("Copy", "c"))
		{
			clipboard = true;

			input_clipboard = *parser->get_frame(frame);
		}

		if (ImGui::MenuItem("Paste", "p", false, clipboard))
		{
			parser->set_frame(frame, input_clipboard, true);
		}

		if (ImGui::MenuItem("Add frame above"))
		{
			parser->add_frame(current_frame - 1, 1, { 0 });
		}

		if (ImGui::MenuItem("Add frame below"))
		{
			parser->add_frame(current_frame + 1, 1, { 0 });
		}

		if (ImGui::MenuItem("Unset"))
		{
			parser->erase_frame(frame);
		}

		if (ImGui::MenuItem("Delete"))
		{
			parser->delete_frame(frame);
		}
		ImGui::EndPopup();
	}

	ImGui::PopID();
	ImGui::TableNextColumn();
}

void state_string(uint8_t player_id, uint32_t frame)
{
	cell++;
	ImGui::PushID(cell);

	if (player_frame_states[player_id].contains(frame))
	{
		RValue my_state = player_frame_states[player_id].at(frame);
		RValue* my_state_string = loader_call_script("gml_Script_get_state_name", nullptr, nullptr, { &my_state });

		if (my_state_string->value_stringref)
		{
			ImGui::Text(my_state_string->value_stringref->string);
		}
	}

	ImGui::PopID();
	ImGui::TableNextColumn();
}