#line 32 "gen.org"
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "gen.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#line 57 "gen.org"
void sk_gen_sine(SKFLT *tab, unsigned long sz)
{
    int i;
    SKFLT step;

    if (sz <= 0) return;

    step = 2 * M_PI / sz;

    for (i = 0; i < sz; i++) {
        tab[i] = sin(i * step);
    }
}
#line 84 "gen.org"
void sk_gen_saw(SKFLT *tab, unsigned long sz)
{
    int i;
    SKFLT step;

    if (sz <= 0) return;

    step = 1.0 / sz;

    for (i = 0; i < sz; i++) {
        tab[i] = 2.0*(i*step) - 1.0;
    }
}
#line 112 "gen.org"
static char * tokenize(char **next, int *size)
{
    char *token;
    char *str;
    char *peak;

    if (*size <= 0) return NULL;
    token = *next;
    str = *next;

    peak = str + 1;

    while ((*size)--) {
        if (*str == ' ') {
            *str = 0;
            if (*peak != ' ') break;
        }
        str = str + 1;
        peak = str + 1;
    }
    *next = peak;
    return token;
}

void sk_gen_vals(SKFLT **ptab, int *psz, const char *string)
{
    int size;
    char *str;
    char *out;
    char *ptr;
    int j;
    int sz;
    SKFLT *tab;

    size = strlen(string);
    str = malloc(sizeof(char) * size + 1);
    strcpy(str, string);
    ptr = str;
    j = 0;

    tab = *ptab;
    sz = *psz;

    while (size > 0) {
        out = tokenize(&str, &size);
        if (sz < j + 1) {
            tab = realloc(tab, sizeof(SKFLT) * (sz + 2));
            /* zero out new tables */
            tab[sz] = 0;
            tab[sz + 1] = 0;
            sz++;
        }
        tab[j] = atof(out);
        j++;
    }

    *ptab = tab;
    *psz = sz;
    free(ptr);
}
#line 184 "gen.org"
void sk_gen_sinesum(SKFLT *tab,
                    int sz,
                    const char *argstring,
                    int normalize)
{
    SKFLT *args;
    int argsz;
    int phs;
    SKFLT amp;
    int flen;
    SKFLT tpdlen;
    int i, n;
    SKFLT ampsum;

    args = malloc(sizeof(SKFLT));
    args[0] = 0;
    argsz = 1;

    ampsum = 0;

    sk_gen_vals(&args, &argsz, argstring);
    flen = sz;
    tpdlen = 2.0 * M_PI / (SKFLT) flen;

    for (i = argsz; i > 0; i--) {
        amp = args[i - 1];
        if (amp > 0) {
            ampsum += amp;
            for (phs = 0, n = 0; n < sz; n++) {
                tab[n] += sin(phs * tpdlen) * amp;
                phs += i;
                phs %= flen;
            }
        }
    }

    if (normalize) {
        SKFLT norm;
        norm = 1.0 / ampsum;
        for (n = 0; n < sz; n++) {
            tab[n] *= norm;
        }
    }

    free(args);
}
#line 239 "gen.org"
int sk_gen_line(SKFLT *tab, int sz, const char *argstring)
{
    uint16_t i, n, seglen;
    SKFLT incr, amp = 0;
    SKFLT x1, x2, y1, y2;
    SKFLT *args;
    int argsz;

    args = malloc(sizeof(SKFLT));
    args[0] = 0;
    argsz = 1;

    sk_gen_vals(&args, &argsz, argstring);

    if ((argsz % 2) == 1 || argsz == 1) {
        fprintf(stderr, "Error: not enough arguments for gen_line.\n");
        free(args);
        return 1;
    } else if (argsz == 2) {
        for (i = 0; i < sz; i++) {
            tab[i] = args[1];
        }
        free(args);
        return 0;
    }

    x1 = args[0];
    y1 = args[1];
    n = 0;
    for (i = 2; i < argsz; i += 2) {
        x2 = args[i];
        y2 = args[i + 1];

        if (x2 < x1) {
            fprintf(stderr, "Error: x coordiates must be sequential!\n");
            break;
        }

        seglen = (x2 - x1);
        incr = (SKFLT)(y2 - y1) / (seglen - 1);
        amp = y1;

        while (seglen != 0) {
            if (n < sz) {
                tab[n] = amp;
                amp += incr;
                seglen--;
                n++;
            } else {
                break;
            }
        }
        y1 = y2;
        x1 = x2;
    }

    free(args);
    return 0;
}
#line 32 "gen.org"
