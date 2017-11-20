calamares_add_plugin( pacstrap-base
  TYPE job
  EXPORT_MACRO PLUGINDLLEXPORT_PRO
  SOURCES
    pacstrap-base.cpp
  LINK_PRIVATE_LIBRARIES
    calamares
  SHARED_LIB
)
