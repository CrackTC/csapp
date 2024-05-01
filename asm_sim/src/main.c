#include "conf.h"
#include "examples.h"
#include "machine.h"
#include "utils.h"

int main() {
  _CLEANUP_(free_machine_ptr) machine_t *machine = new_machine(MM_SIZE);
  _CLEANUP_(free_elf_ptr) elf_t *elf = add();

  machine_load_elf(machine, elf);
  machine_run(machine);
  return 0;
}
