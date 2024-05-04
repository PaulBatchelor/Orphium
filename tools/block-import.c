#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

/* int memory[1024]; */

unsigned char memory[4096];
FILE *ilo_blocks = NULL;

void read_blk(int blk) {
#if 0
  int f;
  char target[4096];
  sprintf(target, "blocks/%d", blk);
  f = open(target, O_RDONLY, 0666);
  read(f, memory, 4096);
  close(f);
#endif
  FILE *fp;
  char target[4096];
  sprintf(target, "blocks/%d", blk);
  fp = fopen(target, "rb");
  fread(memory, 1, 4096, fp);
  fclose(fp);
}

void write_blk(int blk) {
#if 0
  int f;
  f = open("ilo.blocks", O_WRONLY, 0666);
  lseek(f, 4096 * blk, SEEK_SET);
  write(f, memory, 4096);
  close(f);
#endif

  /* FILE *fp; */
  /* size_t fpos; */
  /* fseek(ilo_blocks, 4096*blk, SEEK_SET); */
  fwrite(memory, 1, 4096, ilo_blocks);
  /* fclose(fp); */
}

int main() {
  int i;
  ilo_blocks = fopen("ilo.blocks", "wb");
  for (i = 0; i < 4096; i++) { read_blk(i); write_blk(i); }
  fclose(ilo_blocks);
  return 0;
}
