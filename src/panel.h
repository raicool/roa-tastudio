#pragma once

#include "parser.h"

#include "loader/log.h"
#include "loader/modpanel.h"

#include "GMLScriptEnv/gml.h"

#include <format>
#include <vector>
#include <fstream>

struct test_panel : mod_panel
{
	test_panel() : mod_panel() {};

	void render() override;
};

struct tastudio : mod_panel
{
	tastudio() : mod_panel() {};

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

struct roomview : mod_panel
{
	CRoom* current_room;

	void render() override;
};

struct metrics : mod_panel
{
	metrics() : mod_panel() {};

	void render() override;
};

inline std::vector<mod_panel*> panels;

template<typename T>
inline void add_panel()
{
	panels.emplace_back((mod_panel*)new T());
	loader_log_debug("new panel added at {}", panels.size());
}

// initializes d3d11 context variables
void panel_init();
void render_panels(ID3D11RenderTargetView*, IDXGISwapChain*);