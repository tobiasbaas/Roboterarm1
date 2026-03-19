# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "C:\\Dev\\Thesis_Robot_Pose\\Appli\\build"
  "C:\\Dev\\Thesis_Robot_Pose\\FSBL\\build"
  )
endif()
