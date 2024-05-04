#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int memory[1024];

void read_blk(int blk) {
  int f;
  f = open("ilo.blocks", O_RDONLY, 0666);
  lseek(f, 4096 * blk, SEEK_SET);
  read(f, memory, 4096);
  close(f);
}

void write_blk(int blk) {
  int f, i;
  char target[4096];
  FILE *fp;
  char c[2];

  sprintf(target, "blocks/%d", blk);
#if 0
  f = open(target, O_WRONLY | O_CREAT, 0666);
  write(f, memory, 4096);
  close(f);
#endif

  fp = fopen(target, "wb");
  fwrite(memory, 1, 4096, fp);
  fclose(fp);

  sprintf(target, "blocks/text/%d.txt", blk);
  f = open(target, O_WRONLY | O_CREAT, 0666);
  for (i = 0; i < 1024; i++) {
    if (i % 64 == 0 && i != 0) {
      c[0] = '\n';
      write(f, &c, 1);
    }
    c[0] = (char)memory[i];
    write(f, &c, 1);
  }
  close(f);
#if 0
  sprintf(target, "blocks/gemtext/%d.gmi", blk);
  f = open(target, O_WRONLY | O_CREAT, 0666);
  c[0] = '`';
  write(f, &c, 1);
  write(f, &c, 1);
  write(f, &c, 1);
  c[0] = '\n';
  write(f, &c, 1);
  for (i = 0; i < 1024; i++) {
    if (i % 64 == 0 && i != 0) {
      c[0] = '\n';
      write(f, &c, 1);
    }
    c[0] = (char)memory[i];
    write(f, &c, 1);
  }
  c[0] = '\n';
  write(f, &c, 1);
  c[0] = '`';
  write(f, &c, 1);
  write(f, &c, 1);
  write(f, &c, 1);
  c[0] = '\n';
  write(f, &c, 1);
  close(f);
#endif
}

int main() {
  int i;
  for (i = 0; i < 4096; i++) { read_blk(i); write_blk(i); }
  return 0;
}
