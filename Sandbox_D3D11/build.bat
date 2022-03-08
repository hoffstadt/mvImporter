@setlocal
@set PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build;%PATH%
@set PATH=C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;%PATH%
@set S_OUT_DIR=..\out

@REM --------------------------------------------------------------------------
@REM --------------------------------------------------------------------------
@REM |                          ImGui                                         |
@REM --------------------------------------------------------------------------
@REM --------------------------------------------------------------------------
@set S_OUT_BIN=imgui.lib
@set S_STATIC_LIB=1
@set dir=%~dp0

@REM Include paths
@REM --------------------------------------------------------------------------
@set S_INCLUDE_DIRECTORIES=/I. /I "%WindowsSdkDir%Include\um" /I "%WindowsSdkDir%Include\shared" /I "%DXSDK_DIR%Include"
@set S_INCLUDE_DIRECTORIES=/I../dependencies/imgui /I../dependencies/imgui/backends %S_INCLUDE_DIRECTORIES%

@REM Sources
@REM --------------------------------------------------------------------------
@set S_SOURCES=../dependencies/imgui/*.cpp ../dependencies/imgui/backends/*_dx11.cpp ../dependencies/imgui/backends/*_win32.cpp

@REM Library paths
@REM --------------------------------------------------------------------------
@set S_LINK_DIRECTORIES=
@set S_DEBUG_LINK_DIRECTORIES=
@set S_RELEASE_LINK_DIRECTORIES=

@REM Libraries
@REM --------------------------------------------------------------------------
@set S_LINK_LIBRARIES=
@set S_DEBUG_LINK_LIBARIES=
@set S_RELEASE_LINK_LIBARIES=

@REM Defines
@REM --------------------------------------------------------------------------
@set S_DEFINES=/D _USE_MATH_DEFINES /D WIN32
@set S_DEBUG_DEFINES=/D _DEBUG
@set S_RELEASE_DEFINES=

@REM Compiler flags
@REM --------------------------------------------------------------------------
@set S_COMPILER_FLAGS=/nologo /std:c++17 /EHsc
@set S_DEBUG_COMPILER_FLAGS=/Od /MDd /Zi
@set S_RELEASE_COMPILER_FLAGS=/O2 /MD

@REM Linker flags
@REM --------------------------------------------------------------------------
@set S_LINKER_FLAGS=
@set S_DEBUG_LINKER_FLAGS=
@set S_RELEASE_LINKER_FLAGS=

@REM Setup development environment
@REM --------------------------------------------------------------------------
@call vcvarsall.bat amd64

@REM Run Semper build system
@REM --------------------------------------------------------------------------
@pushd %dir%
@if NOT EXIST %S_OUT_DIR%\%S_OUT_BIN% @call ..\dependencies\Semper\semper_build.bat -c Debug
@popd

@REM --------------------------------------------------------------------------
@REM --------------------------------------------------------------------------
@REM |                          Sandbox                                       |
@REM --------------------------------------------------------------------------
@REM --------------------------------------------------------------------------
@set S_OUT_BIN=sandbox.exe
@set S_STATIC_LIB=0

@REM Include paths
@REM --------------------------------------------------------------------------
@set S_INCLUDE_DIRECTORIES=/I. /I.. /I "%WindowsSdkDir%Include\um" /I "%WindowsSdkDir%Include\shared" /I "%DXSDK_DIR%Include"
@set S_INCLUDE_DIRECTORIES=/I../dependencies/imgui /I../dependencies/imgui/backends %S_INCLUDE_DIRECTORIES%
@set S_INCLUDE_DIRECTORIES=/I../dependencies/stb %S_INCLUDE_DIRECTORIES%

@REM Sources
@REM --------------------------------------------------------------------------
@set S_SOURCES=*.cpp

@REM Library paths
@REM --------------------------------------------------------------------------
@set S_LINK_DIRECTORIES=/LIBPATH:%S_OUT_DIR% /LIBPATH:"%DXSDK_DIR%/Lib/x86"
@set S_DEBUG_LINK_DIRECTORIES=
@set S_RELEASE_LINK_DIRECTORIES=

@REM Libraries
@REM --------------------------------------------------------------------------
@set S_LINK_LIBRARIES=imgui.lib d3d11.lib d3dcompiler.lib
@set S_DEBUG_LINK_LIBARIES=ucrtd.lib
@set S_RELEASE_LINK_LIBARIES=ucrt.lib

@REM Defines
@REM --------------------------------------------------------------------------
@set S_DEFINES=/D _USE_MATH_DEFINES /D WIN32
@set S_DEBUG_DEFINES=/D _DEBUG
@set S_RELEASE_DEFINES=

@REM Compiler flags
@REM --------------------------------------------------------------------------
@set S_COMPILER_FLAGS=/nologo /std:c++17 /EHsc
@set S_DEBUG_COMPILER_FLAGS=/Od /MDd /Zi
@set S_RELEASE_COMPILER_FLAGS=/O2 /MD

@REM Linker flags
@REM --------------------------------------------------------------------------
@set S_LINKER_FLAGS=
@set S_DEBUG_LINKER_FLAGS=
@set S_RELEASE_LINKER_FLAGS=

@REM Run Semper build system
@REM --------------------------------------------------------------------------
@pushd %dir%
@if EXIST %S_OUT_DIR%\%S_OUT_BIN% del %S_OUT_DIR%\%S_OUT_BIN%
@call ..\dependencies\Semper\semper_build.bat -c Debug
@popd

