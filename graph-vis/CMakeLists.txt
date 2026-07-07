
set(CIMGUI_DIR "${PROJECT_SOURCE_DIR}/vendor/cimgui")

# Backends compiled as C++ without CIMGUI_DEFINE_ENUMS_AND_STRUCTS — that flag causes
# cimgui.h to redefine imgui.h C++ enums as C typedef enums, producing redefinition errors
add_library(imgui_backends STATIC
    "${CIMGUI_DIR}/cimgui_impl.cpp"
    "${CIMGUI_DIR}/imgui/backends/imgui_impl_sdl3.cpp"
    "${CIMGUI_DIR}/imgui/backends/imgui_impl_sdlrenderer3.cpp")
target_compile_definitions(imgui_backends PRIVATE CIMGUI_USE_SDL3=1)
target_include_directories(imgui_backends PRIVATE
    "${CIMGUI_DIR}/imgui"
    "${CIMGUI_DIR}/imgui/backends"
)
target_link_libraries(imgui_backends PRIVATE cimgui SDL3::SDL3)
set_property(TARGET imgui_backends PROPERTY CXX_STANDARD 20)

file(GLOB_RECURSE GRAPH_VIS_SOURCES CONFIGURE_DEPENDS "*.c" "*.h")
add_executable(graph-vis ${GRAPH_VIS_SOURCES} "engine/context.h" "defs.h"  "engine/layout.h" "engine/cmd.c")
set_property(TARGET graph-vis PROPERTY C_STANDARD 17)
target_compile_definitions(graph-vis PRIVATE
    CIMGUI_DEFINE_ENUMS_AND_STRUCTS=1
    CIMGUI_USE_SDL3=1
)
target_include_directories(graph-vis PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(graph-vis PRIVATE cimgui imgui_backends SDL3::SDL3)
