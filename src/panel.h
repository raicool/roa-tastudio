#pragma once

#include "parser.h"

#include "loader/log.h"
#include "loader/modpanel.h"

#include "GMLScriptEnv/gml.h"

#include <format>
#include <vector>
#include <fstream>

struct test_panel : modpanel
{
	test_panel() : modpanel() {};

	void render() override;
};

struct tastudio : modpanel
{
	tastudio() : modpanel() {};

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

struct roomview : modpanel
{
	CRoom* current_room;

	void render() override;
};

struct metrics : modpanel
{
	metrics() : modpanel() {};

	void render() override;
};