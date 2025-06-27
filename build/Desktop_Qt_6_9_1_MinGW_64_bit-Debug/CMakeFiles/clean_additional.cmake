# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\Clone_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Clone_autogen.dir\\ParseCache.txt"
  "Clone_autogen"
  )
endif()
