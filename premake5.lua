workspace "term-display"
 configurations { "Debug", "Release" }

 filter "configurations:Debug"
  defines { "DEBUG" }
  runtime "Debug"
  symbols "On"

 filter "configurations:Release"
  defines { "NDEBUG" }
  runtime "Release"
  optimize "On"

 project "term-display"
  kind "StaticLib"
  language "C"
  targetdir "bin/%{cfg.buildcfg}"
  files { "src/*.c" }
  includedirs { "include" }

function tests_project(name, lib)
 project(name)
  kind "ConsoleApp"
  language "C"
  includedirs "include"
  files { "tests/%{prj.name}.c" }
  links { "term-display", lib }
end

tests_project("rgb-scrolling", { "m" })
tests_project("noise", {})
tests_project("multiline-text")
