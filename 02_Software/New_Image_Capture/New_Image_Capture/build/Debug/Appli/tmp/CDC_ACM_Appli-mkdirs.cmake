# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Dev/New_Image_Capture/New_Image_Capture/Appli")
  file(MAKE_DIRECTORY "C:/Dev/New_Image_Capture/New_Image_Capture/Appli")
endif()
file(MAKE_DIRECTORY
  "C:/Dev/New_Image_Capture/New_Image_Capture/Appli/build"
  "C:/Dev/New_Image_Capture/New_Image_Capture/build/Debug/Appli"
  "C:/Dev/New_Image_Capture/New_Image_Capture/build/Debug/Appli/tmp"
  "C:/Dev/New_Image_Capture/New_Image_Capture/build/Debug/Appli/src/CDC_ACM_Appli-stamp"
  "C:/Dev/New_Image_Capture/New_Image_Capture/build/Debug/Appli/src"
  "C:/Dev/New_Image_Capture/New_Image_Capture/build/Debug/Appli/src/CDC_ACM_Appli-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Dev/New_Image_Capture/New_Image_Capture/build/Debug/Appli/src/CDC_ACM_Appli-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Dev/New_Image_Capture/New_Image_Capture/build/Debug/Appli/src/CDC_ACM_Appli-stamp${cfgdir}") # cfgdir has leading slash
endif()
