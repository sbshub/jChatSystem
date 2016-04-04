-- [[
-- This file is part of the jChatSystem project.
--
-- This program is licensed under the GNU General
-- Public License. To view the full license, check
-- LICENSE in the project root.
--]]

workspace "jchat"
	configurations { "Debug", "Release" }
	platforms { "Win32", "Win64", "Unix32", "Unix64" }

	project "jchat_server"
		kind "ConsoleApp"
		language "C++"
		targetdir "build/%{cfg.buildcfg}"

		includedirs { "jchat_lib/", "jchat_common/", "jchat_server/include/", "jchat_server/src/" }
		files { "jchat_lib/**", "jchat_common/**", "jchat_server/include/**", "jchat_server/src/**.cpp" }

		filter "platforms:Win32"
			architecture "x32"
			links { "ws2_32" }

		filter "platforms:Win64"
			architecture "x64"
			links { "ws2_32" }

		filter "platforms:Unix32"
			architecture "x32"

		filter "platforms:Unix64"
			architecture "x64"

		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols" }

		configuration "Release"
			defines { "NDEBUG" }
			optimize "On"

		configuration { "gmake" }
			buildoptions { "-std=c++11" }
			linkoptions { "-pthread" }

	project "jchat_client"
		kind "ConsoleApp"
		language "C++"
		targetdir "build/%{cfg.buildcfg}"

		includedirs { "jchat_lib/", "jchat_common/", "jchat_client/include/", "jchat_client/src/" }
		files { "jchat_lib/**", "jchat_common/**", "jchat_client/include/**", "jchat_client/src/**.cpp" }

		filter "platforms:Win32"
			architecture "x32"
			links { "ws2_32" }

		filter "platforms:Win64"
			architecture "x64"
			links { "ws2_32" }

		filter "platforms:Unix32"
			architecture "x32"

		filter "platforms:Unix64"
			architecture "x64"

		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols" }

		configuration "Release"
			defines { "NDEBUG" }
			optimize "On"

		configuration { "gmake" }
			buildoptions { "-std=c++11" }
			linkoptions { "-pthread" }
