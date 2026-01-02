#pragma once

namespace Utils
{
	HWND GetProcessWindow();
	void UnloadDLL();
	
	HMODULE GetCurrentImageBase();

	int GetCorrectDXGIFormat(int eCurrentFormat);
}