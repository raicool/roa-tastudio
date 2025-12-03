#pragma once

#include <vector>
#include <iostream>

static DWORD base = (DWORD)GetModuleHandle(0);

/*
* goes through a multi-level pointer and returns the pointer from given offsets
*/
inline DWORD seek_ptr(DWORD process_base, std::vector<int> offsets)
{
	DWORD ptr = *(DWORD*)(process_base);

	//std::cout << std::hex << ptr << std::endl;

	for (int i = 0; i < offsets.size(); i++)
	{
		ptr = *(DWORD*)(ptr + offsets[i]);
	}

	return ptr;
}