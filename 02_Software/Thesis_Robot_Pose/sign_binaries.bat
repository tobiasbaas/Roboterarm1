@echo off
REM ============================================================================
REM  STM32N6 FSBL and APPLI Signing + Flashing Script
REM  For STM32N6570-DK Discovery Board with J-Link or ST-Link
REM ============================================================================
REM
REM  This script:
REM  1. Signs the FSBL and APPLI binaries for flash boot mode
REM  2. Flashes both to external flash using J-Link Commander or ST Link CLI
REM
REM  Uses -nk (no keys) for development/unsigned mode.
REM  
REM  Memory Layout:
REM  - FSBL  @ 0x70000000 (External Flash) -> Loads to 0x34180000 (RAM)
REM  - APPLI @ 0x70100000 (External Flash) -> Loads to 0x34000000 (RAM)
REM
REM ============================================================================

setlocal enabledelayedexpansion

REM === Configuration ===
set CUBE_PROGRAMMER_PATH=C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin
set JLINK_PATH=C:\Program Files\SEGGER\JLink_V910
set EXT_LOADER=MX66UW1G45G_STM32N6570-DK.stldr
set EXT_LOADER_PATH=%CUBE_PROGRAMMER_PATH%\ExternalLoader\%EXT_LOADER%
set OBJCOPY=arm-none-eabi-objcopy

REM === Working directories ===
REM Detect VS Code (CMake) or CubeIDE build structure
if exist "FSBL\build" (
    echo Detected CMake ExternalProject build structure.
    set FSBL_DIR=FSBL\build
    set APPLI_DIR=Appli\build
) else if exist "build\FSBL" (
    echo Detected VS Code / CMake build structure.
    set FSBL_DIR=build\FSBL
    set APPLI_DIR=build\Appli
) else if exist "build\Debug" (
    echo Detected VS Code / CMake Debug build structure.
    set FSBL_DIR=build\Debug
    set APPLI_DIR=build\Debug
) else (
    echo Detected STM32CubeIDE build structure.
    set FSBL_DIR=STM32CubeIDE\FSBL\Debug
    set APPLI_DIR=STM32CubeIDE\Appli\Debug
)

REM === Binary names ===
set FSBL_BIN=CDC_ACM_FSBL.bin
set FSBL_SIGNED=CDC_ACM_FSBL-trusted.bin
set APPLI_ELF=CDC_ACM_Appli.elf
set APPLI_BIN=CDC_ACM_Appli.bin
set APPLI_CORE_BIN=CDC_ACM_Appli-core.bin
set APPLI_ROM2_BIN=CDC_ACM_Appli-rom2.bin
set APPLI_SIGNED=CDC_ACM_Appli-trusted.bin

REM === Flash addresses ===
set FSBL_FLASH_ADDR=0x70000000
set APPLI_FLASH_ADDR=0x70100000
set APPLI_ROM2_FLASH_ADDR=0x70200400

REM === Programmer detection ===
set PROGRAMMER=NONE

set STLINK_FOUND=0
set JLINK_FOUND=0

REM Check for ST-Link
"%CUBE_PROGRAMMER_PATH%\STM32_Programmer_CLI.exe" -l port=SWD > stlink_probe.txt 2>&1
findstr /I "ST-LINK STLINK STM32N6570" stlink_probe.txt >nul 2>&1
if %ERRORLEVEL% EQU 0 set STLINK_FOUND=1

if !STLINK_FOUND! EQU 1 (
    set PROGRAMMER=STLINK
    echo Detected ST-Link.
) else (
    echo ST-Link not detected. Probe output:
    type stlink_probe.txt
    "%JLINK_PATH%\JLink.exe" -ListEmu > jlink_probe.txt 2>&1
    findstr /I "J-Link" jlink_probe.txt >nul 2>&1
    if %ERRORLEVEL% EQU 0 set JLINK_FOUND=1
    if !JLINK_FOUND! EQU 1 (
        set PROGRAMMER=JLINK
        echo Detected J-Link.
    ) else (
        echo ERROR: No ST-Link or J-Link detected!
        goto :error
    )
)

