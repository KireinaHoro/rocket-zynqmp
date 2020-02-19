#include "elf.h"
#include "myprintf.h"
#include <stddef.h>

void *memset(void *s, int c, size_t n) {
#ifdef DEBUG
  printf("memset(%p, ", (uint64_t)s);
  printf("%d, ", c);
  printf("%p)...", n);
#endif
  if (NULL == s || n < 0)
    return NULL;
  char *tmpS = (char *)s;
  while (n-- > 0)
    *tmpS++ = c;
  printf("done.\r\n", 0);
  return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
#ifdef DEBUG
  printf("memcpy(%p, ", (uint64_t)dest);
  printf("%p, ", (uint64_t)src);
  printf("%p)...", n);
#endif
  if (NULL == dest || NULL == src || n < 0)
    return NULL;
  char *tempDest = (char *)dest;
  char *tempSrc = (char *)src;

  while (n-- > 0)
    *tempDest++ = *tempSrc++;
  printf("done.\r\n", 0);
  return dest;
}

int load_elf(const uint8_t *elf, const uint32_t elf_size) {
  // sanity checks
  if (elf_size <= sizeof(Elf64_Ehdr))
    return 1; /* too small */

  const Elf64_Ehdr *eh = (const Elf64_Ehdr *)elf;
  if (!IS_ELF64(*eh))
    return 2; /* not a elf64 file */

  const Elf64_Phdr *ph = (const Elf64_Phdr *)(elf + eh->e_phoff);
  if (elf_size < eh->e_phoff + eh->e_phnum * sizeof(*ph))
    return 3; /* internal damaged */

  uint32_t i;
  for (i = 0; i < eh->e_phnum; i++) {
    if (ph[i].p_type == PT_LOAD &&
        ph[i].p_memsz) { /* need to load this physical section */
      printf("[load_elf] still alive... writing %d bytes to ", ph[i].p_filesz);
      printf("%p \n\r", (uintptr_t)ph[i].p_paddr);

      if (ph[i].p_filesz) { /* has data */
        if (elf_size < ph[i].p_offset + ph[i].p_filesz)
          return 3; /* internal damaged */
        memcpy((uint8_t *)ph[i].p_paddr, elf + ph[i].p_offset, ph[i].p_filesz);
      }
      if (ph[i].p_memsz > ph[i].p_filesz) { /* zero padding */
        memset((uint8_t *)ph[i].p_paddr + ph[i].p_filesz, 0,
               ph[i].p_memsz - ph[i].p_filesz);
      }
    }
  }

  return 0;
}
