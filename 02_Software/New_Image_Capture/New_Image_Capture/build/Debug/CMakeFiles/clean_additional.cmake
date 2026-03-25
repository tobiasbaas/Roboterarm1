# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "C:\\Dev\\New_Image_Capture\\New_Image_Capture\\Appli\\build"
  "C:\\Dev\\New_Image_Capture\\New_Image_Capture\\FSBL\\build"
  )
endif()
