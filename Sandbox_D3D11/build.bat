@setlocal
@set PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build;%PATH%
@set PATH=C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;%PATH%
@set S_OUT_DIR=..\out
@set dir=%~dp0

@REM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@REM |                          Common Settings                               |
@REM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

@REM -------------------Setup development environment--------------------------
@call vcvarsall.bat amd64

@REM ------------------------Common Compiler Flags-----------------------------
@set S_COMPILER_FLAGS=/nologo /std:c++17 /EHsc
@set S_DEBUG_COMPILER_FLAGS=/Od /MDd /Zi
@set S_RELEASE_COMPILER_FLAGS=/O2 /MD

@REM --------------------------Common Linker flags-----------------------------
@set S_LINKER_FLAGS=
@set S_DEBUG_LINKER_FLAGS=
@set S_RELEASE_LINKER_FLAGS=

@REM --------------------------Common Include Paths----------------------------
@set S_INCLUDE_DIRECTORIES=/I. /I.. /I "%WindowsSdkDir%Include\um" /I "%WindowsSdkDir%Include\shared" /I "%DXSDK_DIR%Include"
@set S_INCLUDE_DIRECTORIES=/I../dependencies/imgui /I../dependencies/imgui/backends %S_INCLUDE_DIRECTORIES%
@set S_INCLUDE_DIRECTORIES=/I../dependencies/stb %S_INCLUDE_DIRECTORIES%

@REM --------------------------Common Library Paths----------------------------
@set S_LINK_DIRECTORIES=/LIBPATH:%S_OUT_DIR% /LIBPATH:"%DXSDK_DIR%/Lib/x86"
@set S_DEBUG_LINK_DIRECTORIES=
@set S_RELEASE_LINK_DIRECTORIES=

@REM --------------------------Common Link libs--------------------------------
@set S_LINK_LIBRARIES=
@set S_DEBUG_LINK_DIRECTORIES=
@set S_RELEASE_LINK_DIRECTORIES=

@REM -----------------------------Common Defines----------------------------------
@set S_DEFINES=/D _USE_MATH_DEFINES /D WIN32
@set S_DEBUG_DEFINES=/D _DEBUG
@set S_RELEASE_DEFINES=

@REM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@REM |                          Dependency Lib                                |
@REM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@set S_OUT_BIN=dependencies.lib
@set S_STATIC_LIB=1

@REM -----------------------------Sources--------------------------------------
@REM @set S_SOURCES=*.cpp
@set S_SOURCES=../dependencies/imgui/*.cpp                %S_SOURCES%
@set S_SOURCES=../dependencies/imgui/backends/*_dx11.cpp  %S_SOURCES%
@set S_SOURCES=../dependencies/imgui/backends/*_win32.cpp %S_SOURCES%

@REM ---------------------Run Semper build script------------------------------
@pushd %dir%
@if NOT EXIST %S_OUT_DIR%\%S_OUT_BIN% @call ..\dependencies\Semper\semper_build.bat -c Debug
@popd

@REM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@REM |                          Sandbox                                       |
@REM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@set S_OUT_BIN=sandbox.exe
@set S_STATIC_LIB=0

@REM -----------------------------Sources--------------------------------------
@set S_SOURCES=*.cpp ../*.cpp

@REM ----------------------------Libraries-------------------------------------
@set S_LINK_LIBRARIES=dependencies.lib d3d11.lib d3dcompiler.lib
@set S_DEBUG_LINK_LIBARIES=ucrtd.lib
@set S_RELEASE_LINK_LIBARIES=ucrt.lib


@REM ---------------------Run Semper build script------------------------------
@pushd %dir%
@if EXIST %S_OUT_DIR%\%S_OUT_BIN% del %S_OUT_DIR%\%S_OUT_BIN%
@call ..\dependencies\Semper\semper_build.bat -c Debug
@popd

