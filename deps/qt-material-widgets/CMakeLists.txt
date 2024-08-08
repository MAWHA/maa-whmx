add_library(deps_qt-material-widgets OBJECT)

get_filename_component(SOURCE_DIR src REALPATH)
nice_target_sources(deps_qt-material-widgets ${SOURCE_DIR}
PRIVATE
    qtmaterialscrollbar.cpp
    qtmaterialscrollbar.h
    qtmaterialscrollbar_internal.cpp
    qtmaterialscrollbar_internal.h
    qtmaterialscrollbar_p.h
    qtmaterialtextfield.cpp
    qtmaterialtextfield.h
    qtmaterialtextfield_internal.cpp
    qtmaterialtextfield_internal.h
    qtmaterialtextfield_p.h
    qtmaterialdialog.cpp
    qtmaterialdialog.h
    qtmaterialdialog_internal.cpp
    qtmaterialdialog_internal.h
    qtmaterialdialog_p.h
    qtmaterialradiobutton.cpp
    qtmaterialradiobutton.h
    qtmaterialradiobutton_p.h
    qtmaterialtoggle.cpp
    qtmaterialtoggle.h
    qtmaterialtoggle_internal.cpp
    qtmaterialtoggle_internal.h
    qtmaterialtoggle_p.h
    lib/qtmaterialoverlaywidget.cpp
    lib/qtmaterialoverlaywidget.h
    lib/qtmaterialripple.cpp
    lib/qtmaterialripple.h
    lib/qtmaterialrippleoverlay.cpp
    lib/qtmaterialrippleoverlay.h
    lib/qtmaterialstatetransition.cpp
    lib/qtmaterialstatetransition.h
    lib/qtmaterialstatetransitionevent.h
    lib/qtmaterialstyle.cpp
    lib/qtmaterialstyle.h
    lib/qtmaterialstyle_p.h
    lib/qtmaterialtheme.cpp
    lib/qtmaterialtheme.h
    lib/qtmaterialtheme_p.h
    lib/qtmaterialcheckable.cpp
    lib/qtmaterialcheckable.h
    lib/qtmaterialcheckable_internal.cpp
    lib/qtmaterialcheckable_internal.h
    lib/qtmaterialcheckable_p.h
)

target_include_directories(deps_qt-material-widgets
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(deps_qt-material-widgets
PUBLIC
    desktop-app::Qt6
)

add_library(desktop-app::qt-material-widgets ALIAS deps_qt-material-widgets)
