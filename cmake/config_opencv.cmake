find_package(OpenCV REQUIRED)

add_library(OpenCV INTERFACE)

target_include_directories(OpenCV
INTERFACE
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(OpenCV
INTERFACE
    ${CMAKE_SOURCE_DIR}/deps/MaaFramework/lib/opencv_world4_maa.lib
)

add_library(OpenCV::OpenCV ALIAS OpenCV)
