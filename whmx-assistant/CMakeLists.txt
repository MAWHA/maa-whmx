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

nice_install_files(${CMAKE_INSTALL_PREFIX}
ABSOLUTE
    $<REMOVE_DUPLICATES:$<TARGET_RUNTIME_DLLS:whmx-assistant>>
RELATIVE ${CMAKE_SOURCE_DIR}/deps/MaaFramework/bin
    onnxruntime_maa.dll
    opencv_world4_maa.dll
    fastdeploy_ppocr_maa.dll
    MaaAdbControlUnit.dll
)
