#----------------------------------------------------------------
# Generated CMake target import file for configuration "MinSizeRel".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "sentry_crashpad::crashpad_handler" for configuration "MinSizeRel"
set_property(TARGET sentry_crashpad::crashpad_handler APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(sentry_crashpad::crashpad_handler PROPERTIES
  IMPORTED_LOCATION_MINSIZEREL "${_IMPORT_PREFIX}/bin/crashpad_handler"
  )

list(APPEND _cmake_import_check_targets sentry_crashpad::crashpad_handler )
list(APPEND _cmake_import_check_files_for_sentry_crashpad::crashpad_handler "${_IMPORT_PREFIX}/bin/crashpad_handler" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)