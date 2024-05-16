#include "common.h"
#include "elf_parse.h"
#include "linker/elf_info.h"
#include "linker/linker.h"
#include "list.h"
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

  CLEANUP(free_list_ptr) list_t *list = new_list();
  while (!feof(file)) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read = getline(&line, &len, file);
    if (read == -1) {
      free(line);
      break;
    }

    line[read - 1] = '\0';
    list_add(list, line);
  }

  *n = list_size(list);
  const char **lines = malloc(*n * sizeof(char *));
  int i = *n - 1;
  LIST_FOR(list, node) { lines[i--] = list_data(node); }

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

  CLEANUP(free_list_ptr) list_t *lines_list = new_list();
  CLEANUP(free_list_ptr) list_t *lines_count_list = new_list();

  for (int i = 0; i < argc - 2; ++i) {
    size_t *pn = malloc(sizeof(size_t));
    const char **lines = read_all_lines(argv[i + 2], pn);
    if (lines == NULL) {
      printf("Failed to read file: %s\n", argv[i + 2]);
      return 1;
    }

    if (parse_elf(lines, elfs[i]) != 0) {
      printf("Failed to parse ELF file: %s\n", argv[i + 2]);
      return 1;
    }

    list_add(lines_list, lines);
    list_add(lines_count_list, pn);
  }

  elf_t *output = malloc(sizeof(elf_t));
  if (link_executable(elfs, argc - 2, output) != 0) {
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

  for (size_t j = 0; j < output->section_count; ++j) {
    free(output->sections[j].name);
  }
  free(output->sections);
  for (size_t j = 0; j < output->symbol_count; ++j) {
    free(output->symbols[j].name);
  }
  free(output->symbols);
  free(output->lines);
  free(output);

  for (size_t i = 0; i < argc - 2; ++i) {
    for (size_t j = 0; j < elfs[i]->section_count; ++j) {
      free(elfs[i]->sections[j].name);
    }
    free(elfs[i]->sections);

    for (size_t j = 0; j < elfs[i]->symbol_count; ++j) {
      free(elfs[i]->symbols[j].name);
    }
    free(elfs[i]->symbols);
    free(elfs[i]->lines);
    free(elfs[i]);
  }

  list_node_t *count_node = list_head(lines_count_list);
  LIST_FOR(lines_list, node) {
    size_t *pn = list_data(count_node);
    char **lines = list_data(node);

    for (size_t i = 0; i < *pn; ++i) {
      free(lines[i]);
    }
    free(lines);
    free(pn);

    count_node = list_next(count_node);
  }

  free(elfs);
}
