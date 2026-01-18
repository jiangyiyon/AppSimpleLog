@echo off
rem Creates thirdproj folder and clones libzip into it (requires git)
if not exist thirdproj (
  mkdir thirdproj
)
if not exist thirdproj\libzip (
  git clone https://github.com/nih-at/libzip.git thirdproj\libzip
) else (
  echo thirdproj\libzip already exists
)
exit /b 0
