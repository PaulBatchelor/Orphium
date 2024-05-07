config = {}

for _,a in pairs(arg) do
end


function mkrule(name, command, description)
    return {name=name, command=command, description=description}
end

function mkbuild(outputs, rule, inputs, vars, deps)
    return {
        outputs = outputs,
        rule = rule,
        inputs = inputs,
        vars = vars,
        deps = deps
    }
end

rules = {}
build = {}

objects = {}

-- these don't have main
extra_objects = {}

tangled = {}

cc="gcc"

-- generated programs
programs = {}

-- phony targets
phony_targets = {}

cflags = {"-Wall", "-pedantic", "-O3", "-g"}

function insert_object(obj)
    obj_o = obj .. ".o"
    table.insert(build,
        mkbuild(obj_o,
            "c89",
            obj .. ".c"))
    table.insert(extra_objects, obj_o)
end

function insert_objects(t)
    for _,v in pairs(t) do
        insert_object(v)
    end
end

function add_object(obj)
    obj_o = obj .. ".o"
    table.insert(build,
        mkbuild(obj_o,
            "c89",
            obj .. ".c"))

    table.insert(objects, obj_o)
end

function add_c99object(obj)
    obj_o = obj .. ".o"
    table.insert(build,
        mkbuild(obj .. ".o",
            "c99",
            obj .. ".c"))
    table.insert(objects, obj_o)
end

function add_objects(objs)
    for _,o in pairs(objs) do
        add_object(o)
    end
end

function add_c99objects(objs)
    for _,o in pairs(objs) do
        add_c99object(o)
    end
end

function add_cflags(flags)
    for _,f in pairs(flags) do
        table.insert(cflags, f)
    end
end

function add_tangled_object(obj, header)
    if header == nil then header = true end

    tangled_c = obj .. ".c"
    tangled_files = tangled_c

    if header == true then
        tangled_files = {tangled_c, obj .. ".h"}
    end

    table.insert(build,
        mkbuild(tangled_files, "worgle", obj .. ".org", nil, "worglite"))

    table.insert(tangled, tangled_c)

    add_object(obj)
end

function add_tangled_objects(obj)
    for _,v in pairs(obj) do
        if (type(v) == "string") then
            add_tangled_object(v)
        else
            add_tangled_object(v[1], v[2])
        end
    end
end

table.insert(rules,
    mkrule("worgle", "util/worgle/worglite -Werror -g $in"))
table.insert(rules,
    mkrule("c89",
        cc .. " -std=c89 -c $cflags $in -o $out",
        "c89 $in"))
table.insert(rules,
    mkrule("c99",
        cc .. " -std=c99 -c $cflags $in -o $out",
        "c99 $in"))
table.insert(rules,
    mkrule("link",
        cc .. " $cflags $in -o $out $ldflags $libs",
        "creating $out"))
table.insert(rules,
    mkrule("ar", "ar rcs $out $in", "creating $out"))

-- dictionary generator
table.insert(rules,
    mkrule("gendict", "./tools/generate-dictionary", "generating dictionary"))

-- concat files (for konilo.pali amalgamation)
table.insert(rules,
    mkrule("cat", "cat $in > $out"))

-- generate blocks
table.insert(rules,
    mkrule("genblocks", "sh mkblocks.sh", "generating blocks"))

-- generate ilo rom
table.insert(rules,
    mkrule("genrom", "sh mkrom.sh", "generating rom"))

libs = {
    -- "-lm",

    -- -- used by SQLite
    -- "-lpthread",

    -- "-ldl",

    -- "-lreadline",
}

add_cflags({"-Ilib", "-I."})

function add_libs(p)
    for _,v in pairs(p) do
        table.insert(libs, "-l" .. v)
    end
end

function generate_ninja()
    fp = io.open("build.ninja", "w")
    objstr = table.concat(objects, " ")

    table.insert(build, mkbuild("liborphium.a", "ar", objstr))
    fp:write("cflags = " .. table.concat(cflags, " ") .."\n")
    fp:write("libs = " .. table.concat(libs, " ") .."\n")

    for _, v in pairs(rules) do
        fp:write("rule " .. v.name .. "\n")
        fp:write("    command = " .. v.command .. "\n")

        if v.description ~= nil then
            fp:write("    description = "
                .. v.description.. "\n")
        end
    end

    function process_files(f)
        if type(f) == "string" then
            return f
        end

        return table.concat(f, " ")
    end

    function process_deps(deps)
        if deps ~= nil then
            -- process_files has the same logic we need
            -- for implicit deps
            return " || " .. process_files(deps)
        end
        return ""
    end

    for _, v in pairs(build) do
        fp:write("build " ..
            process_files(v.outputs) ..
            ": " ..
            v.rule ..
            " " ..
            process_files(v.inputs) ..
            process_deps(v.deps) ..
            "\n")
        if v.vars ~= nil then
            for _, var in pairs(v.vars) do
                fp:write("    " .. var .. "\n")
            end
        end
    end

    -- phony targets
    for name,deps in pairs(phony_targets) do
        fp:write("build " ..
            name .. ": phony " ..
            table.concat(deps, " ") .. "\n")
    end

    fp:write("default orphium ilo.rom ilo.blocks\n")

    fp:close()
