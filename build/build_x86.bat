@echo off
set /p vs="select a build target (1: vs2013, 2: vs2015): "
if %vs% == 1 (
	premake5 vs2013
	"%VS120COMNTOOLS%..\IDE\devenv.com" vs2013\GeoMipMapping.sln /build "Release|Win32"
)
if %vs% == 2 (
	premake5 vs2015
	"%VS140COMNTOOLS%..\IDE\devenv.com" vs2015\GeoMipMapping.sln /build "Release|Win32"
)
pause