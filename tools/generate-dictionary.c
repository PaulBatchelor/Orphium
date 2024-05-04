#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

FILE *fi, *fo;
char buffer[4096];

void read_line(FILE *file, char *line_buffer) {
  int ch = getc(file);
  int count = 0;
  while ((ch != '\n') && (ch != EOF)) {
    line_buffer[count++] = ch;
    ch = getc(file);
  }
  line_buffer[count] = '\0';
}

int hash(char *s) {
  int c, h = 5381;
  while ((c = *s++)) {
    if (c == 9) return h;
    h = (h * 33) + c;
  }
  return h;
}

int count_tabs(char *s) {
  int count = 0;
  while (*s) { if (*s++ == '\t') count++; }
  return count;
}

char *name(char *s) {
  int count = 0;
  while (*s) {
    if (*s++ == '\t') count++;
    if (count == 1 && *s != 0 && *s != 9) fprintf(fo, "%c", *s);
  }
  return s;
}

int current, prior;

void parse() {
  int tokens;
  while (!feof(fi)) {
    read_line(fi, buffer);
    tokens = count_tabs(buffer);
    if (tokens > 0) {
      current++;
      fprintf(fo, ": ENTRY.%d\n", current);
      if (current == 0)
        fprintf(fo, "d 0\n");
      else
        fprintf(fo, "r ENTRY.%d\n", prior);
      prior = current;
      fprintf(fo, "d %d\n", hash(buffer));
      fprintf(fo, "r ");
      name(buffer);
      fprintf(fo, "\n");
      if (tokens == 2)
        fprintf(fo, "d -1\n");
      else
        fprintf(fo, "d 0\n");
    }
  }
}

void final() {
  fprintf(fo, "\n: Latest\n");
  fprintf(fo, "r ENTRY.%d\n", current);
  fprintf(fo, ": FREE-SPACE\n");
}

int main() {
  current = -1;
  prior = 0;
  fi = fopen("dict.data", "r");
  fo = fopen("forth.dictionary", "w");
  fprintf(fo, "~~~\n");
  parse();
  final();
  fprintf(fo, "~~~\n");
  fclose(fo);
  fclose(fi);
}
