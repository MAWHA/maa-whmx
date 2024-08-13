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
add_subdirectory(launcher)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT whmx-assistant)

install(TARGETS whmx-assistant launcher
RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
)

install_agents(
${CMAKE_SOURCE_DIR}/deps/MaaFramework/share/MaaAgentBinary
${CMAKE_INSTALL_PREFIX}/agents
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/whmx-assistant/assets
DESTINATION ${CMAKE_INSTALL_PREFIX}
PATTERN .gitkeep EXCLUDE
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/whmx-assistant/config
DESTINATION ${CMAKE_INSTALL_PREFIX}
)

set(OCR_MODEL_DIR assets/general/model/ocr)
set(DEFAULT_OCR_MODEL ppocr_v4/zh_cn)
nice_install_files(${CMAKE_INSTALL_PREFIX}/${OCR_MODEL_DIR}
RELATIVE ${CMAKE_SOURCE_DIR}/whmx-assistant/${OCR_MODEL_DIR}/${DEFAULT_OCR_MODEL}
    det.onnx
    rec.onnx
    keys.txt
)

nice_install_files(${CMAKE_INSTALL_PREFIX}
ABSOLUTE
    $<REMOVE_DUPLICATES:$<TARGET_RUNTIME_DLLS:whmx-assistant>>
    $<REMOVE_DUPLICATES:$<TARGET_RUNTIME_DLLS:launcher>>
RELATIVE ${CMAKE_SOURCE_DIR}/deps/MaaFramework/bin
    onnxruntime_maa.dll
    opencv_world4_maa.dll
    fastdeploy_ppocr_maa.dll
    MaaAdbControlUnit.dll
)

add_custom_target(pre-deploy
COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --config $<CONFIG> --target install
)

add_custom_target(deploy
COMMAND
    windeployqt ${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:whmx-assistant>
    --verbose 0
    --no-quick-import
    --no-translations
    --no-libraries
DEPENDS pre-deploy
)

add_custom_target(post-deploy
COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_INSTALL_PREFIX} $<TARGET_FILE_DIR:whmx-assistant>
DEPENDS deploy
)

add_custom_target(deploy-all DEPENDS post-deploy)
