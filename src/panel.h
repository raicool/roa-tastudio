#pragma once

#include "parser.h"

#include "loader/log.h"

#include <format>
#include <vector>
#include <fstream>

void imgui_init();
void imgui_render();
void render_parser(InputParser* parser);
void render_analog_context();
void input_analog(InputParser* parser, Controller* input, uint32_t frame);
void input_button(InputParser* parser, uint32_t input_id, bool down);
void frame_number(InputParser* parser, uint32_t frame);
void state_string(uint8_t player_id, uint32_t frame);

void handle_wndproc(
	const HWND hWnd, 
	UINT& uMsg, 
	WPARAM& wParam, 
	LPARAM& lParam
);

void render_panels(
	ID3D11RenderTargetView* render_target,
	IDXGISwapChain* swapchain,
	ID3D11Device* d3d_device,
	ID3D11DeviceContext* d3d_device_context
);