echo.
echo ============================================================================
echo  STM32N6 Signing + Flashing Script (J-Link)
echo ============================================================================
echo.

REM Kill any stalled instances of the signing tool to release file locks
taskkill /F /IM STM32_SigningTool_CLI.exe >nul 2>&1
taskkill /F /IM STM32_Programmer_CLI.exe >nul 2>&1
taskkill /F /IM JLink.exe >nul 2>&1

REM Check if STM32_SigningTool exists
if not exist "%CUBE_PROGRAMMER_PATH%\STM32_SigningTool_CLI.exe" (
    echo ERROR: STM32_SigningTool_CLI.exe not found at:
    echo        %CUBE_PROGRAMMER_PATH%
    echo.
    echo Please update CUBE_PROGRAMMER_PATH in this script.
    goto :error
)

REM Check external loader
if not exist "%EXT_LOADER_PATH%" (
    echo ERROR: External loader not found at:
    echo        %EXT_LOADER_PATH%
    goto :error
)
echo External Loader: %EXT_LOADER_PATH%
echo.

REM === Sign FSBL ===
echo === Signing FSBL ===
echo Directory: %FSBL_DIR%
echo Input:     %FSBL_BIN%
echo Output:    %FSBL_SIGNED%
echo Load Address: 0x34100000
echo.

if not exist "%FSBL_DIR%\%FSBL_BIN%" (
    echo ERROR: FSBL binary not found: %FSBL_DIR%\%FSBL_BIN%
    echo Please build the FSBL project first.
    goto :error
)

pushd "%FSBL_DIR%"
if exist "%FSBL_SIGNED%" del /F /Q "%FSBL_SIGNED%"
"%CUBE_PROGRAMMER_PATH%\STM32_SigningTool_CLI.exe" -bin %FSBL_BIN% -nk -of 0x80000000 -t fsbl -o %FSBL_SIGNED% -align -hv 2.3 -dump %FSBL_SIGNED%
set FSBL_RESULT=%ERRORLEVEL%
popd

if %FSBL_RESULT% neq 0 (
    echo ERROR: FSBL signing failed!
    goto :error
)

echo.
echo === FSBL signed successfully! ===
echo.

REM === Sign APPLI ===
echo === Signing APPLI ===
echo Directory: %APPLI_DIR%
echo Input:     %APPLI_CORE_BIN%
echo Output:    %APPLI_SIGNED%
echo Load Address: 0x34000400
echo.

if not exist "%APPLI_DIR%\%APPLI_ELF%" (
    if not exist "%APPLI_DIR%\%APPLI_BIN%" (
        echo WARNING: APPLI ELF/BIN not found: %APPLI_DIR%\%APPLI_ELF%
        echo Skipping APPLI signing.
        goto :fsbl_only
    )
)

pushd "%APPLI_DIR%"
if exist "%APPLI_CORE_BIN%" del /F /Q "%APPLI_CORE_BIN%"
if exist "%APPLI_ROM2_BIN%" del /F /Q "%APPLI_ROM2_BIN%"
if exist "%APPLI_SIGNED%" del /F /Q "%APPLI_SIGNED%"

if exist "%APPLI_ELF%" (
    echo Generating compact APPLI binaries from ELF ^(core + ROM2^)...
    %OBJCOPY% -O binary --remove-section=.rodata --remove-section=.rodata.* %APPLI_ELF% %APPLI_CORE_BIN%
    if %ERRORLEVEL% neq 0 (
        echo ERROR: Failed to generate APPLI core binary from ELF.
        popd
        goto :error
    )

    %OBJCOPY% -O binary --only-section=.rodata --only-section=.rodata.* %APPLI_ELF% %APPLI_ROM2_BIN%
    if %ERRORLEVEL% neq 0 (
        echo ERROR: Failed to generate APPLI ROM2 binary from ELF.
        popd
        goto :error
    )
) else (
    echo WARNING: APPLI ELF missing, falling back to %APPLI_BIN% for signing.
    copy /Y %APPLI_BIN% %APPLI_CORE_BIN% >nul
    if %ERRORLEVEL% neq 0 (
        echo ERROR: Could not prepare APPLI core binary.
        popd
        goto :error
    )
)

