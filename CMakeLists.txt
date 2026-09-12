cmake_minimum_required(VERSION 3.20)

project(RavenXD_Launcher LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "RavenXD Launcher is Windows-only. It uses Win32, DXGI Desktop Duplication, Direct3D 11, and HLSL.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P STREQUAL "8")
    message(FATAL_ERROR "RavenXD Launcher is x64-only because the bundled FreeType import library is x64.")
endif()

set(RAVENXD_APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples/example_win32_directx11")

set(RAVENXD_SOURCES
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    misc/freetype/imgui_freetype.cpp
    backends/imgui_impl_dx11.cpp
    backends/imgui_impl_win32.cpp
    examples/example_win32_directx11/src/glass/backdrop.cpp
    examples/example_win32_directx11/src/glass/glass.cpp
    examples/example_win32_directx11/src/glass/surfaces.cpp
    examples/example_win32_directx11/src/main.cpp
    examples/example_win32_directx11/src/ravenxd_rpc.cpp
)

set(RAVENXD_HEADERS
    imconfig.h
    raven_launcher.h
    imgui.h
    imgui_internal.h
    backends/imgui_impl_dx11.h
    backends/imgui_impl_win32.h
    misc/freetype/imgui_freetype.h
    examples/example_win32_directx11/src/glass/backdrop.h
    examples/example_win32_directx11/src/glass/glass.h
    examples/example_win32_directx11/src/glass/shaders.h
    examples/example_win32_directx11/src/glass/spring.h
    examples/example_win32_directx11/src/glass/surfaces.h
    examples/example_win32_directx11/src/ravenxd_rpc.h
)

set(RAVENXD_INLINE_FILES
    examples/example_win32_directx11/src/app/config.inc
    examples/example_win32_directx11/src/app/config_apply.inc
    examples/example_win32_directx11/src/app/megasettings.inc
    examples/example_win32_directx11/src/app/pages.inc
        examples/example_win32_directx11/src/app/account_state.inc
    examples/example_win32_directx11/src/app/pages/account.inc
    examples/example_win32_directx11/src/app/pages/launchpad.inc
    examples/example_win32_directx11/src/app/ui_state.inc
)

set_source_files_properties(${RAVENXD_INLINE_FILES} PROPERTIES HEADER_FILE_ONLY ON)

add_executable(RavenXD_Launcher
    ${RAVENXD_SOURCES}
    ${RAVENXD_HEADERS}
    ${RAVENXD_INLINE_FILES}
)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT RavenXD_Launcher)

target_compile_features(RavenXD_Launcher PRIVATE cxx_std_17)

target_include_directories(RavenXD_Launcher PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/backends"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/freetype/include"
    "${RAVENXD_APP_DIR}/src"
)

target_compile_definitions(RavenXD_Launcher PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

if(MSVC)
    target_compile_options(RavenXD_Launcher PRIVATE
        /W4
        /utf-8
        $<$<CONFIG:Release>:/GS->
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
    )
endif()

target_link_libraries(RavenXD_Launcher PRIVATE
    d3d11
    d3dcompiler
    dxgi
    dwmapi
    windowscodecs
    ole32
    winmm
    user32
    gdi32
    shell32
    comdlg32
    advapi32
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/freetype/win64/freetype.lib"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
    ${RAVENXD_SOURCES}
    ${RAVENXD_HEADERS}
    ${RAVENXD_INLINE_FILES}
)

# Runtime resources used by the launcher.
add_custom_command(TARGET RavenXD_Launcher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:RavenXD_Launcher>/data/fa"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/data/fa/fa-solid-900.ttf"
        "$<TARGET_FILE_DIR:RavenXD_Launcher>/data/fa/fa-solid-900.ttf"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/data/fa/fa-brands-400.ttf"
        "$<TARGET_FILE_DIR:RavenXD_Launcher>/data/fa/fa-brands-400.ttf"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/data/uicons/uicons-regular-rounded.ttf"
        "$<TARGET_FILE_DIR:RavenXD_Launcher>/data/uicons/uicons-regular-rounded.ttf"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:RavenXD_Launcher>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/forge-1.8.9-11.15.1.2318-installer.jar"
        "$<TARGET_FILE_DIR:RavenXD_Launcher>/assets/forge-1.8.9-11.15.1.2318-installer.jar"
)
