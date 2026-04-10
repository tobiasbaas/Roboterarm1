@echo off
REM ============================================================================
REM  STM32N6 FSBL and APPLI Signing + Flashing Script
REM  For STM32N6570-DK Discovery Board with J-Link or ST-Link
REM ============================================================================

setlocal enabledelayedexpansion

REM === Single-run mode (no parameters) ===
echo ============================================================================
echo  STM32N6 EIN-KLICK-FLOW: BUILD + SIGN + ELF + FLASH
echo  (Keine Parameter erforderlich)
echo ============================================================================
echo.

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

REM === Project build (always) ===
if exist "CMakePresets.json" (
    where cmake >nul 2>&1
    if %ERRORLEVEL% neq 0 (
        echo ERROR: cmake not found in PATH.
        echo Please install CMake and retry.
        goto :error
    )

    echo Running CMake configure preset: Debug
    cmake --preset Debug
    if %ERRORLEVEL% neq 0 (
        echo ERROR: CMake configure failed for preset Debug.
        goto :error
    )

    echo Running CMake build preset: Debug
    cmake --build --preset Debug --parallel
    if %ERRORLEVEL% neq 0 (
        echo ERROR: CMake build failed for preset Debug.
        goto :error
    )
    echo Build completed.
    echo.
) else (
    echo WARNING: CMakePresets.json not found. Skipping auto build.
    echo.
)

REM === Binary names ===
set FSBL_ELF=CDC_ACM_FSBL.elf
set FSBL_BIN=CDC_ACM_FSBL.bin
set FSBL_SIGNED=CDC_ACM_FSBL-trusted.bin
set APPLI_ELF=CDC_ACM_Appli.elf
set APPLI_BIN=CDC_ACM_Appli.bin
set APPLI_CORE_BIN=CDC_ACM_Appli-core.bin
set APPLI_ROM2_BIN=CDC_ACM_Appli-rom2.bin
set APPLI_SIGNED=CDC_ACM_Appli-trusted.bin
set FSBL_SIGNED_ELF=FSBL-trusted.elf
set APPLI_SIGNED_ELF=Appli-trusted.elf
set APPLI_ROM2_ELF=Appli-rom2.elf

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

where %OBJCOPY% >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: %OBJCOPY% not found in PATH.
    echo Please ensure STM32CubeCLT or Arm GNU toolchain is installed and in PATH.
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

if not exist "%FSBL_DIR%\%FSBL_ELF%" (
    if not exist "%FSBL_DIR%\%FSBL_BIN%" (
        echo ERROR: FSBL ELF and BIN not found in %FSBL_DIR%
        echo Please build the FSBL project first.
        goto :error
    )
)

pushd "%FSBL_DIR%"
if exist "%FSBL_ELF%" (
    if exist "%FSBL_BIN%" del /F /Q "%FSBL_BIN%"
    echo Generating FSBL full binary from ELF...
    %OBJCOPY% -O binary %FSBL_ELF% %FSBL_BIN%
    if %ERRORLEVEL% neq 0 (
        echo ERROR: Failed to generate FSBL binary from ELF.
        popd
        goto :error
    )
    echo FSBL BIN generated: %FSBL_BIN%
) else (
    echo WARNING: FSBL ELF not found, using existing BIN: %FSBL_BIN%
)

if exist "%FSBL_SIGNED%" del /F /Q "%FSBL_SIGNED%"
"%CUBE_PROGRAMMER_PATH%\STM32_SigningTool_CLI.exe" -bin %FSBL_BIN% -nk -of 0x80000000 -t fsbl -o %FSBL_SIGNED% -align -hv 2.3 -dump %FSBL_SIGNED%
set FSBL_RESULT=%ERRORLEVEL%
popd

if %FSBL_RESULT% neq 0 (
    echo ERROR: FSBL signing failed!
    goto :error
)

pushd "%FSBL_DIR%"
if exist "%FSBL_SIGNED_ELF%" del /F /Q "%FSBL_SIGNED_ELF%"
echo Converting signed FSBL binary to ELF for debugger imageFileName...
%OBJCOPY% -I binary -O elf32-littlearm -B arm --change-addresses=%FSBL_FLASH_ADDR% %FSBL_SIGNED% %FSBL_SIGNED_ELF%
set FSBL_ELF_RESULT=%ERRORLEVEL%
popd

if %FSBL_ELF_RESULT% neq 0 (
    echo ERROR: Failed to convert signed FSBL binary to ELF.
    goto :error
)

echo.
echo === FSBL signed successfully! ===
echo.

REM === Sign APPLI ===
echo === Signing APPLI ===
echo Directory: %APPLI_DIR%
echo Input:     %APPLI_CORE_BIN%
echo ROM2:      %APPLI_ROM2_BIN%
echo Output:    %APPLI_SIGNED%
echo Load Address: 0x34000400
echo.

if not exist "%APPLI_DIR%\%APPLI_ELF%" (
    echo WARNING: APPLI ELF not found: %APPLI_DIR%\%APPLI_ELF%
    echo Skipping APPLI signing.
    goto :fsbl_only
)

pushd "%APPLI_DIR%"
if exist "%APPLI_BIN%" del /F /Q "%APPLI_BIN%"
if exist "%APPLI_CORE_BIN%" del /F /Q "%APPLI_CORE_BIN%"
if exist "%APPLI_ROM2_BIN%" del /F /Q "%APPLI_ROM2_BIN%"
if exist "%APPLI_SIGNED%" del /F /Q "%APPLI_SIGNED%"

