#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void trim(uint32_t *linebuf)
{
    if ((linebuf[63] & 0xFF) == ' ') {
        int i;
        int end;
        char prev;

        prev = 0;
        end = 0;

        for (i = 63; i >= 0; i--) {
            char c;
            c = linebuf[i] & 0xFF;
            if (c != ' ') {
                end = i + 1;
                break;
            }
        }

        for (i = end; i < 64; i++) {
            linebuf[i] = 0;
        }
    }
}

int main(int argc, char *argv[])
{
    FILE *fp;
    int pos;
    int i;
    int line;
    uint32_t linebuf[64];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s blocknum\n", argv[0]);
        return 1;
    }

    fp = fopen("ilo.blocks", "r");

    if (fp == NULL) {
        fprintf(stderr, "Could not open blocks\n");
        return 1;
    }

    pos = atoi(argv[1]);

    fseek(fp, pos*4096, SEEK_SET);

    for (line = 0; line < 16; line++) {
        fread(&linebuf, 1, sizeof(uint32_t)*64, fp);
        trim(linebuf);
        for (i = 0; i < 64; i++) {
            uint32_t word;

            word = linebuf[i];
            if (word != 0) {
                fputc(word & 0xFF, stdout);
            }
        }
        fputc('\n', stdout);
    }

    fclose(fp);
    return 0;
} 
