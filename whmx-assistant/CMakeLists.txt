add_executable(whmx-assistant)
set_target_properties(whmx-assistant PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
set_target_properties(whmx-assistant PROPERTIES MACOSX_BUNDLE $<CONFIG:Release>)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/src/Version.cpp.in
	${CMAKE_CURRENT_BINARY_DIR}/Version.cpp
)
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/src/HomepageUrl.cpp.in
	${CMAKE_CURRENT_BINARY_DIR}/HomepageUrl.cpp
)
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/src/Uuid.cpp.in
	${CMAKE_CURRENT_BINARY_DIR}/Uuid.cpp
)
nice_target_sources(whmx-assistant ${CMAKE_CURRENT_BINARY_DIR}
PRIVATE
    Version.cpp
    HomepageUrl.cpp
    Uuid.cpp
)

get_filename_component(SOURCE_DIR src REALPATH)
nice_target_sources(whmx-assistant ${SOURCE_DIR}
PRIVATE
    Main.cpp
    Consts.h
    DeviceHelper.cpp
    DeviceHelper.h
    PropertyType.h
    Property.cpp
    Property.h
    PropertyContext.cpp
    PropertyContext.h
    TaskInterface.cpp
    TaskInterface.h
    Decode.cpp
    Decode.h
    ReferenceDataSet.cpp
    ReferenceDataSet.h
    Algorithm.cpp
    Algorithm.h
    Logger.cpp
    Logger.h
    MacroHelper.h
    Process.cpp
    Process.h
    Platform.h
    Platform/windows.cpp
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
    UI/NavBar.cpp
    UI/NavBar.h
    UI/NavItemDelegate.cpp
    UI/NavItemDelegate.h
    UI/NavModel.cpp
    UI/NavModel.h
    UI/NavNode.cpp
    UI/NavNode.h
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
    UI/Notification.cpp
    UI/Notification.h
    UI/Card.cpp
    UI/Card.h
    UI/PropertyHelper.h
    UI/TitleBar.cpp
    UI/TitleBar.h
    UI/NavWidget.cpp
    UI/NavWidget.h
    Experimental/ProjectDirs.cpp
    Experimental/ProjectDirs.h
    Experimental/Package.cpp
    Experimental/Package.h
    Experimental/ActuatorInstance.h
    Experimental/MessageProducer.h
    Experimental/UmaClient.cpp
    Experimental/UmaClient.h
    Experimental/UmaWorkbench.cpp
    Experimental/UmaWorkbench.h
    Experimental/StyledPanel.cpp
    Experimental/StyledPanel.h
    Experimental/OverlayDialog.cpp
    Experimental/OverlayDialog.h
    Experimental/TaskItem.cpp
    Experimental/TaskItem.h
    Experimental/TaskModel.cpp
    Experimental/TaskModel.h
    Experimental/TaskView.cpp
    Experimental/TaskView.h
    Experimental/TaskViewStyle.cpp
    Experimental/TaskViewStyle.h
    Experimental/TaskGraph.cpp
    Experimental/TaskGraph.h
    Experimental/TaskRouterSchema.cpp
    Experimental/TaskRouterSchema.h
    Experimental/TaskRouter.cpp
    Experimental/TaskRouter.h
    App.cpp
    App.h
    AppEvent.h
    Action/Research.cpp
    Action/Research.h
    Action/FourInRow.cpp
    Action/FourInRow.h
    Action/Combat.cpp
    Action/Combat.h
    Rec/Utils.cpp
    Rec/Utils.h
    Rec/Research.cpp
    Rec/Research.h
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
    ElaWidgetTools::ElaWidgetTools
    spdlog::spdlog
    Microsoft.GSL::GSL
    tl::expected
    QWindowKit::Widgets
    desktop-app::Qt6
    desktop-app::qt-material-widgets
)

