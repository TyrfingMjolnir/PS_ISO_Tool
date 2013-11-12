@echo off
:: NOTE: Change the paths based on your system...

:: MinGW will process the makefile, but MS Visual C++ Tools will compile everything
set MINGW_PATH=C:\MinGW\bin
set MSYS_PATH=C:\msys\1.0\bin

set VSINSTALLDIR=C:\Program^ Files^ (x86)\Microsoft^ Visual^ Studio^ 11.0
set VCINSTALLDIR=C:\Program^ Files^ (x86)\Microsoft^ Visual^ Studio^ 11.0\VC
set MSSDKDIR=C:\Program^ Files^ (x86)\Microsoft^ SDKs\Windows\v7.1A

:: ---------------------------------------------------------------------------
:: DO NOT EDIT PAST THIS LINE...
call "%VCINSTALLDIR%\vcvarsall.bat" x86
cls
set PATH=%MINGW_PATH%;%MSYS_PATH%;%VCINSTALLDIR%\bin;%VSINSTALLDIR%\Common7\Tools;%VSINSTALLDIR%\Common7\IDE;%VCINSTALLDIR%\VCPackages;%PATH%
set INCLUDE=%VCINSTALLDIR%\Include;%MSSDKDIR%\include;%INCLUDE%
set LIB=%VCINSTALLDIR%\lib;%MSSDKDIR%\lib%LIB%
set LIBPATH=%VCINSTALLDIR%\lib;%MSSDKDIR%\lib

mingw32-make -f Makefile.vc

pause

