#include "buffer.h"

void orph_buffer_init(orph_buffer *ob)
{
    int i;

    for (i = 0; i < ORPH_BUFSIZE; i++) {
        ob->buf[i] = 0;
    }

    orph_buffer_reinit(ob);
}

void orph_buffer_reinit(orph_buffer *ob)
{
    ob->pos = 0;
}

void orph_buffer_put(orph_buffer *ob, unsigned char c)
{
    if (ob->pos >= ORPH_BUFSIZE) return;

    ob->buf[ob->pos] = c;
    ob->pos++;
}

unsigned char* orph_buffer_get(orph_buffer *ob)
{
    return ob->buf;
}

int orph_buffer_size(orph_buffer *ob)
{
    return ob->pos;
}
