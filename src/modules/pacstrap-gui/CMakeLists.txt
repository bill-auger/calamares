calamares_add_plugin( pacstrap-gui
  TYPE job
  EXPORT_MACRO PLUGINDLLEXPORT_PRO
  SOURCES
    pacstrap-gui.cpp
  LINK_PRIVATE_LIBRARIES
    calamares
  SHARED_LIB
)
