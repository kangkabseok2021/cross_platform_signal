#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ballistic::ballistic_engine" for configuration "Release"
set_property(TARGET ballistic::ballistic_engine APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ballistic::ballistic_engine PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libballistic_engine.dylib"
  IMPORTED_SONAME_RELEASE "@rpath/libballistic_engine.dylib"
  )

list(APPEND _cmake_import_check_targets ballistic::ballistic_engine )
list(APPEND _cmake_import_check_files_for_ballistic::ballistic_engine "${_IMPORT_PREFIX}/lib/libballistic_engine.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
