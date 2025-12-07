#include <Windows.h>
#include <thread>
#include <dxgi.h>

#include "utils.hpp"

#include "loader/log.h"

#define RB2STR(x) case x: return #x

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

static BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam) 
{
	const auto isMainWindow = [handle]()
	{
		return GetWindow(handle, GW_OWNER) == nullptr && IsWindowVisible(handle);
	};

	DWORD pID = 0;
	GetWindowThreadProcessId(handle, &pID);

	if (GetCurrentProcessId() != pID || !isMainWindow() || handle == GetConsoleWindow())
	{
		return TRUE;
	}

	*reinterpret_cast<HWND*>(lParam) = handle;

	return FALSE;
}

static DWORD WINAPI _UnloadDLL(LPVOID lpParam)
{
	FreeLibraryAndExitThread(Utils::GetCurrentImageBase(), 0);
	return 0;
}

namespace Utils 
{
	HWND GetProcessWindow()
	{
		HWND hwnd = nullptr;
		EnumWindows(::EnumWindowsCallback, reinterpret_cast<LPARAM>(&hwnd));

		while (!hwnd)
		{
			EnumWindows(::EnumWindowsCallback, reinterpret_cast<LPARAM>(&hwnd));
			loader_log_trace("waiting for window to appear.");
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		char name[128];
		GetWindowTextA(hwnd, name, RTL_NUMBER_OF(name));
		loader_log_trace(std::format("got window with name : '{}'", name));

		return hwnd;
	}

	void UnloadDLL()
	{
		HANDLE hThread = CreateThread(NULL, 0, _UnloadDLL, NULL, 0, NULL);
		if (hThread != NULL) CloseHandle(hThread);
	}

	HMODULE GetCurrentImageBase()
	{
		return (HINSTANCE)(&__ImageBase);
	}

	int GetCorrectDXGIFormat(int eCurrentFormat)
	{
		switch (eCurrentFormat)
		{
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		return eCurrentFormat;
	}
}
