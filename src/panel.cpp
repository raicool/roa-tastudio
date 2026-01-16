#include "pch.h"

#include "parser.h"
#include "panel.h"
#include "game_hook/expert_mode.h"

#include <loader/yyc.h>
#include <loader/d3d11_hook.h>

#include <imgui_impl_win32.h>

constexpr uint8_t column_size = 20;
constexpr uint32_t table_transparency = 0x40000000;

enum table_pallette
{
	// last 2 least significant bits are omitted and overwritten by table_transparency
	// abgr8888
	REGULAR_FRAME = 0x002bde52,
	UNIQUE_FRAME = 0x0051b05f,
	CURRENT_FRAME = 0x00f1b05f,

	ANALOG_HOVERED = 0x003bfe62
};

void panel_init()
{
	ImGui::CreateContext();
	HWND window = loader_get_window();
	ImGui_ImplWin32_Init(window);
}

void handle_wndproc(
	const HWND hWnd, 
	UINT uMsg, 
	WPARAM wParam, 
	LPARAM lParam
)
{
	LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
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

	for (base_panel* _panel : panels)
	{
		if (_panel->visible) _panel->render();
	}

	ImGui::Render();

	d3d_device_context->OMSetRenderTargets(1, &render_target, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

constexpr uint8_t column_count = 14;
const char* file_names[] =
{
	"",
	"Slot 1.input",
	"Slot 2.input",
	"Slot 3.input"
};

static uint32_t cell;
void tastudio::input_table()
{
	constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_HighlightHoveredColumn;
	constexpr uint8_t column_count = 23;

	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Button, 0x00000000);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0x40ff7f00);

	if (ImGui::BeginTable("inputs", column_count, flags))
	{
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 50);
		ImGui::TableSetupColumn("Analog", ImGuiTableColumnFlags_WidthFixed, column_size + 40);


		ImGui::TableSetupColumn("Neutral", ImGuiTableColumnFlags_WidthFixed, column_size);
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
		ImGui::TableHeadersRow();

		cell = 0;
		uint32_t unique_frames = 0;
		uint32_t frame_color;


		controller dummy = { 0x00 };
		buffer = &dummy;
		controller buffer_xor = { 0x00 };

		ImGuiListClipper clipper;
		clipper.Begin(parser.last_frame_number() - parser.frame_offset(), column_size);

		while (clipper.Step())
		{
			for (current_frame = parser.frame_offset() + clipper.DisplayStart; current_frame <= parser.last_frame_number() - 1; current_frame++)
			{
				if (current_frame > parser.frame_offset() + clipper.DisplayEnd) break;

				if (unique_frames < parser.frame_count())
				{
					//if (buffer) buffer->unique = false;

					if (parser.is_unique(current_frame))
					{
						unique_frames++;

						// copy last frame
						if (buffer) buffer_xor = *buffer;

						buffer = parser.get_frame(current_frame);

						// get input differences from last frame
						buffer_xor ^= *buffer;

						// set row bg color to UNIQUE_FRAME
						frame_color = (UNIQUE_FRAME & 0x00ffffff) | table_transparency;
					}
					else
					{
						buffer_xor = { 0 };

						/// set row bg color to REGULAR_FRAME
						frame_color = (REGULAR_FRAME & 0x00ffffff) | table_transparency;
					}

					if (current_frame == expert_mode::current_frame + parser.frame_offset())
					{
						frame_color = (CURRENT_FRAME & 0x00ffffff) | table_transparency;
					}

					ImGui::PushStyleColor(ImGuiCol_TableRowBg, frame_color);
					ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, frame_color);

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					frame_number(current_frame);
					input_analog(buffer, current_frame);
					input_button(e_input::NEUTRAL, buffer->neutral);
					input_button(e_input::ANALOG_MODIFIER, buffer->analog_modifier);
					input_button(e_input::ANALOG_HARD_UP, buffer->analog_hup);
					input_button(e_input::ANALOG_HARD_RIGHT, buffer->analog_hright);
					input_button(e_input::ANALOG_HARD_LEFT, buffer->analog_hleft);
					input_button(e_input::ANALOG_HARD_DOWN, buffer->analog_hdown);
					input_button(e_input::ANALOG_LIGHT_UP, buffer->analog_lup);
					input_button(e_input::ANALOG_LIGHT_RIGHT, buffer->analog_lright);
					input_button(e_input::ANALOG_LIGHT_LEFT, buffer->analog_lleft);
					input_button(e_input::ANALOG_LIGHT_DOWN, buffer->analog_ldown);

					input_button(e_input::ATTACK, buffer->attack);
					input_button(e_input::SPECIAL, buffer->special);
					input_button(e_input::STRONG, buffer->strong);
					input_button(e_input::PARRY, buffer->parry);
					input_button(e_input::JUMP, buffer->jump);
					input_button(e_input::TAUNT, buffer->taunt);
					input_button(e_input::CSTICK_UP, buffer->cstick_up);
					input_button(e_input::CSTICK_RIGHT, buffer->cstick_right);
					input_button(e_input::CSTICK_LEFT, buffer->cstick_left);
					input_button(e_input::CSTICK_DOWN, buffer->cstick_down);
					input_button(e_input::TAP_JUMP, buffer->tap_jump);

					ImGui::PopStyleColor(2);
				}
			}
		}

		ImGui::EndTable();
	}

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar();
}

