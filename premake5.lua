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

function tests_project(name, config)
 project(name)
  kind "ConsoleApp"
  language "C"
  includedirs "include"
  files { "tests/%{prj.name}.c", "tests/test_utils.c" }
  links { "term-display" }
  if config then
   if config.libs then
    links(config.libs)
   end

   if config.platform_libs then
    for platform, libs in pairs(config.platform_libs) do
     filter { "system:" .. platform }
      links(libs)
    end
    filter {}
   end

    if config.include_dirs then
     includedirs(config.include_dirs)
    end
  end
end

tests_project("rgb_scrolling", { libs={"m"}})
tests_project("noise", { platform_libs={["windows"]={"Bcrypt"}}})
tests_project("multiline_text")
tests_project("kbinput")
if os.isdir("tests/deps/stb") then
 tests_project("image_display", {include_dirs={"tests/deps/stb"}, libs={"m"}})
else
 print("Skip building image_display")
end
