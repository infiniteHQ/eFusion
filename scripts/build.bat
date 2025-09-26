@echo off
setlocal enabledelayedexpansion

if exist ..\build rd /s /q ..\build
if exist ..\dist rd /s /q ..\dist

if exist ..\lib\vortex\tests\project\.vx\modules rd /s /q ..\lib\vortex\tests\project\.vx\modules

mkdir ..\build
cd ..\build

cmake -G "Visual Studio 17" -A x64 ..

for /f %%i in ('powershell -command "(Get-WmiObject -Class Win32_Processor).NumberOfLogicalProcessors"') do set THREADS=%%i

cmake --build . --config Release -- /m:%THREADS%
cmake --install . --config Release

cd ..\scripts

for /f "tokens=*" %%A in ('powershell -command "Get-Content ../module.json | ConvertFrom-Json | Select-Object -Property name,version | ForEach-Object { $_.name + ' ' + $_.version }"') do (
    for /f "tokens=1,2" %%B in ("%%A") do (
        set NAME=%%B
        set VERSION=%%C
    )
)

mkdir ..\dist

xcopy ..\build ..\dist\build /E /I /Y
xcopy ..\lib ..\dist\lib /E /I /Y 2>nul
xcopy ..\assets ..\dist\assets /E /I /Y 2>nul
copy ..\module.json ..\dist\module.json /Y

rd /s /q ..\dist\build\CMakeFiles 2>nul
rd /s /q ..\dist\scripts 2>nul
rd /s /q ..\dist\.git 2>nul
rd /s /q ..\dist\.vscode 2>nul
rd /s /q ..\dist\lib 2>nul
del /q ..\dist\buil_
