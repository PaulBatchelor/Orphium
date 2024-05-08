#line 13 "lib/gestvm/memops.org"
#include <stdlib.h>
#include <string.h>
#include "sndkit/graforge/graforge.h"
#include "sndkit/core.h"
#include "sndkit/lil/lil.h"
#include "sndkit/nodes/sklil.h"
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "uxn/uxn.h"

/* TODO: don't use GESTVM_PRIV */
#define GESTVM_PRIV
#include "gestvm.h"

#line 65 "lib/gestvm/memops.org"
typedef struct {
    unsigned char *buf;
    size_t size;
} gestvm_membuf;
#line 13 "lib/gestvm/memops.org"
#line 37 "lib/gestvm/memops.org"
void gestvm_memops_lua(lua_State *L);
void gestvm_memops_lil(lil_t lil);
#line 13 "lib/gestvm/memops.org"
#line 82 "lib/gestvm/memops.org"
static lil_value_t l_gmemnew(lil_t lil,
                             size_t argc,
                             lil_value_t *argv);
#line 140 "lib/gestvm/memops.org"
static unsigned int lookup(gestvm_membuf *rom, const char *sym);
#line 223 "lib/gestvm/memops.org"
static lil_value_t l_gmemsym(lil_t lil,
                             size_t argc,
                             lil_value_t *argv);
#line 274 "lib/gestvm/memops.org"
static int gestvm_compile(lua_State *L);
#line 348 "lib/gestvm/memops.org"
static lil_value_t l_gmemcpy(lil_t lil,
                             size_t argc,
                             lil_value_t *argv);
#line 387 "lib/gestvm/memops.org"
static int load(gestvm_uxn *gu, gestvm_membuf *rom);
#line 435 "lib/gestvm/memops.org"
static int gestvm_last_values(lua_State *L);
#line 458 "lib/gestvm/memops.org"
static int last_conductor(lua_State *L);
#line 13 "lib/gestvm/memops.org"
#line 43 "lib/gestvm/memops.org"
void gestvm_memops_lua(lua_State *L)
{
    lua_register(L, "gestvm_compile", gestvm_compile);
    lua_register(L, "gestvm_last_values", gestvm_last_values);
    lua_register(L, "gestvm_last_conductor", last_conductor);
}
#line 53 "lib/gestvm/memops.org"
void gestvm_memops_lil(lil_t lil)
{
#line 77 "lib/gestvm/memops.org"
lil_register(lil, "gmemnew", l_gmemnew);
#line 131 "lib/gestvm/memops.org"
lil_register(lil, "gmemsym", l_gmemsym);
#line 343 "lib/gestvm/memops.org"
lil_register(lil, "gmemcpy", l_gmemcpy);
#line 56 "lib/gestvm/memops.org"
}
#line 89 "lib/gestvm/memops.org"
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
#line 145 "lib/gestvm/memops.org"
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
#line 230 "lib/gestvm/memops.org"
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
#line 279 "lib/gestvm/memops.org"
int
uxnasm_compile(const char *input,
               size_t ilen,
               int symtab,
               unsigned char **output,
               size_t *olen);

static int gestvm_compile(lua_State *L)
{
    lil_t lil;
    const char *membuf_name;
    const char *tal;
    gestvm_membuf *mem;
    void *ptr;
    sk_core *core;
    int rc;

    lua_getglobal(L, "__lil");
    lil = lua_touserdata(L, -1);

    core = lil_get_data(lil);

    membuf_name = lua_tostring(L, 1);
    tal = lua_tostring(L, 2);

    mem = NULL;
    ptr = NULL;
    rc = sk_core_lookup(core,
                        membuf_name,
                        strlen(membuf_name),
                        &ptr);

    mem = ptr;
    if (mem == NULL || rc ) {
        luaL_error(L, "Could not find %s\n", membuf_name);
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
        luaL_error(L, "uxnasm error");
    }

    return 0;
}
#line 355 "lib/gestvm/memops.org"
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
#line 392 "lib/gestvm/memops.org"
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
#line 440 "lib/gestvm/memops.org"
static int gestvm_last_values(lua_State *L)
{
    gestvm *gvm;
    SKFLT x, y, a;
    gvm = lua_touserdata(L, 1);
    x = y = a = 0.0;
    gestvm_get_last_values(gvm, &x, &y, &a);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, a);
    return 3;
}
#line 463 "lib/gestvm/memops.org"
static int last_conductor(lua_State *L)
{
    gestvm *gvm;
    SKFLT cnd;
    gvm = lua_touserdata(L, 1);
    cnd = gestvm_last_conductor(gvm);
    lua_pushnumber(L, cnd);
    return 1;
}
#line 13 "lib/gestvm/memops.org"
