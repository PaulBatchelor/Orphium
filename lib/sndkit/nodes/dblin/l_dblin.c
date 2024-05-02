#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lil/lil.h"
#include "graforge.h"
#include "core.h"
#include "sklil.h"

int sk_node_dblin(sk_core *core);

static lil_value_t dblin(lil_t lil, size_t argc, lil_value_t *argv)
{
    sk_core *core;
    int rc;
    core = lil_get_data(lil);

    SKLIL_ARITY_CHECK(lil, "dblin", argc, 1);

    rc = sklil_param(core, argv[0]);
    SKLIL_PARAM_CHECK(lil, rc, "dblin");

    rc = sk_node_dblin(core);
    SKLIL_ERROR_CHECK(lil, rc, "dblin didn't work out.");
    return NULL;
}

void sklil_load_dblin(lil_t lil)
{
    lil_register(lil, "dblin", dblin);
}