end

function generate_makefile()
    fp = io.open("Makefile", "w")
    objstr = table.concat(objects, " ")

    -- table.insert(build, mkbuild("liborphium.a", "ar", objstr))
    fp:write("CFLAGS = " .. table.concat(cflags, " ") .."\n")
    fp:write("LIBS = " .. table.concat(libs, " ") .."\n")

    fp:write("default: orphium\n")


    function process_files(f)
        if type(f) == "string" then
            return f
        end

        return table.concat(f, " ")
    end

    function process_deps(deps)
        if deps ~= nil then
            -- process_files has the same logic we need
            -- for implicit deps
            return " || " .. process_files(deps)
        end
        return ""
    end

    function process_rule(rule)
        if rule == "ar" then
            return "ar rcs $@ $^"
        elseif rule == "c89" then
            return "$(CC) -std=c89 $(CFLAGS) -c $< -o $@"
        elseif rule == "c99" then
            return "$(CC) -std=c99 $(CFLAGS) -c $< -o $@"
        elseif rule == "link" then
            return "$(CC) $(CFLAGS) $^ -o $@"
        else
            error(rule)
        end
    end

    local links = {}
    for _, v in pairs(build) do
        if v.rule == "link" then
            table.insert(links, v)
        else
            fp:write(process_files(v.outputs) ..
                ": " ..
                process_files(v.inputs) ..
                process_deps(v.deps) ..
                "\n\t" ..
                process_rule(v.rule) .. "\n"
                )
            if v.vars ~= nil then
                for _, var in pairs(v.vars) do
                    print("    " .. var .. "\n")
                end
            end
        end
    end

    for _,v in pairs(links) do
        fp:write(process_files(v.outputs) ..
            ": " ..
            process_files(v.inputs) ..
            process_deps(v.deps) ..
            "\n\t" ..
            process_rule(v.rule) .. "\n"
            )
    end

    fp:write("clean:\n")
    fp:write("\t $(RM) -rf " .. objstr .. " liborphium.a\n")
    -- remove programs and extra objects
    
    local progstr = ""
    local extraobjstr = ""

    if #programs > 0 then
        progrstr = table.concat(programs, " ")
    end

    if #extra_objects > 0 then
        extra_objects = table.concat(programs, " ")
    end

    fp:write("\t $(RM) -rf " .. progstr .. " liborphium.a\n")
    fp:write("\t $(RM) -rf " .. extraobjstr .. " liborphium.a\n")

    fp:close()
end

function insert_program(name, objs)
    objstr = ""
    if objs == nil then
        objs = name
    end
    if type(objs) == "string" then
        objstr = objs .. ".o"
    else
        objstr = table.concat(objs, ".o ")
    end
    table.insert(build,
        mkbuild(name, "link", objstr .. " liborphium.a"))
    table.insert(programs, name)
end

function phony(name)
    return function (t)
        phony_targets[name] = t
    end
end


add_objects {
    "lib/cmp/cmp",
    "obj",
    "moncmp",
    "buffer",
    "parse",
    "ilo",
    "bitrune",
    "lib/z85/z85",
}

-- this has main, so make a build rule but don't add
-- it to the object list via add_objects
insert_objects {
    "tools/extract_block",
    "tools/insert_block",
    "main",
    "ex/hello",
    "ex/bitrune_test",
    "tools/pali",
    "tools/generate-dictionary",
    "tools/retro-unu",
    "tools/block-import",
    "tools/block-export",
}

phony("tools") {
    "tools/extract_block",
    "tools/insert_block",
    "tools/pali",
    "tools/generate-dictionary",
    "tools/retro-unu",
    "tools/block-import",
    "tools/block-export",
}

insert_program("orphium", "main")
insert_program("tools/extract_block")
insert_program("tools/insert_block")
insert_program("ex/hello")
insert_program("ex/bitrune_test")
insert_program("tools/pali")
insert_program("tools/generate-dictionary")
insert_program("tools/retro-unu")
insert_program("tools/block-import")
insert_program("tools/block-export")

-- generate dictionary
table.insert(build,
    mkbuild("forth.dictionary", "gendict", "dict.data", nil, "tools/generate-dictionary"))

-- generate konilo.pali amalgamation
table.insert(build,
    mkbuild("konilo.pali", "cat", {"forth.pali", "forth.dictionary"}))

table.insert(build,
    mkbuild("ilo.blocks",
    "genblocks", "mkblocks.sh", nil,
    {"mkblocks.sh", "tools/block-import", "tools/insert_block"}))

table.insert(build,
    mkbuild("ilo.rom",
    "genrom", {"mkrom.sh", "konilo.pali", "extend.konilo"}, nil,
    {"mkrom.sh", "tools/pali", "tools/retro-unu", "ilo.blocks", "orphium"}))

require("lib/sndkit/config")

generate_ninja()
-- generate_makefile()
