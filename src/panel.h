#pragma once

#include "parser.h"

#include "loader/log.h"

#include "GMLScriptEnv/gml.h"

#include <format>
#include <vector>
#include <fstream>

struct base_panel
{
	base_panel() : visible(true) {};

	bool visible; //< flag for if panel will be rendered or not

	virtual void render() = 0;
};

struct test_panel : base_panel
{
	test_panel() : base_panel() {};

	void render() override;
};

struct tastudio : base_panel
{
	tastudio() : base_panel() {};

	std::string current_file_directory;
	std::fstream current_file;
	input_parser parser;

	uint32_t current_frame;
	controller input_clipboard;

	controller* buffer;
	controller* analog_ctx = nullptr; // used for the analog input editor

	void render() override;
	void input_table();
	void input_analog(controller* input, uint32_t frame);
	void input_button(uint32_t input_id, bool down);
	void frame_number(uint32_t frame);
};

struct roomview : base_panel
{
	CRoom* current_room;

	void render() override;
};

struct metrics : base_panel
{
	metrics() : base_panel() {};

	void render() override;
};

inline std::vector<base_panel*> panels;

template<typename T>
inline void add_panel()
{
	panels.emplace_back((base_panel*)new T());
	loader_log_debug("new panel added at {}", panels.size());
}

// initializes d3d11 context variables
void panel_init();

void handle_wndproc(
	const HWND hWnd, 
	UINT uMsg, 
	WPARAM wParam, 
	LPARAM lParam
);

void render_panels(
	ID3D11RenderTargetView* render_target,
	IDXGISwapChain* swapchain,
	ID3D11Device* d3d_device,
	ID3D11DeviceContext* d3d_device_context
);