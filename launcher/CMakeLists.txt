add_executable(launcher WIN32 MACOSX_BUNDLE)

get_filename_component(SOURCE_DIR src REALPATH)
nice_target_sources(launcher ${SOURCE_DIR}
PRIVATE
    Main.cpp
    Platform.cpp
    Platform.h
)

get_filename_component(RESOURCE_DIR res REALPATH)
nice_target_sources(launcher ${RESOURCE_DIR}
PRIVATE
    launcher.rc
)

target_link_libraries(launcher
PRIVATE
    desktop-app::Qt6Core
    FileLocksmith::LibInterop
)
