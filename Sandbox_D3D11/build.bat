@REM --------------------------------------------------------------------------
@REM Setup
@REM --------------------------------------------------------------------------
@setlocal
@set PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build;%PATH%
@set OUT_DIR=..\Debug
@set OUT_EXE=sandbox_win32

@REM --------------------------------------------------------------------------
@REM Include paths
@REM --------------------------------------------------------------------------
@set INCLUDES=/I. /I.\.. /I "%WindowsSdkDir%Include\um" /I "%WindowsSdkDir%Include\shared" /I "%DXSDK_DIR%Include"
@set INCLUDES=/Isrc /Ivendor/imgui /Ivendor/imgui/backends /Ivendor/stb %INCLUDES%

@REM --------------------------------------------------------------------------
@REM Sources
@REM --------------------------------------------------------------------------
@set SOURCES=vendor/imgui/*.cpp vendor/imgui/backends/*.cpp
@set SOURCES= src/*.cpp main.cpp mvSandbox.cpp %SOURCES%

IF NOT EXIST %OUT_DIR% mkdir %OUT_DIR%

@REM Setup development environment
@call vcvarsall.bat amd64

@REM --------------------------------------------------------------------------
@REM Compiler flags
@REM --------------------------------------------------------------------------
set CommonCompilerFlags=/nologo /Zi /MD %INCLUDES% /std:c++17
set CommonCompilerFlags=/D _USE_MATH_DEFINES /D _DEBUG /D MV_DEBUG %CommonCompilerFlags% 
set CommonCompilerFlags=ucrtd.lib %CommonCompilerFlags% 

@REM --------------------------------------------------------------------------
@REM Linker flags
@REM --------------------------------------------------------------------------
set CommonLinkerFlags=/LIBPATH:%OUT_DIR% /LIBPATH:..\dependencies\Keyence /LIBPATH:"%DXSDK_DIR%/Lib/x86"
set CommonLinkerFlags=d3d11.lib d3dcompiler.lib %CommonLinkerFlags%

@REM --------------------------------------------------------------------------
@REM Build
@REM --------------------------------------------------------------------------
cl %CommonCompilerFlags% %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %CommonLinkerFlags%

@REM --------------------------------------------------------------------------
@REM Cleanup
@REM --------------------------------------------------------------------------
del %OUT_DIR%\*.obj
@endlocal