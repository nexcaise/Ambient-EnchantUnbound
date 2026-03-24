set_project("EnchantUnbound")
set_version("1.0.0")

set_languages("cxx23")

add_rules("mode.release")

add_repositories(
    "xmake-repo https://github.com/xmake-io/xmake-repo.git"
)

target("EnchantUnbound")
    set_kind("shared")
    add_linkdirs("api/libs/arm64-v8a")
    add_links("nise", "log", "miniAPI", "GlossHook")

    add_files("src/*.cpp", "src/**/*.cpp", "api/deps/gamepwnage/src/*.c", "api/deps/gamepwnage/src/**/*.c")

    add_includedirs("include", "api/include", "api/deps/gamepwnage/includes", {public = true})

    add_cxflags("-O2", "-fvisibility=hidden", "-ffunction-sections", "-fdata-sections", "-w")
    add_cflags("-O2", "-fvisibility=hidden", "-ffunction-sections", "-fdata-sections", "-w")
    add_ldflags("-Wl,--gc-sections,--strip-all", "-s")