echo Generating APPLI full binary from ELF...
%OBJCOPY% -O binary %APPLI_ELF% %APPLI_BIN%
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to generate APPLI full binary from ELF.
    popd
    goto :error
)

echo Generating APPLI core binary from ELF (without .rodata in ROM2)...
%OBJCOPY% -O binary -R .rodata -R .rodata.* %APPLI_ELF% %APPLI_CORE_BIN%
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to generate APPLI core binary from ELF.
    popd
    goto :error
)

echo Generating APPLI ROM2 binary from ELF (.rodata only)...
%OBJCOPY% -O binary -j .rodata -j .rodata.* %APPLI_ELF% %APPLI_ROM2_BIN%
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to generate APPLI ROM2 binary from ELF.
    popd
    goto :error
)

"%CUBE_PROGRAMMER_PATH%\STM32_SigningTool_CLI.exe" -bin %APPLI_CORE_BIN% -nk -of 0x80000000 -t fsbl -o %APPLI_SIGNED% -align -hv 2.3 -dump %APPLI_SIGNED%
set APPLI_RESULT=%ERRORLEVEL%
popd

if %APPLI_RESULT% neq 0 (
    echo ERROR: APPLI signing failed!
    goto :error
)

pushd "%APPLI_DIR%"
if exist "%APPLI_SIGNED_ELF%" del /F /Q "%APPLI_SIGNED_ELF%"
echo Converting signed APPLI binary to ELF for debugger imageFileName...
%OBJCOPY% -I binary -O elf32-littlearm -B arm --change-addresses=%APPLI_FLASH_ADDR% %APPLI_SIGNED% %APPLI_SIGNED_ELF%
set APPLI_ELF_RESULT=%ERRORLEVEL%

if exist "%APPLI_ROM2_BIN%" (
    for %%I in ("%APPLI_ROM2_BIN%") do set ROM2_ELF_SIZE=%%~zI
    if not "!ROM2_ELF_SIZE!"=="0" (
        if exist "%APPLI_ROM2_ELF%" del /F /Q "%APPLI_ROM2_ELF%"
        echo Converting APPLI ROM2 binary to ELF...
        %OBJCOPY% -I binary -O elf32-littlearm -B arm --change-addresses=%APPLI_ROM2_FLASH_ADDR% %APPLI_ROM2_BIN% %APPLI_ROM2_ELF%
        set APPLI_ROM2_ELF_RESULT=!ERRORLEVEL!
    ) else (
        set APPLI_ROM2_ELF_RESULT=0
    )
) else (
    set APPLI_ROM2_ELF_RESULT=0
)
popd

if %APPLI_ELF_RESULT% neq 0 (
    echo ERROR: Failed to convert signed APPLI binary to ELF.
    goto :error
)

if not "%APPLI_ROM2_ELF_RESULT%"=="0" (
    echo ERROR: Failed to convert APPLI ROM2 binary to ELF.
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
echo   FSBL RAW BIN:  %FSBL_DIR%\%FSBL_BIN%
echo   APPLI RAW BIN: %APPLI_DIR%\%APPLI_BIN%
echo.
echo   FSBL:  %FSBL_DIR%\%FSBL_SIGNED%
echo          Flash to: %FSBL_FLASH_ADDR%
echo          ELF for launch.json imageFileName: %FSBL_DIR%\%FSBL_SIGNED_ELF%
echo.
echo   APPLI: %APPLI_DIR%\%APPLI_SIGNED%
echo          Flash to: %APPLI_FLASH_ADDR%
echo          ELF for launch.json imageFileName: %APPLI_DIR%\%APPLI_SIGNED_ELF%
echo.
echo   ROM2:  %APPLI_DIR%\%APPLI_ROM2_BIN%
echo          Flash to: %APPLI_ROM2_FLASH_ADDR%
echo          Optional ELF: %APPLI_DIR%\%APPLI_ROM2_ELF%
echo.
echo.
echo ============================================================================
echo  FLASHING TO BOARD (Auto: ST-Link/J-Link)
echo ============================================================================
echo.

if "%PROGRAMMER%"=="STLINK" (
    echo Using STM32_Programmer_CLI (ST-Link)

    set FLASH_CMD="%CUBE_PROGRAMMER_PATH%\STM32_Programmer_CLI.exe" -c port=SWD mode=UR reset=HWrst freq=4000 -el "%EXT_LOADER_PATH%" -d "%FSBL_DIR%\%FSBL_SIGNED%" %FSBL_FLASH_ADDR% -d "%APPLI_DIR%\%APPLI_SIGNED%" %APPLI_FLASH_ADDR%

    if exist "%APPLI_DIR%\%APPLI_ROM2_BIN%" (
        for %%I in ("%APPLI_DIR%\%APPLI_ROM2_BIN%") do set ROM2_SIZE=%%~zI
        if not "!ROM2_SIZE!"=="0" (
            set FLASH_CMD=!FLASH_CMD! -d "%APPLI_DIR%\%APPLI_ROM2_BIN%" %APPLI_ROM2_FLASH_ADDR%
        ) else (
            echo WARNING: ROM2 binary exists but is empty, skipping ROM2 flash.
        )
    ) else (
        echo WARNING: ROM2 binary not found, skipping ROM2 flash.
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