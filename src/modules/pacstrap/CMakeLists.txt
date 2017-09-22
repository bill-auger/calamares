calamares_add_plugin( pacstrap
  TYPE job
  EXPORT_MACRO PLUGINDLLEXPORT_PRO
  SOURCES
    pacstrap.cpp
  LINK_PRIVATE_LIBRARIES
    calamares
  SHARED_LIB
)

install(
  PROGRAMS
      ${CMAKE_SOURCE_DIR}/src/modules/pacstrap/pacstrap-calamares
  DESTINATION
      ${CMAKE_INSTALL_BINDIR}/
)

install(
  DIRECTORY
      ${CMAKE_SOURCE_DIR}/data/skel/
  DESTINATION
      ${CMAKE_INSTALL_DATADIR}/calamares/skel/
)
