# $Id$

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES 
  VHECRTask.h
  TBBTrigger.h
  TBBReadCmd.h
  VHECRsettings.h
  DESTINATION include/${PACKAGE_NAME})
