set_project("EnchantLimitLess")
set_version("1.0.0")

set_languages("cxx23")

add_rules("mode.release")

add_repositories("xmake-repo https://github.com/xmake-io/xmake-repo.git")

add_requires("nlohmann_json v3.11.3")

target("EnchantLimitLess")
    set_kind("shared")
    add_packages("nlohmann_json")
    add_files("src/*.cpp", "src/**/*.cpp")
    add_linkdirs("libs/arm64-v8a")
    add_links("log")
    add_links("GlossHook", { kind = "static"})
    add_includedirs("include", {public = true})
