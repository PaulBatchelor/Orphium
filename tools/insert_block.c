#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    char *buf;
    size_t bufsize;
    const char *txtfile;
    uint32_t *block;
    int i;
    int lp, bp;
    int blkpos;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s blocknum file.txt\n", argv[0]);
        return 1;
    }

    txtfile = argv[2];
    fp = fopen(txtfile, "a+");
    blkpos = atoi(argv[1]);

    printf("writing to block position %d\n", blkpos);
     
    if (fp == NULL) {
        fprintf(stderr, "Could not open '%s'\n", txtfile);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    bufsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buf = malloc(bufsize + 1);
    fread(buf, 1, bufsize, fp);
    buf[bufsize] = 0;
    fclose(fp);

    fp = fopen("ilo.blocks", "r+");

    if (fp == NULL) {
        fprintf(stderr, "Could not open blocks\n");
        return 1;
    }

    block = malloc(1024 * sizeof(uint32_t));

    for (i = 0; i < 1024; i++) block[i] = ' ';

    lp = 0;

    bp = 0;
    for (i = 0; i < bufsize; i++) {
        char c;
        if (lp >= 64) break;
        c = buf[i];

        if (c == '\n') {
            lp++;
            bp = 64 * lp;
            continue;
        }
        block[bp] = c;        
        bp++;
    } 

    fseek(fp, blkpos * 4096, SEEK_SET);
    fwrite(block, 1, 4096, fp);

    fclose(fp);
    free(block);
    free(buf);
    return 0;
}
