cmake_minimum_required(VERSION 3.8.2)
project("roa-hook")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

file(GLOB_RECURSE CXX_SOURCE "src/*.cpp")
file(GLOB IMGUI_SOURCE 
    "vendor/imgui/imgui.cpp"
    "vendor/imgui/imgui_widgets.cpp"
    "vendor/imgui/imgui_tables.cpp"
    "vendor/imgui/imgui_draw.cpp"
    "vendor/imgui/imgui_demo.cpp"
    "vendor/imgui/backends/imgui_impl_win32.cpp"
    "vendor/imgui/backends/imgui_impl_dx9.cpp"
    "vendor/imgui/backends/imgui_impl_dx10.cpp"
    "vendor/imgui/backends/imgui_impl_dx11.cpp"
    "vendor/imgui/backends/imgui_impl_dx12.cpp"
    "vendor/imgui/backends/imgui_impl_opengl3.cpp"
    #"vendor/imgui/backends/imgui_impl_vulkan.cpp"
)

add_library("roa-hook" SHARED ${CXX_SOURCE} ${IMGUI_SOURCE})
target_include_directories("roa-hook" PRIVATE "src")
target_precompile_headers("roa-hook" PRIVATE "src/pch.h")

# vendors
# minhook
target_include_directories("roa-hook" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/minhook/include")
target_link_libraries("roa-hook" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/minhook/lib/libMinHook.x86.lib")

# roa-mod-loader
#add_subdirectory("vendor/roa-mod-loader")
#target_include_directories("roa-hook" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/roa-mod-loader/include")
#target_link_libraries("roa-hook" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/roa-mod-loader/lib/loader.lib")

# Vulkan SDK
#find_package(Vulkan REQUIRED)
#message(STATUS VULKAN: ${Vulkan_INCLUDE_DIRS})
#target_include_directories("roa-hook" PUBLIC ${Vulkan_INCLUDE_DIRS})
#target_link_libraries("roa-hook" PUBLIC ${Vulkan_LIBRARIES})

# imgui
target_include_directories("roa-hook" PUBLIC "vendor/imgui")
target_include_directories("roa-hook" PUBLIC "vendor/imgui/backends")