int32_t selected_file = 0;
bool should_open_add_frames_popup = false;

void tastudio::render()
{
	ImGui::Begin("TAStudio", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::MenuItem("Save File", 0, false, parser.ready))
		{
			parser.serialize(current_file_directory);
		}

		if (ImGui::MenuItem("Edit", 0, false, parser.ready))
		{
			ImGui::OpenPopup("FileEditMenu");
		}

		if (ImGui::BeginPopup("FileEditMenu"))
		{
			ImGui::MenuItem(current_file_directory.c_str(), 0, false, false);

			ImGui::Separator();

			if (ImGui::MenuItem("Add # of frames"))
			{
				static int32_t _frames_popup_selection = 0;
				static uint32_t _frames_position = analog_ctx ? analog_ctx->frame : 0;
				static uint32_t _frame_amount = 10;

				//ImGui::CloseCurrentPopup();
				should_open_add_frames_popup = true;
			}

			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			ImGui::EndPopup();
		}

		ImGui::EndMenuBar();
	}

	if (should_open_add_frames_popup)
	{
		should_open_add_frames_popup = false;
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
			case 1: _frames_position = parser.frame_offset(); break;
			case 2: _frames_position = parser.last_frame_number(); break;
			}
		}

		ImGui::InputScalar("Frame", ImGuiDataType_U32, &_frames_position, nullptr, nullptr, nullptr, _frames_popup_selection != 0 ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None);
		ImGui::InputScalar("Amount", ImGuiDataType_U32, &_frame_amount, nullptr, nullptr, nullptr, _frames_popup_selection != 0 ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None);

		if (ImGui::Button("Add"))
		{
			parser.add_frame(_frames_position, _frame_amount);
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}


	ImGui::Text("expert_mode::current_frame = %i", expert_mode::current_frame);
	ImGui::Text("map size = %i", parser.frame_count());

	static int selected_file;
	if (ImGui::Combo("Recording File", &selected_file, file_names, IM_ARRAYSIZE(file_names)))
	{
		if (selected_file <= 0)
		{
			ImGui::End();
			return;
		}

		if (current_file) current_file.close();

		wchar_t* appdata = 0;
		if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &appdata) == S_OK) {
			char dest[MAX_PATH];
			wcstombs(dest, appdata, MAX_PATH);

			current_file_directory = std::format("{}\\RivalsofAether\\training\\{}", dest, file_names[selected_file]);
			current_file.open(current_file_directory, std::ios::in | std::ios::out);
			if (!current_file)
			{
				loader_log_warn("file stream couldnt be opened correctly.");
			}
			else
			{
				loader_log_trace("Loaded file {}", current_file_directory.c_str());
				parser.deserialize(&current_file);
				analog_ctx = nullptr;
			}
		}
	}

	if (parser.ready)
	{
		if (ImGui::BeginTable("table", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextColumn();
			input_table();
			ImGui::TableNextColumn();

			if (analog_ctx)
			{
				if (ImGui::BeginTable("analog_edit", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_HighlightHoveredColumn))
				{
					ImGui::TableNextColumn();
					ImGui::Text("Frame: %i", analog_ctx->frame);

					ImGui::DebugDrawCursorPos();

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
						analog_ctx->neutral = true;
						analog_ctx->angle = atan2(pos.y - io.MousePos.y, pos.x - io.MousePos.x) * -180 / M_PI;
						analog_ctx->angle = (analog_ctx->angle + 180) % 360;

						if (io.MouseDownDuration[1] == 0.0f)
						{
							// Toggle angle snapping mode
							should_snap ^= 1;
						}

						// If right click down, snap control stick angle to nearest 45 degree angle
						if (should_snap)
						{
							ImGui::Text("Snapping");
							uint32_t snapped = analog_ctx->angle % 45;
							snapped < 22.5 ? analog_ctx->angle -= snapped : analog_ctx->angle -= snapped - 45;
						}
					}
					else
					{
						should_snap = false;
					}

					ImGui::Text("Angle: %i", analog_ctx->angle);

					const float angle_rad = analog_ctx->angle * M_PI / -180;

					ImVec2 line_end = ImVec2(cosf(angle_rad) * size, sinf(angle_rad) * size);
					line_end.x += pos.x;
					line_end.y += pos.y;

					// Render control stick display
					ImGui::GetWindowDrawList()->AddCircle(pos, size, 0x7fff7f00, 16, 2);
					ImGui::GetWindowDrawList()->AddCircleFilled(pos, size, 0x40ff7f00, 16);
					ImGui::GetWindowDrawList()->AddCircle(pos, size * 0.3, 0x7ff00ff7, 16, 2);
					ImGui::GetWindowDrawList()->AddLine(pos, line_end, should_snap ? 0xffff7f00 : 0xffffffff, 5);

					ImGui::TableNextColumn();
// 					ImGui::Checkbox("Neutral", &analog_ctx->neutral);
// 					ImGui::Checkbox("Modifier", &analog_ctx->analog_modifier);
// 					ImGui::Checkbox("Hard Up", &analog_ctx->analog_hup);
// 					ImGui::Checkbox("Hard Right", &analog_ctx->analog_hright);
// 					ImGui::Checkbox("Hard Left", &analog_ctx->analog_hleft);
// 					ImGui::Checkbox("Hard Down", &analog_ctx->analog_hdown);
// 					ImGui::Checkbox("Light Up", &analog_ctx->analog_lup);
// 					ImGui::Checkbox("Light Right", &analog_ctx->analog_lright);
// 					ImGui::Checkbox("Light Left", &analog_ctx->analog_lleft);
// 					ImGui::Checkbox("Light Down", &analog_ctx->analog_ldown);
					ImGui::EndTable();
				}
			}
			ImGui::EndTable();
		}
	}

	ImGui::End();
}

