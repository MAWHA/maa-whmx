add_executable(whmx-assistant WIN32 MACOSX_BUNDLE)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp.in
	${CMAKE_CURRENT_BINARY_DIR}/Version.cpp
)
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/src/HomepageUrl.cpp.in
	${CMAKE_CURRENT_BINARY_DIR}/HomepageUrl.cpp
)
nice_target_sources(whmx-assistant ${CMAKE_CURRENT_BINARY_DIR}
PRIVATE
    Version.cpp
    HomepageUrl.cpp
)

get_filename_component(SOURCE_DIR src REALPATH)
nice_target_sources(whmx-assistant ${SOURCE_DIR}
PRIVATE
    Main.cpp
    Consts.h
    DeviceHelper.cpp
    DeviceHelper.h
    Decode.cpp
    Decode.h
    ReferenceDataSet.cpp
    ReferenceDataSet.h
    Algorithm.cpp
    Algorithm.h
    Logger.cpp
    Logger.h
    MacroHelper.h
    Platform.cpp
    Platform.h
    Task/TaskGraph.cpp
    Task/TaskGraph.h
    Task/MajorTask.cpp
    Task/MajorTask.h
    Task/Config.cpp
    Task/Config.h
    Task/TaskParam.cpp
    Task/TaskParam.h
    Task/PropGetter.h
    Task/Router.cpp
    Task/Router.h
    UI/Helper.cpp
    UI/Helper.h
    UI/ClickableLabel.cpp
    UI/ClickableLabel.h
    UI/FlatButton.cpp
    UI/FlatButton.h
    UI/Scrollbar.cpp
    UI/Scrollbar.h
    UI/ElidedLabel.cpp
    UI/ElidedLabel.h
    UI/IconButton.cpp
    UI/IconButton.h
    UI/LogPanel.cpp
    UI/LogPanel.h
    UI/CheckableItem.cpp
    UI/CheckableItem.h
    UI/TaskItem.cpp
    UI/TaskItem.h
    UI/QueuedTaskItem.cpp
    UI/QueuedTaskItem.h
    UI/Workbench.cpp
    UI/Workbench.h
    UI/DeviceConn.cpp
    UI/DeviceConn.h
    UI/Settings.cpp
    UI/Settings.h
    UI/TaskConfigPanel.cpp
    UI/TaskConfigPanel.h
    UI/Client.cpp
    UI/Client.h
    CustomAction/Research.cpp
    CustomAction/Research.h
    CustomAction/FourInRow.cpp
    CustomAction/FourInRow.h
    CustomAction/Combat.cpp
    CustomAction/Combat.h
    CustomRecognizer/Utils.cpp
    CustomRecognizer/Utils.h
    CustomRecognizer/Research.cpp
    CustomRecognizer/Research.h
)

get_filename_component(RESOURCE_DIR res REALPATH)
nice_target_sources(whmx-assistant ${RESOURCE_DIR}
PRIVATE
    res.qrc
    whmx-assistant.rc
)

target_link_libraries(whmx-assistant
PRIVATE
    MaaFramework::MaaPP
    OpenCV::OpenCV
    magic_enum::magic_enum
    desktop-app::Qt6
    desktop-app::qt-material-widgets
    FileLocksmith::LibInterop
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

set(OCR_MODEL_DIR assets/general/model/ocr)
set(DEFAULT_OCR_MODEL ppocr_v4/zh_cn)
nice_install_files(${CMAKE_INSTALL_PREFIX}/${OCR_MODEL_DIR}
RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${OCR_MODEL_DIR}/${DEFAULT_OCR_MODEL}
    det.onnx
    rec.onnx
    keys.txt
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
