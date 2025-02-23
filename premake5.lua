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

  filter "system:windows"
   removefiles { "src/term_priv_linux.c" }
   filter "system:linux"
   removefiles { "src/term_priv_win32.c" }

function tests_project(name, lib, platform_libs)
 project(name)
  kind "ConsoleApp"
  language "C"
  includedirs "include"
  files { "tests/%{prj.name}.c", "tests/test_utils.c" }
  links { "term-display" }

  if lib then
   links(lib)
  end

  if platform_libs then
   for platform, libs in pairs(platform_libs) do
    filter { "system:" .. platform }
     links(libs)
   end
   filter {}
  end
end

tests_project("rgb_scrolling", { "m" }, {})
tests_project("noise", {}, { ["windows"] = {"Bcrypt"} })
tests_project("multiline-text", {}, {})
tests_project("kbinput", {}, {})
