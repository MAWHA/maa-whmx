# config MaaFramework
set(MAA_SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/MaaFramework)

find_package(MaaFramework REQUIRED GLOBAL
HINTS ${MAA_SOURCE_DIR}/share/cmake
)

# config meojson
set(MEOJSON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/meojson)

add_library(meojson INTERFACE)

target_include_directories(meojson
INTERFACE
    ${MEOJSON_SOURCE_DIR}/include
)

add_library(meojson::meojson ALIAS meojson)

# config MaaPP
set(MAAPP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/MaaPP)

add_library(MaaPP INTERFACE)

target_include_directories(MaaPP
INTERFACE
    ${MAAPP_SOURCE_DIR}/include
)

target_link_libraries(MaaPP
INTERFACE
    MaaFramework::MaaFramework
    MaaFramework::MaaToolkit
    meojson::meojson
)

add_library(MaaFramework::MaaPP ALIAS MaaPP)
