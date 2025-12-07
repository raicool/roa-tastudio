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
        
        //io.IniFilename = io.LogFilename = nullptr;
    }

    void Render() 
    {
        if (!bShowMenu)
            return;

//         if (ImGui::BeginMainMenuBar())
//         {
//             if (ImGui::BeginMenu("Unhook From Rivals"))
//             {
//                 bClosing = true;
//                 Hooks::bShuttingDown = true;
// 
//                 ImGui::EndMenu();
//             }
//             ImGui::EndMainMenuBar();
//         }

        for (base_panel* panel : panels)
        {
            if (panel->visible)
                panel->render();
        }
    }
}