cmake_minimum_required(VERSION 3.23)
if(POLICY CMP0149)
    cmake_policy(SET CMP0149 NEW)
endif()

include(cmake/version.cmake)
desktop_app_parse_version(whmx-assistant/build/VERSION)

project(maa-whmx
LANGUAGES C CXX
VERSION ${desktop_app_version_cmake}
DESCRIPTION "Wuhuamixin assistant powered by MAA framework."
HOMEPAGE_URL "https://github.com/MAWHA/maa-whmx"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/nice_target_sources.cmake)
include(cmake/nice_install_files.cmake)
include(cmake/install_agents.cmake)
include(cmake/config_maafw.cmake)
include(cmake/config_opencv.cmake)

add_subdirectory(whmx-assistant)
