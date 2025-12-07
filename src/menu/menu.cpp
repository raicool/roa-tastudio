#include "menu.hpp"

#include "pch.h"

#include "loader/log.h"

#include "panel.h"
#include "hooks/hooks.hpp"
#include "utils/utils.hpp"

namespace Menu
{
    void InitializeContext(HWND hwnd)
    {
        if (ImGui::GetCurrentContext())
            return;

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(hwnd);

        ImGuiIO& io = ImGui::GetIO();
    }

    void Render() 
    {
        if (!bShowMenu)
            return;

        for (base_panel* panel : panels)
        {
            if (panel->visible)
                panel->render();
        }
    }
}