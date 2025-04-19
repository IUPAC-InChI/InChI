rem djb-rwth: InChI CMake build tool for Windows OS

@echo off
cls

setlocal EnableExtensions DisableDelayedExpansion
for /F %%a in ('echo prompt $E ^| cmd') do (
	set "ESC=%%a"
)

setlocal EnableDelayedExpansion

echo !ESC![4;96mInChI CMake build tool!ESC![0m
echo.
echo Please select an option from the list:
echo.
echo !ESC![96m1!ESC![0m: Build CLI project (inchi-1)
echo !ESC![96m2!ESC![0m: Build LIBINCHI library (without binaries)
echo !ESC![96m3!ESC![0m: Build MAIN API project (LIBINCHI w/ corresponding binary INCHI_MAIN)
echo !ESC![96m4!ESC![0m: Build TEST_IXA project (for testing IXA API)
echo !ESC![96m5!ESC![0m: Build MOL2INCHI project (conversion of MOL/SDF files to InChI w/ multithreading support)
echo !ESC![96m6!ESC![0m: Build ALL projects (please refer to options 1-5 above)
echo.

:cmb_opt
echo | set /p="!ESC![96mPlease enter the option number [1-6]:!ESC![0m "
set /P B_OPT= %=%
if "%B_OPT%"=="1" goto cli
if "%B_OPT%"=="2" goto lib
if "%B_OPT%"=="3" goto main
if "%B_OPT%"=="4" goto ixa
if "%B_OPT%"=="5" goto mol2inchi
if "%B_OPT%"=="6" goto allinchi
goto cmb_opt

:cli
echo.
echo.
echo !ESC![96mCreating build directory!ESC![0m
echo.
mkdir ..\CMake_build
cd ..\CMake_build
echo.
echo !ESC![96mCreating build directory for InChI CLI!ESC![0m
if exist cli_build ( rmdir "cli_build" /S /Q )
mkdir cli_build
cd cli_build
echo.
echo !ESC![96mConfiguring InChI CLI!ESC![0m
echo.
cmake ../../INCHI-1-SRC/INCHI_EXE/inchi-1/src
echo.
echo !ESC![96mBuilding InChI CLI!ESC![0m
echo.
if exist *.sln ( cmake --build . --target ALL_BUILD --config Release ) else ( cmake --build . )
echo.
echo.
echo !ESC![92m========== BUILD SUCCEEDED ==========!ESC![0m
echo | set /p="!ESC![96minchi-1.exe!ESC![0m is located in directory "
if exist bin\Release\ ( echo !ESC![96mCMake_build/cli_build/bin/Release!ESC![0m ) else ( echo !ESC![96mCMake_build/cli_build/bin!ESC![0m )
echo.
if /i "%comspec% /c %~0 " equ "%cmdcmdline:"=%" pause
goto :eof

:lib
echo.
echo.
echo !ESC![96mCreating build directory!ESC![0m
echo.
mkdir ..\CMake_build
cd ..\CMake_build
echo.
echo !ESC![96mCreating build directory for LIBINCHI!ESC![0m
if exist libinchi_build ( rmdir "libinchi_build" /S /Q )
mkdir libinchi_build
cd libinchi_build
echo.
echo !ESC![96mConfiguring LIBINCHI!ESC![0m
echo.
cmake ../../INCHI-1-SRC/INCHI_API/libinchi/src
echo.
echo !ESC![96mBuilding LIBINCHI!ESC![0m
echo.
if exist *.sln ( cmake --build . --target ALL_BUILD --config Release ) else ( cmake --build . )
echo.
echo.
echo !ESC![92m========== BUILD SUCCEEDED ==========!ESC![0m
echo | set /p="!ESC![96mlibinchi.dll!ESC![0m is located in directory "
if exist bin\Release\ ( echo !ESC![96mCMake_build/libinchi_build/bin/Release!ESC![0m ) else ( echo !ESC![96mCMake_build/libinchi_build/bin!ESC![0m )
echo.
if /i "%comspec% /c %~0 " equ "%cmdcmdline:"=%" pause
goto :eof

:main
echo.
echo.
echo !ESC![96mCreating build directory!ESC![0m
echo.
mkdir ..\CMake_build
cd ..\CMake_build
echo.
echo !ESC![96mCreating build directory for INCHI_MAIN!ESC![0m
if exist inchi_main_build ( rmdir "inchi_main_build" /S /Q )
mkdir inchi_main_build
cd inchi_main_build
echo.
echo !ESC![96mConfiguring INCHI_MAIN!ESC![0m
echo.
cmake ../../INCHI-1-SRC/INCHI_API/demos/inchi_main/src
echo.
echo !ESC![96mBuilding INCHI_MAIN!ESC![0m
echo.
if exist *.sln ( cmake --build . --target ALL_BUILD --config Release ) else ( cmake --build . )
echo.
echo.
echo !ESC![92m========== BUILD SUCCEEDED ==========!ESC![0m
echo | set /p="!ESC![96minchi_main.exe & libinchi.dll!ESC![0m are located in directory "
if exist bin\Release\ ( echo !ESC![96mCMake_build/inchi_main_build/bin/Release!ESC![0m ) else ( echo !ESC![96mCMake_build/inchi_main_build/bin!ESC![0m )
echo.
if /i "%comspec% /c %~0 " equ "%cmdcmdline:"=%" pause
goto :eof

:ixa
echo.
echo.
echo !ESC![96mCreating build directory!ESC![0m
echo.
mkdir ..\CMake_build
cd ..\CMake_build
echo.
echo !ESC![96mCreating build directory for TEST_IXA!ESC![0m
if exist test_ixa_build ( rmdir "test_ixa_build" /S /Q )
mkdir test_ixa_build
cd test_ixa_build
echo.
echo !ESC![96mConfiguring TEST_IXA!ESC![0m
echo.
cmake ../../INCHI-1-SRC/INCHI_API/demos/test_ixa/src
echo.
echo !ESC![96mBuilding TEST_IXA!ESC![0m
echo.
if exist *.sln ( cmake --build . --target ALL_BUILD --config Release ) else ( cmake --build . )
echo.
echo.
echo !ESC![92m========== BUILD SUCCEEDED ==========!ESC![0m
echo | set /p="!ESC![96mtest_ixa.exe & libinchi.dll!ESC![0m are located in directory "
if exist bin\Release\ ( echo !ESC![96mCMake_build/test_ixa_build/bin/Release!ESC![0m ) else ( echo !ESC![96mCMake_build/test_ixa_build/bin!ESC![0m )
echo.
if /i "%comspec% /c %~0 " equ "%cmdcmdline:"=%" pause
goto :eof

:mol2inchi
echo.
echo.
echo !ESC![96mCreating build directory!ESC![0m
echo.
mkdir ..\CMake_build
cd ..\CMake_build
echo.
echo !ESC![96mCreating build directory for MOL2INCHI!ESC![0m
if exist mol2inchi_build ( rmdir "mol2inchi_build" /S /Q )
mkdir mol2inchi_build
cd mol2inchi_build
echo.
echo !ESC![96mConfiguring MOL2INCHI!ESC![0m
echo.
cmake ../../INCHI-1-SRC/INCHI_API/demos/mol2inchi/src
echo.
echo !ESC![96mBuilding MOL2INCHI!ESC![0m
echo.
if exist *.sln ( cmake --build . --target ALL_BUILD --config Release ) else ( cmake --build . )
echo.
echo.
echo !ESC![92m========== BUILD SUCCEEDED ==========!ESC![0m
echo | set /p="!ESC![96mmol2inchi.exe & libinchi.dll!ESC![0m are located in directory "
if exist bin\Release\ ( echo !ESC![96mCMake_build/mol2inchi_build/bin/Release!ESC![0m ) else ( echo !ESC![96mCMake_build/mol2inchi_build/bin!ESC![0m )
echo.
if /i "%comspec% /c %~0 " equ "%cmdcmdline:"=%" pause
goto :eof

:allinchi
echo.
echo.
echo !ESC![96mCreating build directory!ESC![0m
echo.
mkdir ..\CMake_build
cd ..\CMake_build
echo.
echo !ESC![96mCreating build directory for InChI CLI!ESC![0m
if exist cli_build ( rmdir "cli_build" /S /Q )
mkdir cli_build
cd cli_build
echo.
echo !ESC![96mConfiguring InChI CLI!ESC![0m
echo.
cmake ../../INCHI-1-SRC/INCHI_EXE/inchi-1/src
echo.
echo !ESC![96mBuilding InChI CLI!ESC![0m
echo.
if exist *.sln ( cmake --build . --target ALL_BUILD --config Release ) else ( cmake --build . )
echo.
cd ..
echo.
echo !ESC![96mCreating build directory for LIBINCHI!ESC![0m
if exist libinchi_build ( rmdir "libinchi_build" /S /Q )
mkdir libinchi_build
cd libinchi_build
echo.
echo !ESC![96mConfiguring LIBINCHI!ESC![0m
echo.
cmake ../../INCHI-1-SRC/INCHI_API/libinchi/src
echo.
echo !ESC![96mBuilding LIBINCHI!ESC![0m
echo.
if exist *.sln ( cmake --build . --target ALL_BUILD --config Release ) else ( cmake --build . )
echo.
cd ..
echo.
echo !ESC![96mCreating build directory for INCHI_MAIN!ESC![0m
if exist inchi_main_build ( rmdir "inchi_main_build" /S /Q )
mkdir inchi_main_build
cd inchi_main_build
echo.
echo !ESC![96mConfiguring INCHI_MAIN!ESC![0m
echo.
cmake ../../INCHI-1-SRC/INCHI_API/demos/inchi_main/src
echo.
echo !ESC![96mBuilding INCHI_MAIN!ESC![0m
echo.
if exist *.sln ( cmake --build . --target ALL_BUILD --config Release ) else ( cmake --build . )
echo.
cd ..
echo.
echo !ESC![96mCreating build directory for TEST_IXA!ESC![0m
if exist test_ixa_build ( rmdir "test_ixa_build" /S /Q )
mkdir test_ixa_build
cd test_ixa_build
echo.
echo !ESC![96mConfiguring TEST_IXA!ESC![0m
echo.
cmake ../../INCHI-1-SRC/INCHI_API/demos/test_ixa/src
echo.
echo !ESC![96mBuilding TEST_IXA!ESC![0m
echo.
if exist *.sln ( cmake --build . --target ALL_BUILD --config Release ) else ( cmake --build . )
echo.
cd ..
echo.
echo !ESC![96mCreating build directory for MOL2INCHI!ESC![0m
if exist mol2inchi_build ( rmdir "mol2inchi_build" /S /Q )
mkdir mol2inchi_build
cd mol2inchi_build
echo.
echo !ESC![96mConfiguring MOL2INCHI!ESC![0m
echo.
cmake ../../INCHI-1-SRC/INCHI_API/demos/mol2inchi/src
echo.
echo !ESC![96mBuilding MOL2INCHI!ESC![0m
echo.
if exist *.sln ( cmake --build . --target ALL_BUILD --config Release ) else ( cmake --build . ) 
echo.
echo.
echo !ESC![92m========== BUILD SUCCEEDED ==========!ESC![0m
echo | set /p="!ESC![96minchi-1.exe!ESC![0m is located in directory "
if exist bin\Release\ ( echo !ESC![96mCMake_build/cli_build/bin/Release!ESC![0m ) else ( echo !ESC![96mCMake_build/cli_build/bin!ESC![0m )
echo | set /p="!ESC![96mlibinchi.dll!ESC![0m is located in directory "
if exist bin\Release\ ( echo !ESC![96mCMake_build/libinchi_build/bin/Release!ESC![0m ) else ( echo !ESC![96mCMake_build/libinchi_build/bin!ESC![0m )
echo | set /p="!ESC![96minchi_main.exe & libinchi.dll!ESC![0m are located in directory "
if exist bin\Release\ ( echo !ESC![96mCMake_build/inchi_main_build/bin/Release!ESC![0m ) else ( echo !ESC![96mCMake_build/inchi_main_build/bin!ESC![0m )
echo | set /p="!ESC![96mtest_ixa.exe & libinchi.dll!ESC![0m are located in directory "
if exist bin\Release\ ( echo !ESC![96mCMake_build/test_ixa_build/bin/Release!ESC![0m ) else ( echo !ESC![96mCMake_build/test_ixa_build/bin!ESC![0m )
echo | set /p="!ESC![96mmol2inchi.exe & libinchi.dll!ESC![0m are located in directory "
if exist bin\Release\ ( echo !ESC![96mCMake_build/mol2inchi_build/bin/Release!ESC![0m ) else ( echo !ESC![96mCMake_build/mol2inchi_build/bin!ESC![0m )
echo.
if /i "%comspec% /c %~0 " equ "%cmdcmdline:"=%" pause
goto :eof
