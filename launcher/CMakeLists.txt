add_executable(launcher)
set_target_properties(launcher PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
set_target_properties(launcher PROPERTIES MACOSX_BUNDLE $<CONFIG:Release>)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/src/Uuid.cpp.in
	${CMAKE_CURRENT_BINARY_DIR}/Uuid.cpp
)
nice_target_sources(launcher ${CMAKE_CURRENT_BINARY_DIR}
PRIVATE
    Uuid.cpp
)

get_filename_component(SOURCE_DIR src REALPATH)
nice_target_sources(launcher ${SOURCE_DIR}
PRIVATE
    Main.cpp
    VirtualDesktop.h
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
    Microsoft.GSL::GSL
    desktop-app::Qt6Core
    FileLocksmith::LibInterop
)

# Output directory
set_target_properties(launcher PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
if(${CMAKE_GENERATOR} MATCHES "Visual Studio*")
    set_target_properties(launcher PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}")
    set_target_properties(launcher PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}")
endif()
