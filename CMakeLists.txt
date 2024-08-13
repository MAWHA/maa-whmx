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

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(cmake/nice_target_sources.cmake)
include(cmake/nice_install_files.cmake)
include(cmake/install_agents.cmake)
include(cmake/file_copy_target.cmake)
include(cmake/config_maafw.cmake)
include(cmake/config_opencv.cmake)
include(cmake/config_qt6.cmake)

# Get the library directory for the current platform
set(VCPKG_LIBRARY_ROOT "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
set(VCPKG_LIBRARY_DIR "${VCPKG_LIBRARY_ROOT}/bin")
set(VCPKG_DEBUG_LIBRARY_DIR "${VCPKG_LIBRARY_ROOT}/debug/bin")

# Print the library paths
message(STATUS "Vcpkg library directory: ${VCPKG_LIBRARY_DIR}")
message(STATUS "Vcpkg debug library directory: ${VCPKG_DEBUG_LIBRARY_DIR}")

set (OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")

set(MAGIC_ENUM_OPT_BUILD_EXAMPLES OFF)
set(MAGIC_ENUM_OPT_BUILD_TESTS OFF)
set(MAGIC_ENUM_OPT_INSTALL OFF)
add_subdirectory(deps/magic_enum)

add_subdirectory(deps/FileLocksmithLibInterop)
add_subdirectory(deps/qt-material-widgets)
add_subdirectory(whmx-assistant)
add_subdirectory(launcher)

add_dependencies(launcher whmx-assistant)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT whmx-assistant)
