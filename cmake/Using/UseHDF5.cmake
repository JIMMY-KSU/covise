MACRO(USE_HDF5)
  FIND_PACKAGE(HDF5 COMPONENTS C CXX HL CXX_HL static)
  IF ((NOT HDF5_FOUND) AND (${ARGC} LESS 1))
    USING_MESSAGE("Skipping because of missing HDF5")
    RETURN()
  ENDIF((NOT HDF5_FOUND) AND (${ARGC} LESS 1))
  IF(NOT HDF5_USED AND HDF5_FOUND)
    SET(HDF5_USED TRUE)
    INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIR})
    SET(EXTRA_LIBS ${EXTRA_LIBS} ${HDF5_LIBRARIES})
  ENDIF()
ENDMACRO(USE_HDF5)

