#ifndef ORPH_BUFFER_H
#define ORPH_BUFFER_H

#define ORPH_BUFSIZE 1024

typedef struct {
    unsigned char buf[ORPH_BUFSIZE];
    int pos;
} orph_buffer;

void orph_buffer_init(orph_buffer *ob);
void orph_buffer_put(orph_buffer *ob, unsigned char c);
unsigned char* orph_buffer_get(orph_buffer *ob);
int orph_buffer_size(orph_buffer *ob);
void orph_buffer_reinit(orph_buffer *ob);

#endif
