cmake_minimum_required(VERSION 3.23)
if(POLICY CMP0149)
    cmake_policy(SET CMP0149 NEW)
endif()

include(cmake/version.cmake)
desktop_app_parse_version(whmx-assistant/build/VERSION)
set(desktop_app_homepage_url "https://github.com/MAWHA/maa-whmx")
set(desktop_app_uuid "{96D62A1E-D713-4DD9-83EB-D8511620B135}")

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

set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")

find_package(spdlog CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)

set(MAGIC_ENUM_OPT_BUILD_EXAMPLES OFF)
set(MAGIC_ENUM_OPT_BUILD_TESTS OFF)
set(MAGIC_ENUM_OPT_INSTALL OFF)
add_subdirectory(deps/magic_enum)

set(ELAWIDGETTOOLS_BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
add_subdirectory(deps/ElaWidgetTools)

set(QWINDOWKIT_INSTALL OFF)
set(QWINDOWKIT_BUILD_STATIC ON)
set(QWINDOWKIT_BUILD_EXAMPLES OFF)
add_subdirectory(deps/qwindowkit)

set(QMSETUP_APPLOCAL_DEPS_PATHS_DEBUG "${CMAKE_BUILD_DIR}/vcpkg_installed/x64-windows/debug/bin")
set(QMSETUP_APPLOCAL_DEPS_PATHS_RELEASE "${CMAKE_BUILD_DIR}/vcpkg_installed/x64-windows/release/bin")

set(VCPKG_PACKAGE_DIR "${CMAKE_SOURCE_DIR}/vcpkg/packages")
file(GLOB VCPKG_INSTALLED_PACKAGES RELATIVE ${VCPKG_PACKAGE_DIR} ${VCPKG_PACKAGE_DIR}/*)
foreach(package ${VCPKG_INSTALLED_PACKAGES})
    list(APPEND QMSETUP_APPLOCAL_DEPS_PATHS_DEBUG "${VCPKG_PACKAGE_DIR}/${package}/debug/bin")
    list(APPEND QMSETUP_APPLOCAL_DEPS_PATHS_RELEASE "${VCPKG_PACKAGE_DIR}/${package}/bin")
endforeach()

set(QMSETUP_MODULES_DIR "${CMAKE_SOURCE_DIR}/deps/qwindowkit/qmsetup/cmake")
find_package(qmsetup REQUIRED)

add_subdirectory(deps/FileLocksmithLibInterop)
add_subdirectory(deps/qt-material-widgets)
add_subdirectory(whmx-assistant)
add_subdirectory(launcher)

add_dependencies(launcher whmx-assistant)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT whmx-assistant)
