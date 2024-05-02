#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "obj.h"

#include "sndkit/graforge/graforge.h"
#include "sndkit/core.h"
#include "sndkit/nodes/sknodes.h"
#include "parse.h"

void parse(unsigned char *bytes, int sz)
{
    orph_obj *obj;
    int rc;
    rc = orph_obj_parse(bytes, sz, &obj);

    if (rc) {
        fprintf(stderr, "oops");
    } else {
        xmt_print(obj);
    }

    orph_obj_del(&obj);
}

void add_string(orph_buffer *ob, const char *str)
{
    int i;
    int sz;

    sz = strlen(str);

    orph_buffer_put(ob, (0x5 << 5) | sz);

    for (i = 0; i < sz; i++) {
        orph_buffer_put(ob, str[i]);
    }
}

void add_keyval(orph_buffer *ob, const char *key, const char *val)
{
    int i;
    int keylen;
    int vallen;

    keylen = strlen(key);
    vallen = strlen(val);
    orph_buffer_put(ob, (0x5 << 5) | keylen);

    for (i = 0; i < keylen; i++) {
        orph_buffer_put(ob, key[i]);
    }

    orph_buffer_put(ob, (0x5 << 5) | strlen(val));
    for (i = 0; i < vallen; i++) {
        orph_buffer_put(ob, val[i]);
    }
}

int hello_buffer(void)
{
    orph_buffer *ob;
    int str_sz;
    int i;
    int rc;
    const char *str = "hello";
    rc = 0;

    ob = malloc(sizeof(orph_buffer));
    orph_buffer_init(ob);
    str_sz = strlen(str);

    if (str_sz > 31) {
        fprintf(stderr, "invalid string: greater than 31\n");
        goto clean;
        rc = 1;
    }

    /* msgpack fixedstr: 101XXXXX + data */
    orph_buffer_put(ob, (0x5 << 5) | str_sz);
    for (i = 0; i < str_sz; i++) {
        orph_buffer_put(ob, str[i]);
    }

    printf("buffer size is %d\n", orph_buffer_size(ob));
    parse(orph_buffer_get(ob), orph_buffer_size(ob));

    orph_buffer_reinit(ob);

    /* msgpack map: {node="wavout", data="test.wav"} */

    /* fixmap: 1000XXXX + key/val data */
    /* 2 items */
    orph_buffer_put(ob, (0x8 << 4) | 2);

    add_keyval(ob, "node", "wavout");
    add_keyval(ob, "data", "test.wav");

    parse(orph_buffer_get(ob), orph_buffer_size(ob));
    clean:
    free(ob);
    return rc;
}

void eval(sk_core *core, orph_buffer *ob)
{
    orph_obj *obj;
    unsigned char *bytes;
    int sz;
    int rc;

    bytes = orph_buffer_get(ob);
    sz = orph_buffer_size(ob);

    rc = orph_obj_parse(bytes, sz, &obj);

    if (rc) {
        fprintf(stderr, "oops");
    } else {
        orph_parse_object(core, obj);
    }

    orph_obj_del(&obj);
}

int msgpack_sine(void)
{
    orph_buffer *ob;
    sk_core *core;
    int rc;

    core = sk_core_new(44100);

    rc = 0;

    ob = malloc(sizeof(orph_buffer));
    orph_buffer_init(ob);

    add_string(ob, "440");
    eval(core, ob);

    orph_buffer_reinit(ob);
    add_string(ob, "0.5");
    eval(core, ob);

    orph_buffer_reinit(ob);
    /* sine */
    add_string(ob, "sine");
    eval(core, ob);

    orph_buffer_reinit(ob);

    /* msgpack map: {node="wavout", data="test.wav"} */

    /* fixmap: 1000XXXX + key/val data */
    /* 2 items */
    orph_buffer_put(ob, (0x8 << 4) | 2);

    add_keyval(ob, "node", "wavout");
    add_keyval(ob, "data", "test.wav");
    eval(core, ob);

    orph_buffer_reinit(ob);
    orph_buffer_put(ob, (0x8 << 4) | 2);
    add_keyval(ob, "cmd", "computes");
    add_keyval(ob, "data", "10.0");

    eval(core, ob);

    orph_buffer_reinit(ob);
    add_string(ob, "hello");
    eval(core, ob);

    free(ob);
    sk_core_del(core);
    return rc;
}

int main(int argc, char *argv[])
{
    /* hello_buffer(); */

    msgpack_sine();
    return 0;
}
