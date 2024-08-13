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

# Output directory
set_target_properties(launcher PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
if(${CMAKE_GENERATOR} MATCHES "Visual Studio*")
    set_target_properties(launcher PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}")
    set_target_properties(launcher PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}")
endif()
