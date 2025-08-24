-- set minimum xmake version
set_xmakever("2.8.2")

includes("lib/commonlibsse-ng")

set_project("FremSKSE")
set_version("1.0.0")
set_license("GPL-3.0")

set_languages("c++23")
set_warnings("allextra")

set_policy("package.requires_lock", true)

add_rules("mode.release")
add_rules("plugin.vsxmake.autoupdate")

-- targets
target("FremSKSE")
    add_deps("commonlibsse-ng")

    add_rules("commonlibsse-ng.plugin", {
       name = "FremSKSE",
       author = "Frem <discord: frem_xdx>",
       description = "SKSE64 plugin template using CommonLibSSE-NG and PrismaUI"
    })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")

after_build(function (target)
    import("core.project.config")
    import("core.tool.compiler")
    import("core.project.project")
    
    -- Формирование пути к собранной .dll
    local target_file = path.join(target:targetdir(), target:filename())
    
    -- Путь назначения
    local dest_dir = "D:/STB/[STB] Mod Organizer/mods/FremUI/SKSE/Plugins"
    
    -- Убедимся, что каталог назначения существует
    os.mkdir(dest_dir)
    
    -- Копирование файла
    os.cp(target_file, dest_dir)
    
    -- Вывод информации о копировании
    print(string.format("Copied %s to %s", target_file, dest_dir))
end)