add_executable(whmx-assistant)

get_filename_component(SOURCE_DIR src REALPATH)
nice_target_sources(whmx-assistant ${SOURCE_DIR}
PRIVATE
    Main.cpp
    Consts.h
    DeviceHelper.cpp
    DeviceHelper.h
    CustomAction/Research.cpp
    CustomAction/Research.h
    CustomRecognizer/Research.cpp
    CustomRecognizer/Research.h
)

target_link_libraries(whmx-assistant
PRIVATE
    MaaFramework::MaaPP
    OpenCV::OpenCV
    desktop-app::Qt6
)

add_custom_command(TARGET whmx-assistant POST_BUILD
COMMAND
    windeployqt ${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:whmx-assistant>
    --verbose 0
    --no-quick-import
    --no-translations
    --no-libraries
)

install(TARGETS whmx-assistant
RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
)

install_agents(
${CMAKE_SOURCE_DIR}/deps/MaaFramework/share/MaaAgentBinary
${CMAKE_INSTALL_PREFIX}/agents
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets
DESTINATION ${CMAKE_INSTALL_PREFIX}
PATTERN .gitkeep EXCLUDE
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/config
DESTINATION ${CMAKE_INSTALL_PREFIX}
)

nice_install_files(${CMAKE_INSTALL_PREFIX}
ABSOLUTE
    $<REMOVE_DUPLICATES:$<TARGET_RUNTIME_DLLS:whmx-assistant>>
RELATIVE ${CMAKE_SOURCE_DIR}/deps/MaaFramework/bin
    onnxruntime_maa.dll
    opencv_world4_maa.dll
    fastdeploy_ppocr_maa.dll
    MaaAdbControlUnit.dll
)

add_custom_target(predeploy
COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --config $<CONFIG> --target install
)

add_custom_target(deploy
COMMAND
    windeployqt ${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:whmx-assistant>
    --verbose 0
    --no-quick-import
    --no-translations
    --no-libraries
DEPENDS predeploy
)
