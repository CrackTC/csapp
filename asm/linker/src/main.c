#include "common.h"
#include "elf_parse.h"
#include "linker/elf_info.h"
#include "linker/linker.h"
#include "stack.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char **read_all_lines(const char *filename, size_t *n) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "Failed to open file %s: %s\n", filename, strerror(errno));
    return NULL;
  }

  CLEANUP(free_stack_ptr) stack_t *stack = new_stack();
  while (!feof(file)) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read = getline(&line, &len, file);
    if (read == -1) {
      free(line);
      break;
    }

    line[read - 1] = '\0';
    stack_add(stack, line);
  }

  *n = stack_size(stack);
  const char **lines = malloc(*n * sizeof(char *));
  int i = *n - 1;
  STACK_FOR(stack, node) { lines[i--] = stack_data(node); }

  fclose(file);
  return lines;
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf("Usage: %s <output> <input1> <input2> ... <inputN>\n", argv[0]);
    return 1;
  }

  elf_t **elfs = malloc((argc - 2) * sizeof(elf_t *));
  for (size_t i = 0; i < argc - 2; ++i) {
    elfs[i] = malloc(sizeof(elf_t));
  }

  for (int i = 0; i < argc - 2; ++i) {
    size_t n;
    const char **lines = read_all_lines(argv[i + 2], &n);
    if (lines == NULL) {
      printf("Failed to read file: %s\n", argv[i + 2]);
      return 1;
    }

    if (parse_elf(lines, elfs[i]) != 0) {
      printf("Failed to parse ELF file: %s\n", argv[i + 2]);
      return 1;
    }

    for (size_t l = 0; l < n; ++l) {
      free((char *)lines[l]);
    }
    free(lines);
  }

  elf_t *output = malloc(sizeof(elf_t));
  if (link_objects(elfs, argc - 2, output, 1) != 0) {
    printf("Failed to link ELF files\n");
    return 1;
  }

  FILE *file = fopen(argv[1], "w");
  if (file == NULL) {
    fprintf(stderr, "Failed to open file %s: %s\n", argv[1], strerror(errno));
    return 1;
  }

  write_elf(output, file);
  fclose(file);
  fprintf(stderr, "output written to %s\n", argv[1]);

  free_elf_t(output);
  for (size_t i = 0; i < argc - 2; ++i) {
    free_elf_t(elfs[i]);
  }
  free(elfs);
}
