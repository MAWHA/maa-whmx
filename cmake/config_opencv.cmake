# config MaaFramework
set(CV_SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/OpenCV)

find_package(OpenCV GLOBAL QUIET
HINTS ${CV_SOURCE_DIR}/x64/vc16/lib
)

if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found, install it to ${CV_SOURCE_DIR} first.")
endif()

add_library(OpenCV INTERFACE IMPORTED)

target_link_directories(OpenCV
INTERFACE
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(OpenCV
INTERFACE
    ${OpenCV_LIBS}
)

add_library(OpenCV::OpenCV ALIAS OpenCV)
