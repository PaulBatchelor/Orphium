#ifndef ORPH_BITRUNE_H
#define ORPH_BITRUNE_H

/* Page: 16x16 bitmap */

typedef struct {
    unsigned short rows[16];
} bitrune_page;

/* zero out and clear a page */
void bitrune_clear(bitrune_page *page);

/* print to stdout */
void bitrune_print(bitrune_page *page);

/* set a bitrune tile, given an X/Y location */
void bitrune_set(bitrune_page *page, int x, int y, int s);

/* get a bitrune tile, given an X/Y location */
int bitrune_get(bitrune_page *page, int x, int y);

/* extract the top-left most bitrune from page to rune */
/* returns non-zero if no bitrune found */
int bitrune_extract(bitrune_page *page, bitrune_page *rune);

/* determine the bounding box of a rune or page */
void bitrune_bounds(bitrune_page *rune,
    int *x, int *y,
    int *w, int *h);
#endif
