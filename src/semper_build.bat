@if [%1]==[] @goto usage
@goto Run

:Usage
echo.%~nx0 [flags and arguments]
echo.
echo.Available flags:
echo.  -h  Display this help message
echo.
echo.Available arguments:
echo.  -c Release ^| Debug ^|
echo.     Set the configuration (default: Debug)
exit /b 127

:Run
@set S_COMPILER_FLAGS_FINAL=%S_COMPILER_FLAGS%
@set S_LINKER_FLAGS_FINAL=%S_LINKER_FLAGS%
@set S_LINK_LIBRARIES_FINAL=%S_LINK_LIBRARIES%
@set S_INCLUDE_DIRECTORIES_FINAL=%S_INCLUDE_DIRECTORIES%
@set S_LINK_DIRECTORIES_FINAL=%S_LINK_DIRECTORIES%
@set S_DEFINES_FINAL=%S_DEFINES%
@set S_FINAL_RESULT=[1m[36mPending.[0m
@set S_CONFIG_INFO=Debug

:CheckOpts
@if "%~1"=="-h" @goto Usage
@if "%~1"=="-i" @goto PrintInfo
@if "%~1"=="-c" (@set conf=%2) & @shift & @shift & @goto CheckOpts

@if "%conf%" EQU "Release" (
    @set S_COMPILER_FLAGS_FINAL=%S_COMPILER_FLAGS_FINAL% %S_RELEASE_COMPILER_FLAGS%
    @set S_LINKER_FLAGS_FINAL=%S_LINKER_FLAGS_FINAL% %S_RELEASE_LINKER_FLAGS%
    @set S_LINK_LIBRARIES_FINAL=%S_LINK_LIBRARIES_FINAL% %S_RELEASE_LINK_LIBARIES%
    @set S_DEFINES_FINAL=%S_DEFINES_FINAL% %S_RELEASE_DEFINES%
    @set S_LINK_DIRECTORIES_FINAL=%S_LINK_DIRECTORIES_FINAL% %S_RELEASE_LINK_DIRECTORIES%
    @set S_FINAL_RESULT=[1m[36mPending.[0m
    @set S_CONFIG_INFO=Release
    )
@if "%conf%" EQU "Debug" (
    @set S_COMPILER_FLAGS_FINAL=%S_COMPILER_FLAGS_FINAL% %S_DEBUG_COMPILER_FLAGS%
    @set S_LINKER_FLAGS_FINAL=%S_LINKER_FLAGS_FINAL% %S_DEBUG_LINKER_FLAGS%
    @set S_LINK_LIBRARIES_FINAL=%S_LINK_LIBRARIES_FINAL% %S_DEBUG_LINK_LIBARIES%
    @set S_DEFINES_FINAL=%S_DEFINES_FINAL% %S_DEBUG_DEFINES%
    @set S_LINK_DIRECTORIES_FINAL=%S_LINK_DIRECTORIES_FINAL% %S_DEBUG_LINK_DIRECTORIES%
    @set S_FINAL_RESULT=[1m[36mPending.[0m
    @set S_CONFIG_INFO=Debug
    )

@REM Final flag consolidation (don't touch)
@REM --------------------------------------------------------------------------
@set CommonCompilerFlags=%S_COMPILER_FLAGS_FINAL% %S_DEFINES_FINAL%
@set CommonLinkerFlags=%S_LINKER_FLAGS_FINAL% %S_LINK_DIRECTORIES_FINAL% %S_LINK_LIBRARIES_FINAL% 

@if EXIST %S_OUT_DIR%\%S_OUT_BIN% del %S_OUT_DIR%\%S_OUT_BIN%
@if NOT EXIST %S_OUT_DIR% mkdir %S_OUT_DIR%

@REM --------------------------------------------------------------------------
@REM Build
@REM --------------------------------------------------------------------------

@if %S_STATIC_LIB% EQU 0 (
    @echo [1m[36mCompiling and linking...[0m
    cl %S_INCLUDE_DIRECTORIES% %CommonCompilerFlags% /permissive- %S_SOURCES% /Fe%S_OUT_DIR%/%S_OUT_BIN% /Fo%S_OUT_DIR%/ /link %CommonLinkerFlags%
)

@if %S_STATIC_LIB% EQU 1 (
    @echo [1m[36mCompiling...[0m
    cl /c %S_INCLUDE_DIRECTORIES% %CommonCompilerFlags% /permissive- %S_SOURCES% /Fe%S_OUT_DIR%/%S_OUT_BIN% /Fo%S_OUT_DIR%/
)

@if %ERRORLEVEL% EQU 0 ( goto Linking)

@if %ERRORLEVEL% NEQ 0 (
    echo [1m[91mCompilation Failed with error code[0m: %ERRORLEVEL%
    @set S_FINAL_RESULT=[1m[91mFailed.[0m
    goto Cleanup  
)

@REM --------------------------------------------------------------------------
@REM Linking
@REM --------------------------------------------------------------------------
:Linking
    @echo [1m[92mCompiled successfully.[0m   
    @if %S_STATIC_LIB% EQU 1 (
        @echo [1m[36mLinking...[0m
        lib /nologo /OUT:%S_OUT_DIR%/%S_OUT_BIN% %S_OUT_DIR%\*.obj
    )
    
    @echo [1m[92mLinked successfully.[0m
    @set S_FINAL_RESULT=[1m[92mSuccessful.[0m
    goto Cleanup

@REM --------------------------------------------------------------------------
@REM Cleanup
@REM --------------------------------------------------------------------------
:Cleanup
    @echo [1m[36mCleaning up intermediate files...[0m
    del %S_OUT_DIR%\*.obj

@REM --------------------------------------------------------------------------
@REM Information Output
@REM --------------------------------------------------------------------------
:PrintInfo
@echo [36m--------------------------------------------------------------------------[0m
@echo [1m[93m                        Build Information [0m
@echo [36mResults:             [0m %S_FINAL_RESULT%
@echo [36mConfiguration:       [0m [35m%S_CONFIG_INFO%[0m
@echo [36mWorking directory:   [0m [35m%dir%[0m
@echo [36mOutput directory:    [0m [35m%S_OUT_DIR%[0m
@echo [36mOutput binary:       [0m [33m%S_OUT_BIN%[0m
@echo [36mLink libraries:      [0m [35m%S_LINK_LIBRARIES_FINAL%[0m
@echo [36mDefines:             [0m [33m%S_DEFINES_FINAL%[0m
@echo [36mCompiler flags:      [0m [35m%S_COMPILER_FLAGS_FINAL%[0m
@echo [36mLinker flags:        [0m [33m%S_LINKER_FLAGS_FINAL%[0m
@echo [36mInclude directories: [0m [35m%S_INCLUDE_DIRECTORIES%[0m
@echo [36mLink directories:    [0m [33m%S_LINK_DIRECTORIES_FINAL%[0m
@echo [36mSources:             [0m [35m%S_SOURCES%[0m
@echo [36m--------------------------------------------------------------------------[0m
:End