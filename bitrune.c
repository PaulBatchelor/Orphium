#include <stdio.h>
#include "bitrune.h"

void bitrune_clear(bitrune_page *page)
{
    int i;
    for (i = 0; i < 16; i++) {
        page->rows[i] = 0;
    }
}

void bitrune_print(bitrune_page *page)
{
    int x, y;
    for (y = 0; y < 16; y++) {
        unsigned short row;
        char rowstr[18];
        row = page->rows[y];
        
        for (x = 0; x < 16; x++) {
            int s;
            s = (row & (1<<x)) >> x;

            if (s) {
                rowstr[x] = '1';
            } else {
                rowstr[x] = '0';
            }
        }

        rowstr[16] = '\n';
        rowstr[17] = 0;
        printf("%s", rowstr);
    }
}

void bitrune_set(bitrune_page *page, int x, int y, int s)
{
    unsigned short row;

    if (x < 0 || x >= 16) return;
    if (y < 0 || y >= 16) return;

    row = page->rows[y];

    if (s) {
        row |= (1 << x);
    } else {
        row &= ~(1 << x);
    }

    page->rows[y] = row;
}

int bitrune_get(bitrune_page *page, int x, int y)
{
    if (x < 0 || x >= 16) return 0;
    if (y < 0 || y >= 16) return 0;
    return (page->rows[y] & (1 << x)) >> x;
}

static void clear_and_copy(bitrune_page *pg,
                           bitrune_page *rn,
                           int x, int y)
{
    bitrune_set(pg, x, y, 0);
    bitrune_set(rn, x, y, 1);
}

static void traverse(bitrune_page *pg,
                     bitrune_page *rn,
                     int x, int y)
{
    /* North */
    if (bitrune_get(pg, x, y - 1)) {
        clear_and_copy(pg, rn, x, y - 1);
        traverse(pg, rn, x, y - 1);
    }

    /* East */
    if (bitrune_get(pg, x + 1, y)) {
        clear_and_copy(pg, rn, x + 1, y);
        traverse(pg, rn, x + 1, y);
    }

    /* South */

    if (bitrune_get(pg, x, y + 1)) {
        clear_and_copy(pg, rn, x, y + 1);
        traverse(pg, rn, x, y + 1);
    }
    
    /* West */

    if (bitrune_get(pg, x - 1, y)) {
        clear_and_copy(pg, rn, x - 1, y);
        traverse(pg, rn, x - 1, y);
    }
}

static void find_topleft(bitrune_page *page,
                         int *topleft_x,
                         int *topleft_y)
{
    int x, y;

    if (topleft_x == NULL || topleft_y == NULL) return;

    *topleft_x = -1;
    *topleft_y = -1;

    /* sweep and find top-left most tile */
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            if (bitrune_get(page, x, y)) {
                *topleft_x = x;
                *topleft_y = y;
                break;
            }
        }

        if (*topleft_x >= 0) break;
    }
}

int bitrune_extract(bitrune_page *page, bitrune_page *rune)
{
    int topleft_x, topleft_y;

    topleft_x = topleft_y = -1;
    /* no tiles found, break */
    find_topleft(page, &topleft_x, &topleft_y);
    if (topleft_x < 0) return 1;

    /* clear and copy topleftmost tile, scan NESW for tiles */
    clear_and_copy(page, rune, topleft_x, topleft_y);
    traverse(page, rune, topleft_x, topleft_y);

    return 0;
}

void bitrune_bounds(bitrune_page *rune,
    int *x, int *y,
    int *w, int *h)
{

}
