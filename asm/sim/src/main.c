#include "common.h"
#include "conf.h"
#include "machine.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  CLEANUP(free_machine_ptr)
  machine_t *machine = new_machine(CORE_COUNT, MM_SIZE);
  // CLEANUP(free_elf_ptr) elf_t *elf = add();

  const char *filename = "/home/chen/proj/c/csapp/asm/sim/src/examples/add.txt";
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
  return 0;
}
