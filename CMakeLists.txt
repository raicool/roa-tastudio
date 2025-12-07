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
    "vendor/imgui/backends/imgui_impl_dx11.cpp"
)

add_library("roa-hook" SHARED ${CXX_SOURCE} ${IMGUI_SOURCE})
target_include_directories("roa-hook" PRIVATE "src")
target_precompile_headers("roa-hook" PRIVATE "src/pch.h")

# vendors
# roa-mod-loader
add_subdirectory("vendor/roa-mod-loader")
target_link_libraries("roa-hook" PRIVATE roa-mod-loader)
target_include_directories("roa-hook" PRIVATE "vendor/roa-mod-loader/loader/include")

# imgui
target_include_directories("roa-hook" PRIVATE "vendor/imgui")
target_include_directories("roa-hook" PRIVATE "vendor/imgui/backends")