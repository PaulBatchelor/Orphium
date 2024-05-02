#ifndef XMT_OBJ_H
#define XMT_OBJ_H
typedef struct {
    int type;
    void *data;
} orph_obj;

typedef struct {
    orph_obj obj;
    int val;
} orph_obj_int;

typedef struct {
    orph_obj obj;
    const char *val;
} orph_obj_str;

typedef struct {
    orph_obj obj;
    orph_obj **val;
    int length;
} orph_obj_array;

typedef struct {
    orph_obj *key;
    orph_obj *val;
} orph_obj_pair;

typedef struct {
    orph_obj obj;
    orph_obj_pair *val;
    int length;
} orph_obj_map;

orph_obj * orph_obj_mkint(int val);
orph_obj * orph_obj_mkstr(const char *str, size_t sz);
orph_obj * orph_obj_mkarray(int length);
orph_obj ** orph_obj_array_values(orph_obj *obj);
orph_obj * orph_obj_mkmap(int length);
void orph_obj_map_insert(orph_obj *map,
                        const char *key,
                        size_t sz,
                        orph_obj *val,
                        int pos);
void orph_obj_map_insert_v2(orph_obj *map,
                           orph_obj *key,
                           orph_obj *val,
                           int pos);
void xmt_print(orph_obj *obj);
void orph_obj_del(orph_obj **obj);
orph_obj * orph_obj_map_lookup(orph_obj *map, const char *key);

int orph_obj_parse(unsigned char *bytes, size_t sz, orph_obj **obj);

int orph_obj_isint(orph_obj *obj);
int orph_obj_isstr(orph_obj *obj);
int orph_obj_isarray(orph_obj *obj);
int orph_obj_ismap(orph_obj *obj);
#endif
