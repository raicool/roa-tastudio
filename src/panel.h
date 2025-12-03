#pragma once

#include "console/console.hpp"
#include "parser.h"

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
	void input_button(uint32_t input_id, bool down, bool just_pressed = false);
	void frame_number(uint32_t frame);
};

struct metrics : base_panel
{
	metrics() : base_panel() {};

	void render() override;
};

inline std::vector<base_panel*> panels;

template <typename T>
inline void add_panel()
{
	panels.emplace_back((base_panel*)new T());
	LOG("[!] new panel added at %i\n", panels.size());
}