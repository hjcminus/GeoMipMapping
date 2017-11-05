workspace "GeoMipMapping"
	configurations { "Debug", "Release" }
	platforms { "Win32", "x64" }
	location "%{_ACTION}"
	
	filter { "platforms:Win32" }
		system "Windows"
		architecture "x86"
		
	filter { "platforms:x64" }
		system "Windows"
		architecture "x86_64"
		
	filter "configurations:Debug"
		defines { "_DEBUG" }
		symbols "On"
			
	filter "configurations:Release"
		defines { "NDEBUG" }
		symbols "On"
		optimize "Speed"
	filter {} -- close filter scope
	
project "GeoMipMapping"
	location "../build/%{_ACTION}/projects" -- put '../build/' prefix or 'includedirs' will generate absolute path
	kind "ConsoleApp"
	language "C++"
	targetdir "../bin/%{_ACTION}/%{cfg.platform}/%{cfg.buildcfg}"
	objdir "../intermediate/%{_ACTION}/%{cfg.platform}/%{cfg.buildcfg}/GeoMipMapping"
	links { "d3d11.lib", "libpng16.lib" }
	
	defines {
		"WIN32",
		"_CONSOLE"
	}
	
	files {
		"../source/**.h",
		"../source/**.cpp"
	}
	
	pchheader "Precompiled.h"
	pchsource "../source/Precompiled.cpp"
	
	includedirs { 
		"../external/lpng1615/include",
		"../source"
	}
	
	filter "platforms:Win32"
		libdirs { 
			"../external/lpng1615/lib/x86"
		}
		
		-- the path must using backslash and the path is relative to project's directory
		postbuildcommands { "if not exist ..\\..\\..\\bin\\%{_ACTION}\\%{cfg.platform}\\%{cfg.buildcfg}\\libpng16.dll ( \
copy ..\\..\\..\\external\\lpng1615\\lib\\x86\\libpng16.dll ..\\..\\..\\bin\\%{_ACTION}\\%{cfg.platform}\\%{cfg.buildcfg})"
		}
		
	filter "platforms:x64"
		libdirs { 
			"../external/lpng1615/lib/x64"
		}
		postbuildcommands { "if not exist ..\\..\\..\\bin\\%{_ACTION}\\%{cfg.platform}\\%{cfg.buildcfg}\\libpng16.dll ( \
copy ..\\..\\..\\external\\lpng1615\\lib\\x64\\libpng16.dll ..\\..\\..\\bin\\%{_ACTION}\\%{cfg.platform}\\%{cfg.buildcfg})"
		}
	
	filter "files:../source/DXApp.cpp"
		flags { "NoPCH" }
		
	filter {}
	
	