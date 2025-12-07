#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

// Windows
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <shlobj_core.h>

// DLL
#include "panel.h"
#include "utils.h"
//#include "d3d11/d3d11hook.h"

// STL
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#define _USE_MATH_DEFINES
#include <cmath>

// Dear ImGui..
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
//#include "imgui_impl_dx9.h"
//#include "imgui_impl_dx10.h"
#include "imgui_impl_dx11.h"
//#include "imgui_impl_dx12.h"
//#include "imgui_impl_opengl3.h"
//#include "imgui_impl_vulkan.h"
