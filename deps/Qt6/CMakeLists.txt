set(CMAKE_AUTOUIC ON PARENT_SCOPE)
set(CMAKE_AUTOMOC ON PARENT_SCOPE)
set(CMAKE_AUTORCC ON PARENT_SCOPE)

find_package(QT REQUIRED NAMES Qt6)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED
COMPONENTS
    Core
    Gui
    Widgets
    Network
    Svg
)

add_library(deps_qt6 INTERFACE)

target_link_libraries(deps_qt6
INTERFACE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Svg
)

add_library(desktop-app::Qt6 ALIAS deps_qt6)
