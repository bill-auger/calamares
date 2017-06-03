calamares_add_plugin( desktop
  TYPE job
  EXPORT_MACRO PLUGINDLLEXPORT_PRO
  SOURCES
    desktop.cpp
  LINK_PRIVATE_LIBRARIES
    calamares
  SHARED_LIB
)
