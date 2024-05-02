#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "obj.h"
#include "cmp/cmp.h"
#include "moncmp.h"

enum {
    TYPE_NONE,
    TYPE_INT,
    TYPE_STR,
    TYPE_ARRAY,
    TYPE_MAP
};

orph_obj * orph_obj_mkint(int val)
{
    orph_obj_int *obj;
    obj = malloc(sizeof(orph_obj_int));
    obj->obj.type = TYPE_INT;
    obj->val = val;
    obj->obj.data = obj;
    return &obj->obj;
}

orph_obj * orph_obj_mkstr(const char *str, size_t sz)
{
    orph_obj_str *obj;
    char *outstr;
    obj = malloc(sizeof(orph_obj_str) + sz + 1);
    obj->obj.type = TYPE_STR;
    /* string bits are allocated at the end of the struct */
    outstr = (char *)&obj[1];
    obj->obj.data = obj;
    memcpy(outstr, str, sz);
    outstr[sz] = '\0';
    obj->val = (const char *)outstr;
    return &obj->obj;
}

orph_obj * orph_obj_mkarray(int length)
{
    orph_obj_array *obj;
    int i;
    obj = malloc(sizeof(orph_obj_array) + length * sizeof(orph_obj*));
    obj->obj.type = TYPE_ARRAY;
    obj->obj.data = obj;
    obj->length = length;
    obj->val = (orph_obj **)&obj[1];
    for (i = 0; i < length; i++) {
        obj->val[i] = NULL;
    }
    return &obj->obj;
}

orph_obj ** orph_obj_array_values(orph_obj *obj)
{
    orph_obj_array *a;
    if (obj->type != TYPE_ARRAY) return NULL;
    a = obj->data;
    return a->val;
}

orph_obj * orph_obj_mkmap(int length)
{
    orph_obj_map *obj;
    int i;
    obj = malloc(sizeof(orph_obj_map) + length * sizeof(orph_obj_pair));
    obj->obj.type = TYPE_MAP;
    obj->obj.data = obj;
    obj->length = length;
    obj->val = (orph_obj_pair *)&obj[1];
    for (i = 0; i < length; i++) {
        obj->val[i].key = NULL;
        obj->val[i].val = NULL;
    }
    return &obj->obj;
}

void orph_obj_map_insert(orph_obj *map,
                        const char *key,
                        size_t sz,
                        orph_obj *val,
                        int pos)
{
    orph_obj_map *m;
    if (map->type != TYPE_MAP) return;
    m = map->data;

    if (m->val[pos].key != NULL) return;

    m->val[pos].key = orph_obj_mkstr(key, sz);
    m->val[pos].val = val;
}

void orph_obj_map_insert_v2(orph_obj *map,
                           orph_obj *key,
                           orph_obj *val,
                           int pos)
{
    orph_obj_map *m;
    if (map->type != TYPE_MAP) return;
    m = map->data;

    if (m->val[pos].key != NULL) return;

    m->val[pos].key = key;
    m->val[pos].val = val;
}


void xmt_print(orph_obj *obj)
{
    switch(obj->type) {
        case TYPE_INT: {
            orph_obj_int *i;
            i = obj->data;
            printf("%d\n", i->val);
        }
            break;
        case TYPE_STR: {
            orph_obj_str *s;
            s = obj->data;
            printf("%s\n", s->val);
        }
            break;
        case TYPE_ARRAY: {
            orph_obj_array *a;
            int i;
            a = obj->data;
            printf("[\n");
            for (i = 0; i < a->length; i++) {
                if (a->val[i] == NULL) {
                    printf("NULL\n");
                } else {
                    xmt_print(a->val[i]);
                }
            }
            printf("]\n");
        }
            break;
        case TYPE_MAP: {
            orph_obj_map *m;
            int i;
            m = obj->data;
            printf("{\n");
            for (i = 0; i < m->length; i++) {
                if (m->val[i].key != NULL) {
                    orph_obj_str *key;
                    key = m->val[i].key->data;
                    printf("%s:\n\t", key->val);
                    xmt_print(m->val[i].val);
                }
            }
            printf("}\n");
        }
            break;
        default:
            break;
    }
}

