#ifndef GESTVM_MEMOPS_H
#define GESTVM_MEMOPS_H

typedef struct {
    unsigned char *buf;
    size_t size;
} gestvm_membuf;

int gestvm_load_mem(gestvm_uxn *gu, gestvm_membuf *rom);
unsigned int gestvm_lookup_mem(gestvm_membuf *rom, const char *sym);
#endif