"%CUBE_PROGRAMMER_PATH%\STM32_SigningTool_CLI.exe" -bin %APPLI_CORE_BIN% -nk -of 0x80000000 -t fsbl -o %APPLI_SIGNED% -align -hv 2.3 -dump %APPLI_SIGNED%
set APPLI_RESULT=%ERRORLEVEL%
popd

if %APPLI_RESULT% neq 0 (
    echo ERROR: APPLI signing failed!
    goto :error
)

echo.
echo === APPLI signed successfully! ===
echo.

:fsbl_only
echo ============================================================================
echo  SIGNING COMPLETE
echo ============================================================================
echo.
echo Signed binaries ready for flashing:
echo   FSBL:  %FSBL_DIR%\%FSBL_SIGNED%
echo          Flash to: %FSBL_FLASH_ADDR%
echo.
echo   APPLI: %APPLI_DIR%\%APPLI_SIGNED%
echo          Flash to: %APPLI_FLASH_ADDR%
echo.
if exist "%APPLI_DIR%\%APPLI_ROM2_BIN%" (
    for %%I in ("%APPLI_DIR%\%APPLI_ROM2_BIN%") do set APPLI_ROM2_SIZE=%%~zI
    if NOT "!APPLI_ROM2_SIZE!"=="0" (
        echo   APPLI ROM2 data: %APPLI_DIR%\%APPLI_ROM2_BIN%
        echo                   Flash to: %APPLI_ROM2_FLASH_ADDR%
    )
)
echo.
echo ============================================================================
echo  FLASHING TO BOARD (Auto: ST-Link/J-Link)
echo ============================================================================
echo.

if "%PROGRAMMER%"=="STLINK" (
    echo Using STM32_Programmer_CLI (ST-Link)

    set FLASH_CMD="%CUBE_PROGRAMMER_PATH%\STM32_Programmer_CLI.exe" -c port=SWD mode=UR reset=HWrst freq=4000 -el "%EXT_LOADER_PATH%" -d "%FSBL_DIR%\%FSBL_SIGNED%" %FSBL_FLASH_ADDR% -d "%APPLI_DIR%\%APPLI_SIGNED%" %APPLI_FLASH_ADDR%
    if exist "%APPLI_DIR%\%APPLI_ROM2_BIN%" (
        for %%I in ("%APPLI_DIR%\%APPLI_ROM2_BIN%") do set APPLI_ROM2_SIZE=%%~zI
        if NOT "!APPLI_ROM2_SIZE!"=="0" (
            set FLASH_CMD=!FLASH_CMD! -d "%APPLI_DIR%\%APPLI_ROM2_BIN%" %APPLI_ROM2_FLASH_ADDR%
        )
    )
    call !FLASH_CMD!
    if %ERRORLEVEL% neq 0 (
        echo ERROR: Flashing with ST-Link failed!
        goto :error
    )

    echo.
    echo Programming finished successfully on external flash.
    echo Checking optional MCU run/reset state...
    "%CUBE_PROGRAMMER_PATH%\STM32_Programmer_CLI.exe" -c port=SWD mode=UR reset=HWrst freq=4000 -rst >nul 2>&1
    if %ERRORLEVEL% neq 0 (
        echo WARNING: Programming was successful, but automatic run/reset failed.
        echo          Press RESET on the board to start the new image.
    ) else (
        echo MCU reset successful.
    )

    goto :done
)


if "%PROGRAMMER%"=="JLINK" (
    echo Using J-Link Commander (J-Link)
    "%JLINK_PATH%\JLink.exe" -Device STM32N657X0 -If SWD -Speed 4000 -CommanderScript flash.jlink
    if %ERRORLEVEL% neq 0 (
        echo ERROR: Flashing with J-Link failed!
        goto :error
    )
    goto :done
)

echo ERROR: No valid programmer detected. Aborting.
goto :error

:done
echo.
echo Flashing complete.
echo Note: If the MCU does not start automatically, press RESET on the board.
exit /b 0

:error
echo.
echo Script failed!
exit /b 1
