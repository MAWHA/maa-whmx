cmake_minimum_required(VERSION 3.23)
if(POLICY CMP0149)
    cmake_policy(SET CMP0149 NEW)
endif()

include(cmake/version.cmake)
desktop_app_parse_version(whmx-assistant/build/VERSION)
set(desktop_app_homepage_url "https://github.com/MAWHA/maa-whmx")

project(maa-whmx
LANGUAGES C CXX
VERSION ${desktop_app_version_cmake}
HOMEPAGE_URL ${desktop_app_homepage_url}
DESCRIPTION "Wuhuamixin assistant powered by MAA framework."
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/nice_target_sources.cmake)
include(cmake/nice_install_files.cmake)
include(cmake/install_agents.cmake)
include(cmake/config_maafw.cmake)
include(cmake/config_opencv.cmake)

set(MAGIC_ENUM_OPT_BUILD_EXAMPLES OFF)
set(MAGIC_ENUM_OPT_BUILD_TESTS OFF)
set(MAGIC_ENUM_OPT_INSTALL OFF)
add_subdirectory(deps/magic_enum)

add_subdirectory(deps/FileLocksmithLibInterop)
add_subdirectory(deps/Qt6)
add_subdirectory(deps/qt-material-widgets)
add_subdirectory(whmx-assistant)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT whmx-assistant)
