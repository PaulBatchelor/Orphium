#include <stdlib.h>
#include <string.h>
#include "sndkit/graforge/graforge.h"
#include "sndkit/core.h"
#include "sndkit/lil/lil.h"
#include "sndkit/nodes/sklil.h"
#include "uxn/uxn.h"

/* TODO: don't use GESTVM_PRIV */
#define GESTVM_PRIV
#include "gestvm.h"

#include "memops.h"

void gestvm_memops_lil(lil_t lil);
static lil_value_t l_gmemnew(lil_t lil,
        size_t argc,
        lil_value_t *argv);
static unsigned int lookup(gestvm_membuf *rom, const char *sym);
static lil_value_t l_gmemsym(lil_t lil,
        size_t argc,
        lil_value_t *argv);
static lil_value_t l_gmemcpy(lil_t lil,
        size_t argc,
        lil_value_t *argv);
static int load(gestvm_uxn *gu, gestvm_membuf *rom);
void gestvm_memops_lil(lil_t lil)
{
    lil_register(lil, "gmemnew", l_gmemnew);
    lil_register(lil, "gmemsym", l_gmemsym);
    lil_register(lil, "gmemcpy", l_gmemcpy);
}
static void membuf_clean(void *ptr)
{
    gestvm_membuf *mem;
    mem = ptr;
    free(mem->buf);
    free(mem);
}

static lil_value_t l_gmemnew(lil_t lil,
        size_t argc,
        lil_value_t *argv)
{
    int rc;
    const char *key;
    gestvm_membuf *mem;
    sk_core *core;

    SKLIL_ARITY_CHECK(lil, "gmemnew", argc, 1);

    core = lil_get_data(lil);

    key = lil_to_string(argv[0]);
    mem = malloc(sizeof(gestvm_membuf));
    mem->buf = NULL;
    mem->size = 0;

    rc = sk_core_append(core,
            key,
            strlen(key),
            mem,
            membuf_clean);

    SKLIL_ERROR_CHECK(lil, rc, "gmemnew didn't work out.");

    return NULL;
}
static void not_fread(void *ptr,
        int sz,
        gestvm_membuf *mem,
        int *pos)
{
    int i;
    unsigned char *m;

    m = ptr;
    for (i = 0; i < sz; i++) {
        if (*pos >= mem->size) return;
        m[i] = mem->buf[*pos];
        *pos = (*pos) + 1;
    }
}

static unsigned int lookup(gestvm_membuf *rom, const char *sym)
{
    unsigned char symlen;
    unsigned short sz;
    unsigned char buf[64];
    unsigned int addr;
    int pos;

    symlen = strlen(sym);
    addr = 0;

    memset(buf, 0, 64);

    pos = 0;
    not_fread(buf, 3, rom, &pos);

    if (buf[0] != 'S' || buf[1] != 'Y' || buf[2] != 'M') {
        return 0;
    }

    sz = 0;
    not_fread(buf, 2, rom, &pos);

    sz = buf[0] + (buf[1] << 8);

    while (sz) {
        unsigned char len;
        len = 0;
        not_fread(&len, 1, rom, &pos);

        if (len == symlen) {
            int i;
            int match;
            not_fread(buf, len, rom, &pos);
            match = 1;
            for (i = 0; i < len; i++) {
                if (buf[i] != sym[i]) {
                    match = 0;
                    break;
                }
            }

            if (match) {
                not_fread(buf, 2, rom, &pos);
                addr = buf[0] + (buf[1] << 8);
                break;
            } else {
                pos += 2;
            }
        } else {
            pos += len + 2;
        }

        sz -= (len + 2 + 1);
    }

    return addr;
}
static lil_value_t l_gmemsym(lil_t lil,
        size_t argc,
        lil_value_t *argv)
{
    const char *sym;
    unsigned int addr;
    int rc;
    gestvm_membuf *mem;
    void *ud;
    sk_core *core;

    SKLIL_ARITY_CHECK(lil, "gestvmsym", argc, 2);

    core = lil_get_data(lil);
    rc = sk_core_generic_pop(core, &ud);

    SKLIL_ERROR_CHECK(lil, rc, "could not get memory");

    mem = ud;

    sym = lil_to_string(argv[1]);

    addr = lookup(mem, sym);

    if (addr == 0) {
        char errmsg[128];
        sprintf(errmsg, "Problem finding symbol: %s", sym);
        SKLIL_ERROR_CHECK(lil, 1, errmsg);
    }

    return lil_alloc_integer(addr);
}
int
uxnasm_compile(const char *input,
        size_t ilen,
        int symtab,
        unsigned char **output,
        size_t *olen);

int gestvm_compile(sk_core *core, const char *membuf_name, const char *tal)
{
    gestvm_membuf *mem;
    void *ptr;
    int rc;

    mem = NULL;
    ptr = NULL;
    rc = sk_core_lookup(core,
            membuf_name,
            strlen(membuf_name),
            &ptr);

    mem = ptr;
    if (mem == NULL || rc ) {
        /* luaL_error(L, "Could not find %s\n", membuf_name); */
        return 0;
    }

    if (mem->buf != NULL) {
        free(mem->buf);
        mem->buf = NULL;
    }

    rc = uxnasm_compile(tal,
            strlen(tal),
            1,
            &mem->buf,
            &mem->size);

    if (rc) {
        /* luaL_error(L, "uxnasm error"); */
    }

    return 0;
}

static lil_value_t l_gmemcpy(lil_t lil,
        size_t argc,
        lil_value_t *argv)
{
    int rc;
    gestvm_membuf *mem;
    gestvm_uxn *gu;
    void *ud;
    sk_core *core;

    SKLIL_ARITY_CHECK(lil, "gmemload", argc, 2);

    core = lil_get_data(lil);

    rc = sk_core_generic_pop(core, &ud);
    SKLIL_ERROR_CHECK(lil, rc, "could not get memory");
    gu = ud;

    rc = sk_core_generic_pop(core, &ud);
    SKLIL_ERROR_CHECK(lil, rc, "could not get memory");
    mem = ud;

    rc = load(gu, mem);

    SKLIL_ERROR_CHECK(lil, rc, "could not copy.");

    return NULL;
}

static int load(gestvm_uxn *gu, gestvm_membuf *rom)
{
    Uxn *u;
    char sym[3];
    int pos;

    sym[0] = sym[1] = sym[2] = 0;

    pos = 0;
    not_fread(sym, 3, rom, &pos);

    if (sym[0] == 'S' && sym[1] == 'Y' && sym[2] == 'M') {
        unsigned char b[2];
        unsigned short sz;
        b[0] = b[1] = 0;
        not_fread(b, 2, rom, &pos);
        sz = b[0] | (b[1] << 8);
        pos += sz;

    } else pos = 0;

    /* TODO: create getters for gestvm */

    u = &gu->u;

    not_fread(u->ram.dat + PAGE_PROGRAM,
            sizeof(u->ram.dat) - PAGE_PROGRAM,
            rom,
            &pos);
    return 0;
}


