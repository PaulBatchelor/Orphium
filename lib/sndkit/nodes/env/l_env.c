#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lil/lil.h"
#include "graforge.h"
#include "core.h"
#include "sklil.h"

int sk_node_env(sk_core *core);

static lil_value_t env(lil_t lil, size_t argc, lil_value_t *argv)
{
    sk_core *core;
    int rc;
    int i;
    core = lil_get_data(lil);

    SKLIL_ARITY_CHECK(lil, "env", argc, 4);

    for (i = 0; i < 4; i ++) {
        rc = sklil_param(core, argv[i]);
        SKLIL_PARAM_CHECK(lil, rc, "env");
    }

    rc = sk_node_env(core);
    SKLIL_ERROR_CHECK(lil, rc, "env didn't work out.");
    return NULL;
}

void sklil_load_env(lil_t lil)
{
    lil_register(lil, "env", env);
}
