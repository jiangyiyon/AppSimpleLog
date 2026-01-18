thirdproj folder and libzip download

This repository could not create the thirdproj folder or download libzip from this environment (no network / PowerShell Core not available).

To create the thirdproj folder and fetch libzip locally, run the included batch file from the repository root:

  download_libzip_into_thirdproj.bat

Alternatively run the commands manually from a shell in the repository root:

  mkdir thirdproj
  git clone https://github.com/nih-at/libzip.git thirdproj\libzip

After running, libzip will be available at thirdproj\libzip
