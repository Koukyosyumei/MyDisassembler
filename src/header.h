#include <iostream>
#include <vector>

typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} ELF64_FILE_HEADER;

typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} ELF64_SECTION_HEADER;

typedef struct {
    uint32_t st_name;        // 4
    unsigned char st_info;   // 1
    unsigned char st_other;  // 1
    uint16_t st_shndx;       // 2
    uint64_t st_value;       // 8
    uint64_t st_size;        // 8
} ELF64_SYM;


typedef struct
{
  uint64_t	r_offset;		/* Address */
  uint64_t	r_info;			/* Relocation type and symbol index */
  int64_t	r_addend;		/* Addend */
} ELF64_RELA;
