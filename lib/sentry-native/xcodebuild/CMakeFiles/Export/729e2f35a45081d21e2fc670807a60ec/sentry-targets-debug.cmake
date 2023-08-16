#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "sentry::sentry" for configuration "Debug"
set_property(TARGET sentry::sentry APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(sentry::sentry PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libsentry.dylib"
  IMPORTED_SONAME_DEBUG "@rpath/libsentry.dylib"
  )

list(APPEND _cmake_import_check_targets sentry::sentry )
list(APPEND _cmake_import_check_files_for_sentry::sentry "${_IMPORT_PREFIX}/lib/libsentry.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
