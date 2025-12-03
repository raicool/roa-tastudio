#pragma once

#include <Windows.h>

namespace Menu 
{
    void InitializeContext(HWND hwnd);
    void Render();

    inline bool bShowMenu = true;
    inline bool bClosing = false;
} // namespace Menu
