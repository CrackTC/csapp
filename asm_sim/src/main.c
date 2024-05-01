#include "conf.h"
#include "elf.h"
#include "examples.h"
#include "machine.h"
#include "utils.h"

int main() {
  CLEANUP(free_machine_ptr)
  machine_t *machine = new_machine(CORE_COUNT, MM_SIZE);
  CLEANUP(free_elf_ptr) elf_t *elf = add();

  machine_run(machine, elf, 0);
  return 0;
}
