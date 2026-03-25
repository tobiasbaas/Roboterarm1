cmake_minimum_required(VERSION 3.22)
include(ExternalProject)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# In order to use CMake for cross-compiling
set(CMAKE_CXX_COMPILER_FORCED "true")

# Set the project name
set(CMAKE_PROJECT_NAME test_cdc_cmake)

# Core project settings
project(${CMAKE_PROJECT_NAME})

# Include external project file
include("mx-generated.cmake")

# Ensure OBJCOPY is defined (fallback if toolchain didn't set it)
if(NOT CMAKE_OBJCOPY)
    set(CMAKE_OBJCOPY "arm-none-eabi-objcopy")
endif()

# Generate .bin files for sub-projects (FSBL and Appli)
if(TARGET CDC_ACM_FSBL AND DEFINED ST_BOOT_FLASH_FSBL_PROJECT_BUILD_TARGET)
    get_filename_component(FSBL_EXEC_DIR ${ST_BOOT_FLASH_FSBL_PROJECT_BUILD_TARGET} DIRECTORY)
    get_filename_component(FSBL_EXEC_NAME ${ST_BOOT_FLASH_FSBL_PROJECT_BUILD_TARGET} NAME_WE)
    set(FSBL_BIN_PATH "${FSBL_EXEC_DIR}/${FSBL_EXEC_NAME}.bin")

    add_custom_command(TARGET CDC_ACM_FSBL POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary ${ST_BOOT_FLASH_FSBL_PROJECT_BUILD_TARGET} ${FSBL_BIN_PATH}
        COMMENT "Generating FSBL binary: ${FSBL_BIN_PATH}"
    )
endif()

if(TARGET CDC_ACM_Appli AND DEFINED ST_BOOT_FLASH_APPLI_PROJECT_BUILD_TARGET)
    get_filename_component(APPLI_EXEC_DIR ${ST_BOOT_FLASH_APPLI_PROJECT_BUILD_TARGET} DIRECTORY)
    get_filename_component(APPLI_EXEC_NAME ${ST_BOOT_FLASH_APPLI_PROJECT_BUILD_TARGET} NAME_WE)
    set(APPLI_BIN_PATH "${APPLI_EXEC_DIR}/${APPLI_EXEC_NAME}.bin")

    add_custom_command(TARGET CDC_ACM_Appli POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary ${ST_BOOT_FLASH_APPLI_PROJECT_BUILD_TARGET} ${APPLI_BIN_PATH}
        COMMENT "Generating Appli binary: ${APPLI_BIN_PATH}"
    )
endif()

# Add a custom target to sign and flash the binaries
add_custom_target(flash
    COMMAND cmd /c "${CMAKE_SOURCE_DIR}/sign_binaries.bat"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS CDC_ACM_FSBL
    COMMENT "Signing and Flashing binaries to STM32N6..."
    USES_TERMINAL
)
