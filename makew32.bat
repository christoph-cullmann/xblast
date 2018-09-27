@echo off
del xblast.exe
copy makefile.w32 makefile
cmake clean
cmake
strip xblast.exe
copy xblast.exe c:\d\games\xblast\xeblast.exe
zip -9 ..\XB300104.zip xblast.exe
cmake clean
sweep
del xblast.exe