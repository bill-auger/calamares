calamares_add_plugin( pacstrap
  TYPE job
  EXPORT_MACRO PLUGINDLLEXPORT_PRO
  SOURCES
    pacstrap.cpp
  LINK_PRIVATE_LIBRARIES
    calamares
  SHARED_LIB
)
