#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"
#include "sndkit/graforge/graforge.h"
#include "sndkit/core.h"
#include "sndkit/nodes/sknodes.h"
#include "obj.h"
#include "parse.h"

static void n_wavout(sk_core *core, orph_obj *data)
{
    orph_obj_str *str;
    const char *filename;

    if (!orph_obj_isstr(data)) {
        /* TODO: error handling if incorrect data found */
        printf("oops\n");
        return;
    }

    str = data->data;
    filename = str->val;

    printf("wavout: filename: %s\n", filename);
    sk_node_wavout(core, filename);
}

static void n_sine(sk_core *core, orph_obj *data)
{
    printf("sine\n");
    sk_node_sine(core);
}

static void computes(sk_core *core, orph_obj *data)
{
    float dur;
    const char *sdur;
    orph_obj_str *os;

    os = (orph_obj_str *)data->data;
    sdur = (const char *)os->val;
    dur = atof(sdur);
    printf("computes %g\n", dur);

    sk_core_computes(core, dur);
}

static int is_number(const char *str)
{
    /* simple check: see if first digit is in rage 0-9 */
    return str[0] >= '0' && str[0] <= '9';
}

static void mkparam(sk_core *core, const char *str)
{
    float param;

    param = atof(str);

    printf("param: %g\n", param);
    sk_core_constant(core, param);
}

static void word_lookup(sk_core *core, const char *node_name, orph_obj *node_data)
{
    /* TODO: replace with more robust dictionary */

    if (!strcmp(node_name, "sine")) {
        n_sine(core, node_data);
    } else if (!strcmp(node_name, "wavout")) {
        n_wavout(core, node_data);
    } else if (!strcmp(node_name, "computes")) {
        computes(core, node_data);
    } else if (!strcmp(node_name, "hello")) {
        printf("hello orphium!\n");
    } else {
        /* TODO: error handling */
    }
}

static void parse_word(sk_core *core, const char *word)
{
    printf("word: %s\n", word);

    if (is_number(word)) {
        mkparam(core, word);
        return;
    }

    word_lookup(core, word, NULL);
}

static void append_word(const char *word, orph_buffer *talbuf)
{
    int len;
    int i;
    len = strlen(word);

    for (i = 0; i < len; i++) {
        orph_buffer_put(talbuf, word[i]);
    }
    orph_buffer_put(talbuf, ' ');
}

int orph_parse_object(sk_core *core, orph_buffer *talbuf, orph_obj *obj)
{
    if (orph_obj_isstr(obj)) {
        const char *word;
        orph_obj_str *str;
        str = (orph_obj_str *)obj->data;
        word = str->val;
        parse_word(core, word);
    } else if (orph_obj_ismap(obj)) {
        orph_obj_map *m;
        int i;
        int is_node;
        int is_tal;
        orph_obj *node_data;
        const char *node_name;
        const char *tal_word;

        is_node = 0;
        is_tal = 0;
        m = (orph_obj_map *)obj->data;
        node_name = NULL;
        node_data = NULL;
        tal_word = NULL;

        for (i = 0; i < m->length; i++) {
            if (m->val[i].key != NULL) {
                orph_obj_str *key;
                key = m->val[i].key->data;
                if (!strcmp(key->val, "node")) {
                    orph_obj_str *name;
                    is_node = 1;
                    name = m->val[i].val->data;
                    node_name = name->val;
                } else if (!strcmp(key->val, "cmd")) {
                    /* 'cmd' is semantically different from 'node'
                     * this eventually might want to be split apart more.
                     */
                    orph_obj_str *name;
                    is_node = 1;
                    name = m->val[i].val->data;
                    node_name = name->val;
                } else if (!strcmp(key->val, "tal")) {
                    orph_obj_str *name;
                    is_tal = 1;
                    name = m->val[i].val->data;
                    tal_word = name->val;
                } else if (!strcmp(key->val, "data")) {
                    node_data = m->val[i].val;
                }
            }
        }

        if (is_node) {
            /* TODO: node_lookup vs word_lookup? */
            word_lookup(core, node_name, node_data);
        } else if (is_tal) {
            printf("Tal word: %s\n", tal_word);
            append_word(tal_word, talbuf);
        }
    }
    return 0;
}
