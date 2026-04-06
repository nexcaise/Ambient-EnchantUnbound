set_project("EnchantUnbound")
set_version("1.0.0")

set_languages("cxx23")

add_rules("mode.release")

add_repositories("xmake-repo https://github.com/xmake-io/xmake-repo.git")

add_requires("nlohmann_json v3.11.3")

target("EnchantUnbound")
    set_kind("shared")
    add_packages("nlohmann_json")
    add_files("src/main.cpp")
    add_linkdirs("l/arm64")
    add_linkdirs("libs/arm64-v8a")
    add_links("log", "android", "dl")
    add_links("nexcaiseAPI", "GlossHook", {kind = "static"})
    add_includedirs("include", {public = true})
