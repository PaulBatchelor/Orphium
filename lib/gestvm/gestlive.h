#line 25 "lib/gestvm/gestlive.org"
#ifndef GESTLIVE_H
#define GESTLIVE_H
#line 59 "lib/gestvm/gestlive.org"
#line 59 "lib/gestvm/gestlive.org"
struct gestlive_node {
    int init;
    struct gestlive *glive;
};
#line 71 "lib/gestvm/gestlive.org"
struct gestlive {
    gestvm_uxn *uxn[2];
    int read;
    int write;
    int please_swap;
    struct gestlive_node node[2];
};
#line 25 "lib/gestvm/gestlive.org"
#line 82 "lib/gestvm/gestlive.org"
struct gestlive * gestlive_new(void);
void gestlive_del(struct gestlive *glive);
#line 133 "lib/gestvm/gestlive.org"
gestvm_uxn * gestlive_get(struct gestlive *glive);
#line 152 "lib/gestvm/gestlive.org"
void gestlive_unlock(struct gestlive *glive);
#line 170 "lib/gestvm/gestlive.org"
void gestlive_swap(struct gestlive *glive);
#line 197 "lib/gestvm/gestlive.org"
void gestlive_update(struct gestlive *glive);
#line 240 "lib/gestvm/gestlive.org"
int sk_node_gestlive_swapper(sk_core *core);
#line 301 "lib/gestvm/gestlive.org"
void load_gestlive(lil_t lil);
#line 29 "lib/gestvm/gestlive.org"
#endif
