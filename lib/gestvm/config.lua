add_objects {
    "lib/gestvm/gestvm",
	-- "lib/gestvm/l_gestvm",
	"lib/gestvm/uxn/uxn",
	"lib/gestvm/uxn/uxnasm",
	"lib/gestvm/gestlive",
    -- "lib/gestvm/memops"
}

add_cflags {
    "-DNO_UXNASM_MAIN"
}
