find_package(OpenCV REQUIRED)

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
