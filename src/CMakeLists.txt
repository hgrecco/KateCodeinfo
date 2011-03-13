########### independent build #########
# This section can be deleted if 
# the plugin is compiled within kate
cmake_minimum_required(VERSION 2.8)

project(kate-codeinfo)
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES})


########### next target ###############

set(katecodeinfoplugin_PART_SRCS
    katecodeinfo.cpp
    codeinfoparser.cpp
    btfileindexer.cpp
    codeinfodatabase.cpp
)
set(katecodeinfoplugin_PART_UI
    codeinfowidget.ui
    codeinfoconfigwidget.ui
)
kde4_add_ui_files(katecodeinfoplugin_PART_SRCS ${katecodeinfoplugin_PART_UI} )
kde4_add_plugin(katecodeinfoplugin ${katecodeinfoplugin_PART_SRCS})
target_link_libraries(katecodeinfoplugin ${KDE4_KFILE_LIBS} kateinterfaces ${KDE4_KTEXTEDITOR_LIBS})

########### install files ###############

install( TARGETS katecodeinfoplugin DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES katecodeinfoplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
