#include <stdio.h>
#include "bitrune.h"

int main(int argc, char *argv[])
{
    bitrune_page pg;
    bitrune_page rune;
    int runepos;

    bitrune_clear(&pg);

    /* dot */
    bitrune_set(&pg, 0, 0, 1);

    /* cross */
    bitrune_set(&pg, 1, 1, 1);
    bitrune_set(&pg, 0, 2, 1);
    bitrune_set(&pg, 1, 2, 1);
    bitrune_set(&pg, 2, 2, 1);
    bitrune_set(&pg, 1, 3, 1);

    /* box */
    bitrune_set(&pg, 4, 1, 1);
    bitrune_set(&pg, 4, 2, 1);
    bitrune_set(&pg, 4, 3, 1);

    bitrune_set(&pg, 5, 1, 1);
    bitrune_set(&pg, 6, 1, 1);

    bitrune_set(&pg, 5, 3, 1);
    bitrune_set(&pg, 6, 3, 1);

    bitrune_set(&pg, 6, 2, 1);

    bitrune_print(&pg);

    bitrune_clear(&rune);
    runepos = 0;

    while (!bitrune_extract(&pg, &rune)) {
        int x, y, w, h;
        printf("rune: %d\n", runepos);
        bitrune_bounds(&rune, &x, &y, &w, &h);
        printf("bounds: %d %d %d %d\n", x, y, w, h);
        bitrune_print_bounds(&rune, x, y, w, h);
        bitrune_clear(&rune);
        runepos++;
    }
    return 0;
}
