#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitrune.h"

typedef unsigned char byte;
char * Z85_encode (byte *data, size_t size);

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
                rowstr[x] = '#';
            } else {
                rowstr[x] = '.';
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
    int min_x, max_x;
    int min_y, max_y;

    int xp, yp;

    /* initialize variables */
    *x = *y = *w = *h = -1;
    min_x = min_y = 100;
    max_x = max_y = -1;

    for (yp = 0; yp < 16; yp++) {
        for (xp = 0; xp < 16; xp++) {
            if (bitrune_get(rune, xp, yp)) {
                if (xp < min_x) min_x = xp;
                if (yp < min_y) min_y = yp;
                if (xp > max_x) max_x = xp;
                if (yp > max_y) max_y = yp;
            }
        }
    }

    *x = min_x;
    *y = min_y;
    *w = (max_x - min_x) + 1; 
    *h = (max_y - min_y) + 1;
}

void bitrune_print_bounds(bitrune_page *rune,
                          int xoff, int yoff,
                          int w, int h)
{
    int x, y;

    for (y = 0; y < h; y++) {
        char rowstr[18];
        for (x = 0; x < w; x++) {
            if (bitrune_get(rune, x + xoff, y + yoff)) {
                rowstr[x] = '#';
            } else {
                rowstr[x] = '.';
            }
        }
        rowstr[w] = '\n';
        rowstr[w+1] = 0;
        printf("%s", rowstr);
    }
}

char* bitrune_autoname(bitrune_page *rune,
                       int xoff, int yoff,
                       int w, int h)
{
    byte *bits;
    char *z85str;
    int nbytes;
    int nbits;
    int x, y;
    int bitpos;
    char *varname;

    /* determine number of bytes required for bitstream */

    nbits = w * h;
    nbytes = 0;

    /* there's probably a more efficient way to do this */
    while ((nbytes * 8) < nbits) nbytes++;

    /* z85 only takes bytes bounded to 4 bytes */
    while ((nbytes % 4) != 0) nbytes++;

    /* allocate and zero out */

    bits = malloc(nbytes);

    for (x = 0; x < nbytes; x++) bits[x] = 0;

    /* iterate through bitmap, copy bits to bitstream */

    bitpos = 0;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            int which_byte;
            int which_bit;
            int s, b;

            which_bit = bitpos % 8;
            which_byte = bitpos / 8;
            s = bitrune_get(rune, x + xoff, y + yoff);
            b = bits[which_byte];

            if (s) {
                b |= 1 << which_bit; 
            } else {
                b &= ~(1 << which_bit);
            }

            bits[which_byte] = b;

            bitpos++;
        }
    }

    z85str = Z85_encode(bits, nbytes);
    varname = malloc(2 + strlen(z85str));
    free(bits);
    varname[0] = 'B';
    varname[1] = 'R';
    /* really quick and dirty copy */
    for (x = 0; x < strlen(z85str); x++) {
        varname[x + 2] = z85str[x];
    }
    free(z85str);
    /* encode to z85 */
    return varname;
}
