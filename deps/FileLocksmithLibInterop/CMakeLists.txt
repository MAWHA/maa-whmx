add_library(FileLocksmithLibInterop)

get_filename_component(SOURCE_DIR src REALPATH)
nice_target_sources(FileLocksmithLibInterop ${SOURCE_DIR}
PRIVATE
    FileLocksmith.cpp
    FileLocksmith.h
    FileLocksmithLibInterop.cpp
    NtdllBase.cpp
    NtdllBase.h
    NtdllExtensions.cpp
    NtdllExtensions.h
    pch.cpp
    pch.h
)

target_include_directories(FileLocksmithLibInterop
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_library(FileLocksmith::LibInterop ALIAS FileLocksmithLibInterop)
