#pragma once

enum RenderingBackend_t {
	NONE = 0,
	DIRECTX9,
	DIRECTX10,
	DIRECTX11,
	DIRECTX12,
	OPENGL,
	VULKAN,
};

namespace Utils
{
	HWND GetProcessWindow();
	void UnloadDLL();
	
	HMODULE GetCurrentImageBase();

	int GetCorrectDXGIFormat(int eCurrentFormat);
}