void orph_obj_del(orph_obj **obj)
{
    if ((*obj)->type == TYPE_ARRAY) {
        int i;
        orph_obj_array *a;
        a = (*obj)->data;
        for (i = 0; i < a->length; i++) {
            if (a->val[i] != NULL) {
                orph_obj_del(&a->val[i]);
            }
        }
    } else if ((*obj)->type == TYPE_MAP) {
        int i;
        orph_obj_map *m;
        m = (*obj)->data;
        for (i = 0; i < m->length; i++) {
            if (m->val[i].key != NULL) {
                orph_obj_del(&m->val[i].key);
                orph_obj_del(&m->val[i].val);
            }
        }
    }
    free(*obj);
}

orph_obj * orph_obj_map_lookup(orph_obj *obj, const char *key)
{
    orph_obj_map *map;
    int i;

    if (obj->type != TYPE_MAP) return NULL;

    map = obj->data; 

    for (i = 0; i < map->length; i++) {
        orph_obj *tmp;
        orph_obj_str *s;

        tmp = map->val[i].key;
        s = tmp->data;

        if (!strcmp(key, s->val)) {
            return map->val[i].val;
        }
    }

    return NULL;
}

static orph_obj * read_object_v2(cmp_ctx_t *cmp)
{
    cmp_object_t obj;
    orph_obj *o;

    o = NULL;

    if (!cmp_read_object(cmp, &obj)) {
        return NULL;
    }

    switch(obj.type) {
        case CMP_TYPE_FIXMAP:
        case CMP_TYPE_MAP16:
        case CMP_TYPE_MAP32: {
            size_t i;
            o = orph_obj_mkmap(obj.as.map_size);
            for (i = 0; i < obj.as.map_size; i++) {
                orph_obj *key, *val;
                key = read_object_v2(cmp);
                val = read_object_v2(cmp);
                orph_obj_map_insert_v2(o, key, val, i);
            }
            break;
        }
        case CMP_TYPE_FIXSTR:
        case CMP_TYPE_STR8:
        case CMP_TYPE_STR16:
        case CMP_TYPE_STR32: {
            char *sbuf;
            sbuf = malloc(obj.as.str_size + 1);
            memset(sbuf, 0, obj.as.str_size + 1);
            moncmp_read(cmp, sbuf, obj.as.str_size);
            o = orph_obj_mkstr(sbuf, obj.as.str_size);
            free(sbuf);
            break;
        }
        case CMP_TYPE_POSITIVE_FIXNUM:
        case CMP_TYPE_UINT8:
            o = orph_obj_mkint(obj.as.u8);
            break;
        case CMP_TYPE_SINT8:
            o = orph_obj_mkint(obj.as.s8);
            break;
        case CMP_TYPE_UINT16:
            o = orph_obj_mkint(obj.as.u16);
            break;
        case CMP_TYPE_UINT32:
            o = orph_obj_mkint(obj.as.u32);
            break;
        case CMP_TYPE_FIXARRAY:
        case CMP_TYPE_ARRAY16:
        case CMP_TYPE_ARRAY32: {
            size_t i;
            orph_obj **vals;
            o = orph_obj_mkarray(obj.as.array_size);
            vals = orph_obj_array_values(o);
            for (i = 0; i < obj.as.array_size; i++) {
                vals[i] = read_object_v2(cmp);
            }
       }
            break;
        default:
            printf("not sure: %d\n", obj.type);
            break;
    }
    return o;
}

int orph_obj_parse(unsigned char *bytes, size_t sz, orph_obj **obj)
{
    moncmp_d m;
    cmp_ctx_t cmp;
    orph_obj *o;

    moncmp_init_read(&m, &cmp, bytes, sz);

    o = read_object_v2(&cmp);

    *obj = o;
    return 0;
}

int orph_obj_isint(orph_obj *obj)
{
    return obj->type == TYPE_INT;
}

int orph_obj_isstr(orph_obj *obj)
{
    return obj->type == TYPE_STR;
}

int orph_obj_isarray(orph_obj *obj)
{
    return obj->type == TYPE_ARRAY;
}

int orph_obj_ismap(orph_obj *obj)
{
    return obj->type == TYPE_MAP;
}
