#line 12 "lib/gestvm/gestlive.org"
#include <stdlib.h>
#include <string.h>
#include "sndkit/graforge/graforge.h"
#include "sndkit/core.h"
#include "sndkit/lil/lil.h"
#include "sndkit/nodes/sklil.h"
#include "gestvm.h"
#include "gestlive.h"
#line 92 "lib/gestvm/gestlive.org"
struct gestlive * gestlive_new(void)
{
    struct gestlive *glive;
    int i;

    glive = NULL;

    glive = malloc(sizeof(struct gestlive));
    for (i = 0; i < 2; i++) {
        glive->uxn[i] = malloc(gestvm_uxn_sizeof());
        gestvm_uxn_init(glive->uxn[i]);
        glive->node[i].init = 0;
        glive->node[i].glive = glive;
    }

    glive->read = 1;
    glive->write = 0;
    glive->please_swap = 0;

    return glive;
}
#line 117 "lib/gestvm/gestlive.org"
void gestlive_del(struct gestlive *glive)
{
    free(glive->uxn[0]);
    free(glive->uxn[1]);
    free(glive);
}
#line 138 "lib/gestvm/gestlive.org"
gestvm_uxn * gestlive_get(struct gestlive *glive)
{
    if (glive->please_swap) {
        return NULL;
    }

    return glive->uxn[glive->write];
}
#line 157 "lib/gestvm/gestlive.org"
void gestlive_unlock(struct gestlive *glive)
{
    glive->please_swap = 0;
}
#line 175 "lib/gestvm/gestlive.org"
void gestlive_swap(struct gestlive *glive)
{
    glive->please_swap = 1;
}
#line 207 "lib/gestvm/gestlive.org"
void gestlive_update(struct gestlive *glive)
{
    if (glive->please_swap) {
        int tmp;
        tmp = glive->read;
        glive->read = glive->write;
        glive->write = tmp;

        /* re-init uxn VM instance */
        gestvm_uxn_init(glive->uxn[glive->write]);
        glive->please_swap = 0;
    }
}
#line 240 "lib/gestvm/gestlive.org"
#line 283 "lib/gestvm/gestlive.org"
static void swapper_compute(gf_node *node)
{
    struct gestlive_node *gl_node;

    gl_node = gf_node_get_data(node);

    if (gl_node->init) {
        gl_node->init = 0;
        gestlive_update(gl_node->glive);
    }
}
#line 251 "lib/gestvm/gestlive.org"
int sk_node_gestlive_swapper(sk_core *core)
{
    int rc;
    void *ud;
    gf_patch *patch;
    struct gestlive *glive;
    gf_node *node;
    struct gestlive_node *node_data;

    rc = sk_core_generic_pop(core, &ud);
    SK_ERROR_CHECK(rc);
    glive = ud;

    node_data = &glive->node[glive->write];
    node_data->init = 1;

    patch = sk_core_patch(core);
    rc = gf_patch_new_node(patch, &node);
    SK_GF_ERROR_CHECK(rc);

    gf_node_set_compute(node, swapper_compute);
    gf_node_set_data(node, node_data);

    return 0;
}
#line 301 "lib/gestvm/gestlive.org"
#line 322 "lib/gestvm/gestlive.org"
static void delgestlive(void *ptr)
{
    struct gestlive *glive;

    glive = ptr;

    gestlive_del(glive);
    ptr = glive = NULL;
}

static lil_value_t l_glnew(lil_t lil,
                           size_t argc,
                           lil_value_t *argv)
{
    int rc;
    const char *key;
    struct gestlive *glive;
    sk_core *core;

    SKLIL_ARITY_CHECK(lil, "glnew", argc, 1);

    core = lil_get_data(lil);

    glive = gestlive_new();

    key = lil_to_string(argv[0]);

    rc = sk_core_append(core, key, strlen(key),
                        glive, delgestlive);

    SKLIL_ERROR_CHECK(lil, rc, "glnew didn't work out.");

    return NULL;
}
#line 374 "lib/gestvm/gestlive.org"
static lil_value_t l_glget(lil_t lil,
                           size_t argc,
                           lil_value_t *argv)
{
    int rc;
    struct gestlive *glive;
    sk_core *core;
    void *ud;
    gestvm_uxn *gu;

    SKLIL_ARITY_CHECK(lil, "glget", argc, 1);

    core = lil_get_data(lil);

    rc = sk_core_generic_pop(core, &ud);
    glive = ud;

    gu = gestlive_get(glive);

    rc = gu == NULL;
    SKLIL_ERROR_CHECK(lil, rc, "glget: waiting for hotswap.");

    rc = sk_core_generic_push(core, gu);
    SKLIL_ERROR_CHECK(lil, rc, "glget didn't work out.");

    return NULL;
}
#line 413 "lib/gestvm/gestlive.org"
static lil_value_t l_gldone(lil_t lil,
                            size_t argc,
                            lil_value_t *argv)
{
    struct gestlive *glive;
    sk_core *core;
    void *ud;

    SKLIL_ARITY_CHECK(lil, "gldone", argc, 1);

    core = lil_get_data(lil);

    sk_core_generic_pop(core, &ud);
    glive = ud;

    gestlive_swap(glive);

    return NULL;
}
#line 444 "lib/gestvm/gestlive.org"
static lil_value_t l_glreset(lil_t lil,
                             size_t argc,
                             lil_value_t *argv)
{
    struct gestlive *glive;
    sk_core *core;
    void *ud;

    SKLIL_ARITY_CHECK(lil, "gldone", argc, 1);

    core = lil_get_data(lil);

    sk_core_generic_pop(core, &ud);
    glive = ud;

    gestlive_unlock(glive);

    return NULL;
}
#line 476 "lib/gestvm/gestlive.org"
static lil_value_t l_glswapper(lil_t lil,
                               size_t argc,
                               lil_value_t *argv)
{
    int rc;
    sk_core *core;

    SKLIL_ARITY_CHECK(lil, "glswapper", argc, 1);

    core = lil_get_data(lil);

    rc = sk_node_gestlive_swapper(core);

    SKLIL_ERROR_CHECK(lil, rc, "glswapper didn't work out.");

    return NULL;
}
#line 307 "lib/gestvm/gestlive.org"
void load_gestlive(lil_t lil)
{
#line 317 "lib/gestvm/gestlive.org"
lil_register(lil, "glnew", l_glnew);
#line 369 "lib/gestvm/gestlive.org"
lil_register(lil, "glget", l_glget);
#line 408 "lib/gestvm/gestlive.org"
lil_register(lil, "gldone", l_gldone);
#line 439 "lib/gestvm/gestlive.org"
lil_register(lil, "glreset", l_glreset);
#line 471 "lib/gestvm/gestlive.org"
lil_register(lil, "glswapper", l_glswapper);
#line 310 "lib/gestvm/gestlive.org"
}
#line 12 "lib/gestvm/gestlive.org"
