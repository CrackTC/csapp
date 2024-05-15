#include "common.h"
#include "conf.h"
#include "machine.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  CLEANUP(free_machine_ptr)
  machine_t *machine = new_machine(CORE_COUNT, MM_SIZE);

  const char *filename = argv[1];
  FILE *file = fopen(filename, "re");

  if (file == NULL) {
    puts(strerror(errno));
    return 1;
  }

  int capacity = 1024;
  const char **lines = malloc(capacity * sizeof(char *));

  int line_count = 0;
  while (1) {
    if (line_count >= capacity) {
      capacity *= 2;
      lines = realloc(lines, capacity * sizeof(char *));
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read = getline(&line, &len, file);

    if (read == -1) {
      free(line);
      break;
    }

    lines[line_count] = line;
    line[read - 1] = '\0'; // remove '\n' at the end
    line_count++;
  }

  machine_run_text(machine, line_count, lines, 0);

  for (int i = 0; i < line_count; i++) {
    free((void *)lines[i]);
  }

  free(lines);
  return 0;
}