# Output directory
set_target_properties(whmx-assistant PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
if(${CMAKE_GENERATOR} MATCHES "Visual Studio*")
    set_target_properties(whmx-assistant PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}")
    set_target_properties(whmx-assistant PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}")
endif()


# Copy Dlls
add_custom_command(TARGET whmx-assistant POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_RUNTIME_DLLS:whmx-assistant> $<TARGET_FILE_DIR:whmx-assistant>
COMMAND_EXPAND_LISTS
)

# Copy Maa
file_copy_target(copy_maa
GLOB "*"
SRC_DIR ${CMAKE_SOURCE_DIR}/deps/MaaFramework/share/MaaAgentBinary
DST_DIR ${OUTPUT_DIR}/agents
MSG_NAME "MAA files"
)
add_dependencies(copy_maa MaaFramework::MaaPP)

# Copy assets
file_copy_target(copy_assets
GLOB "*"
SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets
DST_DIR ${OUTPUT_DIR}/assets
MSG_NAME "assets files"
)

# Copy configs
file_copy_target(copy_configs
GLOB "*"
SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config
DST_DIR ${OUTPUT_DIR}/config
MSG_NAME "config files"
)

# Copy OCR models
set(OCR_MODEL_DIR assets/general/model/ocr)
set(DEFAULT_OCR_MODEL ppocr_v4/zh_cn)
set(OCR_MODEL_LIST
    "det.onnx"
    "rec.onnx"
    "keys.txt"
)
file_copy_target(copy_ocr_models
GLOB ${OCR_MODEL_LIST}
SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${OCR_MODEL_DIR}/${DEFAULT_OCR_MODEL}
DST_DIR ${OUTPUT_DIR}/${OCR_MODEL_DIR}
MSG_NAME "OCR model files"
)

# Copy MAA Dlls
set(MAA_DLL_LIST
    "onnxruntime_maa.dll"
    "opencv_world4_maa.dll"
    "fastdeploy_ppocr_maa.dll"
    "MaaAdbControlUnit.dll"
)
file_copy_target(copy_maa_dlls
GLOB ${MAA_DLL_LIST}
SRC_DIR ${CMAKE_SOURCE_DIR}/deps/MaaFramework/bin
DST_DIR ${OUTPUT_DIR}
MSG_NAME "MAA library files"
)
add_dependencies(copy_maa_dlls MaaFramework::MaaPP)

# Add dependencies
add_dependencies(whmx-assistant
    copy_maa
    copy_assets
    copy_configs
    copy_ocr_models
    copy_maa_dlls
)

# Qt deploy
find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt)
message(STATUS "Windeployqt executable path: ${WINDEPLOYQT_EXECUTABLE}")

get_filename_component(WINDEPLOYQT_DIR ${WINDEPLOYQT_EXECUTABLE} DIRECTORY)
set(WINDEPLOYQT_DEBUG_EXECUTABLE "${WINDEPLOYQT_DIR}/windeployqt.debug.bat")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(WINDEPLOYQT_CMD "${WINDEPLOYQT_DEBUG_EXECUTABLE}")
    set(WINDEPLOYQT_TYPE_ARGS "--debug")
    if(MSVC)
        set(WINDEPLOYQT_TYPE_ARGS ${WINDEPLOYQT_TYPE_ARGS} "--pdb")
    endif()
else()
    set(WINDEPLOYQT_CMD "${WINDEPLOYQT_EXECUTABLE}")
    set(WINDEPLOYQT_TYPE_ARGS "--release")
endif()

add_custom_command(TARGET whmx-assistant POST_BUILD
COMMAND
    "${WINDEPLOYQT_CMD}" "$<TARGET_FILE:whmx-assistant>"
    --verbose 0
    --no-quick-import
    --no-translations
    --no-libraries
    --force
    ${WINDEPLOYQT_TYPE_ARGS}
)

# Deep deploy dll deps
qm_import(Deploy)
qm_deploy_directory("$<TARGET_FILE_DIR:whmx-assistant>"
LIBRARY_DIR .
PLUGIN_DIR .
VERBOSE
)

# Install vcruntime
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${OUTPUT_DIR}")
include(InstallRequiredSystemLibraries)
