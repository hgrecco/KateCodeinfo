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

set(katebacktracebrowserplugin_PART_SRCS
    katebacktracebrowser.cpp
    btparser.cpp
    btfileindexer.cpp
    btdatabase.cpp
)
set(katebacktracebrowserplugin_PART_UI
    btbrowserwidget.ui
    btconfigwidget.ui
)
kde4_add_ui_files(katebacktracebrowserplugin_PART_SRCS ${katebacktracebrowserplugin_PART_UI} )
kde4_add_plugin(katebacktracebrowserplugin ${katebacktracebrowserplugin_PART_SRCS})
target_link_libraries(katebacktracebrowserplugin ${KDE4_KFILE_LIBS} kateinterfaces ${KDE4_KTEXTEDITOR_LIBS})

########### install files ###############

install( TARGETS katebacktracebrowserplugin DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES katebacktracebrowserplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
