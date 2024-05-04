/* RETRO ------------------------------------------------------
  A personal, minimalistic forth
  Copyright (c) Charles Childers

  This is retro-unu, a tool to extract code blocks from a
  RETRO-flavored Markdown source file.

  Code blocks start and end with ~~~ and test blocks start
  and end with ```, though this does support setting them
  from the command line.
  ---------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef void (*Handler)(char *);

char code_start[33], code_end[33], test_start[33], test_end[33];


void read_line(FILE *file, char *line_buffer) {
  int ch = getc(file);
  int count = 0;
  while ((ch != '\n') && (ch != EOF)) {
    line_buffer[count] = ch;
    count++;
    ch = getc(file);
  }
  line_buffer[count] = '\0';
}


/* Check to see if a line is a fence boundary.
   This will check code blocks in all cases, and test blocks
   if tests_enabled is set to a non-zero value. */

int fence_boundary(char *buffer, int tests_enabled) {
  int flag = 1;
  if (strcmp(buffer, code_start) == 0) { flag = -1; }
  if (strcmp(buffer, code_end) == 0)   { flag = -1; }
  if (tests_enabled == 0) { return flag; }
  if (strcmp(buffer, test_start) == 0) { flag = -1; }
  if (strcmp(buffer, test_end) == 0)   { flag = -1; }
  return flag;
}


/* The actual guts of this are handled here. Pass in
   a file name, a flag to indicate if you want to also
   extract tests, and a Handler function pointer. The
   Handler will be called once for each line in a block,
   with the line being passed as a character array
   pointer. */

void unu(char *fname, int tests_enabled, Handler handler) {
  int inBlock = 0;
  char buffer[4096];
  FILE *fp;
  fp = fopen(fname, "r");
  if (fp == NULL) {
    printf("Unable to load file\n");
    exit(2);
  }
  while (!feof(fp)) {
    read_line(fp, buffer);
    if (fence_boundary(buffer, tests_enabled) == -1) {
      if (inBlock == 0) {
        inBlock = 1;
      } else {
        inBlock = 0;
      }
    } else {
      if (inBlock == 1) {
        handler(buffer);
      }
    }
  }
  fclose(fp);
}


/* The default behavior for Unu is to display the line */

void display(char *buffer) {
  printf("%s\n", buffer);
}


/* Just a readabilty aid for the command line processing */
int arg_is(char *arg, char *value) {
  return (strcmp(arg, value) == 0);
}


int main(int argc, char **argv) {
  int tests = 0;
  int i = 1;
  strlcpy(code_start, "~~~", 33);
  strlcpy(code_end,   "~~~", 33);
  strlcpy(test_start, "```", 33);
  strlcpy(test_end,   "```", 33);
  if (argc > 1) {
    while (i < argc) {
      if (arg_is(argv[i], "--code-start") || arg_is(argv[i], "-cs")) {
        i++;
        strlcpy(code_start, argv[i], 33);
      } else if (arg_is(argv[i], "--code-end") || arg_is(argv[i], "-ce")) {
        i++;
        strlcpy(code_end, argv[i], 33);
      } else if (arg_is(argv[i], "--test-start") || arg_is(argv[i], "-ts")) {
        i++;
        strlcpy(test_start, argv[i], 33);
      } else if (arg_is(argv[i], "--test-end") || arg_is(argv[i], "-te")) {
        i++;
        strlcpy(test_end, argv[i], 33);
      } else if (arg_is(argv[i], "--include-tests") || arg_is(argv[i], "-t")) {
        tests = -1;
      } else {
        unu(argv[i], tests, &display);
      }
      i++;
    }
  }
  else {
    printf("err: no file specified\n");
  }
  return 0;
}