void tastudio::input_analog(controller* input, uint32_t frame)
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
		controller* new_frame = parser.set_frame(frame, *input);
		if (new_frame)
		{
			analog_ctx = new_frame;
			loader_log_trace(std::format("new input added at {}", analog_ctx->frame));
		}
		else
		{
			loader_log_warn("tastudio::input_analog(): new input could not be created.");
		}
	}

	ImGui::TableNextColumn();
	ImGui::PopID();
}

void tastudio::input_button(uint32_t input_id, bool down)
{
	cell++;
	ImGui::PushID(cell);

	const char* indicator = entry_dict[input_id].indicator;

	if (ImGui::Button(down == true ? indicator : " ", ImVec2(-FLT_MIN, column_size)))
	{
		parser.set_frame(current_frame, *buffer);
		parser.set_input_bool(current_frame, input_id, down ^ 0x01);
	}

	ImGui::TableNextColumn();
	ImGui::PopID();
}

void tastudio::frame_number(uint32_t frame)
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

			input_clipboard = *parser.get_frame(frame);
		}

		if (ImGui::MenuItem("Paste", "p", false, clipboard))
		{
			parser.set_frame(frame, input_clipboard, true);
		}

		if (ImGui::MenuItem("Add frame above"))
		{
			parser.add_frame(current_frame - 1, 1, { 0 });
		}

		if (ImGui::MenuItem("Add frame below"))
		{
			parser.add_frame(current_frame + 1, 1, { 0 });
		}

		if (ImGui::MenuItem("Unset"))
		{
			parser.erase_frame(frame);
		}

		if (ImGui::MenuItem("Delete"))
		{
			parser.delete_frame(frame);
		}
		ImGui::EndPopup();
	}

	ImGui::PopID();
	ImGui::TableNextColumn();
}


void metrics::render()
{
	ImGui::ShowMetricsWindow();
}


void roomview::render()
{
	if (ImGui::Begin("Room View"))
	{
		static size_t room_id;
		ImGui::InputScalar("room_id", ImGuiDataType_U32, &room_id);

		if (ImGui::Button("Set"))
		{
			current_room = loader_get_room_by_index(room_id);
		}

		if (current_room)
		{
			ImGui::Text("0x%08x", current_room);
			ImGui::InputInt("Width", &current_room->m_Width);
			ImGui::InputInt("Height", &current_room->m_Height);
			ImGui::Text("instance handle: 0x%08x", current_room->m_InstanceHandle);
// 			if (ImGui::TreeNode("Internals"))
// 			{
// 				CRoomInternal internals = current_room->GetMembers();
// 				ImGui::Text("name: %s", internals.m_Name);
// 				ImGui::TreePop();
// 			}
		}

		ImGui::End();
	}
}