@echo off
setlocal

echo [INFO] Looking for NDK-BUILD...

:: 1. Check if in PATH
where ndk-build >nul 2>nul
if %errorlevel%==0 (
    echo [OK] Found in PATH.
    call ndk-build NDK_PROJECT_PATH=. NDK_APPLICATION_MK=jni/Application.mk
    goto end
)

:: 2. Check common SDK paths
set "SDK_ROOT=%LOCALAPPDATA%\Android\Sdk"
if not exist "%SDK_ROOT%" set "SDK_ROOT=C:\Android\Sdk"

if exist "%SDK_ROOT%\ndk" (
    echo [INFO] Searching in %SDK_ROOT%\ndk...
    for /d %%D in ("%SDK_ROOT%\ndk\*") do (
        if exist "%%D\ndk-build.cmd" (
            echo [OK] Found NDK at %%D
            call "%%D\ndk-build.cmd" NDK_PROJECT_PATH=. NDK_APPLICATION_MK=jni/Application.mk
            goto end
        )
    )
)

echo [ERR] NDK not found!
echo Please set NDK_PATH or run this script from NDK directory.
pause
exit /b 1

:end
if %errorlevel%==0 (
    echo.
    echo [SUCCESS] Build Complete! Check /libs folder.
) else (
    echo.
    echo [FAIL] Build Failed!
)
pause
