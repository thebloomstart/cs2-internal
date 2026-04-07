@echo off
echo [~] Cleaning build artifacts...

:: Deleting commonly ignored folders
if exist "build" rmdir /s /q "build"
if exist "intermediate" rmdir /s /q "intermediate"
if exist "bin" rmdir /s /q "bin"
if exist "bin-int" rmdir /s /q "bin-int"
if exist ".vs" rmdir /s /q ".vs"

:: Deleting project files
del /q /s *.sln *.vcxproj *.vcxproj.filters *.vcxproj.user *.suo *.db *.opendb *.sdf *.ipch *.pdb *.ilk *.exe *.dll 2>nul

echo [!] Clean finished.
pause
