local MUSA_ROOT = os.getenv("MUSA_ROOT") or os.getenv("MUSA_HOME") or os.getenv("MUSA_PATH")
add_includedirs(MUSA_ROOT .. "/include")
add_linkdirs(MUSA_ROOT .. "/lib")
add_links("musa", "musart", "mudnn", "mublas")

rule("mu")
    set_extensions(".mu")
    on_load(function (target)
        target:add("includedirs", "include")
    end)

    on_build_file(function (target, sourcefile)
        local objectfile = target:objectfile(sourcefile)
        os.mkdir(path.directory(objectfile))

        local mcc = MUSA_ROOT .. "/bin/mcc"
        local includedirs = table.concat(target:get("includedirs"), " ")

        local args = {"-c", sourcefile, "-o", objectfile, "-I" .. MUSA_ROOT .. "/include", "-O3", "-fPIC", "-Wall", "-std=c++17", "-pthread"}
        for _, includedir in ipairs(target:get("includedirs")) do
            table.insert(args, "-I" .. includedir)
        end

        -- ============================
        -- Retrieve all preprocessor defines added to the current target
        local defines = target:get("defines")
        if defines then
            for _, define in ipairs(defines) do
                table.insert(args, "-D" .. define)
            end
        end
        -- ===================================

        os.execv(mcc, args)
        table.insert(target:objectfiles(), objectfile)
    end)
rule_end()

target("infiniop-moore")
    set_kind("static")
    on_install(function (target) end)
    set_languages("cxx17")
    set_warnings("all", "error")
    add_cxflags("-lstdc++", "-fPIC", "-Wno-comment")
    add_files("../src/infiniop/devices/moore/*.cc")
    add_files("../src/infiniop/ops/*/moore/*.mu", {rule = "mu"})
target_end()

target("infinirt-moore")
    set_kind("static")
    set_languages("cxx17")
    on_install(function (target) end)
    add_deps("infini-utils")
    set_warnings("all", "error")
    add_cxflags("-lstdc++", "-fPIC")
    add_files("../src/infinirt/moore/*.cc")
target_end()
