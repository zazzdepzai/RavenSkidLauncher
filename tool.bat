@echo off
setlocal EnableExtensions EnableDelayedExpansion
title RavenSkidLauncher - FULL REUPLOAD

echo ==========================================
echo   RavenSkidLauncher FULL REUPLOAD TOOL
echo ==========================================
echo.
echo This will REPLACE the remote main branch
echo with the files in this current folder.
echo Old files not present locally will disappear
echo from the main branch.
echo.
set /p "CONFIRM=Type DELETE to continue: "
if /I not "%CONFIRM%"=="DELETE" (
    echo.
    echo Cancelled.
    pause
    exit /b 0
)

echo.
set "REPO=https://github.com/zazzdepzai/RavenSkidLauncher.git"

if not exist ".git" (
    echo [1/7] Initializing Git...
    git init
    if errorlevel 1 goto :error
) else (
    echo [1/7] Existing Git repository detected.
)

echo [2/7] Setting branch to main...
git branch -M main
if errorlevel 1 goto :error

echo [3/7] Setting remote...
git remote remove origin >nul 2>&1
git remote add origin "%REPO%"
if errorlevel 1 goto :error

echo [4/7] Removing old local Git index...
git rm -r --cached . >nul 2>&1

echo [5/7] Adding ALL current project files...
git add -A
if errorlevel 1 goto :error

echo [6/7] Creating clean replacement commit...
git commit --allow-empty -m "Clean reupload RavenSkidLauncher"
if errorlevel 1 (
    echo Commit failed.
    goto :error
)

echo [7/7] FORCE PUSHING clean repository to GitHub...
echo.
git push -u origin main --force
if errorlevel 1 goto :error

echo.
echo ==========================================
echo SUCCESS
echo ==========================================
echo Remote main has been replaced by this folder.
echo.
echo Repo:
echo %REPO%
echo.
pause
exit /b 0

:error
echo.
echo ==========================================
echo ERROR
echo ==========================================
echo The upload failed. Check the message above.
echo.
pause
exit /b 1
