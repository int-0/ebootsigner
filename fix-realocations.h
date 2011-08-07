#include "types.h"

typedef u32 Elf32_Addr;
typedef s32 Elf32_Off;
typedef s32 Elf32_Word;
typedef u16 Elf32_Half;

#define	EI_NIDENT	16

typedef struct {
	unsigned char	e_ident[EI_NIDENT];
	Elf32_Half	e_type;		
	Elf32_Half	e_machine;	
	Elf32_Word	e_version;	
	Elf32_Addr	e_entry;	
	Elf32_Off	e_phoff;	
	Elf32_Off	e_shoff;	
	Elf32_Word	e_flags;	
	Elf32_Half	e_ehsize;	
	Elf32_Half	e_phentsize;
	Elf32_Half	e_phnum;	
	Elf32_Half	e_shentsize;
	Elf32_Half	e_shnum;	
	Elf32_Half	e_shstrndx;	
} Elf32_Ehdr_fix;

typedef struct {
	Elf32_Word	p_type;	
	Elf32_Off	p_offset;
	Elf32_Addr	p_vaddr;
	Elf32_Addr	p_paddr;
	Elf32_Word	p_filesz;
	Elf32_Word	p_memsz;
	Elf32_Word	p_flags;
	Elf32_Word	p_align;
} Elf32_Phdr_fix;

typedef struct {
	Elf32_Word	sh_name;
	Elf32_Word	sh_type;
	Elf32_Word	sh_flags;
	Elf32_Addr	sh_addr;
	Elf32_Off	sh_offset;
	Elf32_Word	sh_size;
	Elf32_Word	sh_link;
	Elf32_Word	sh_info;
	Elf32_Word	sh_addralign;
	Elf32_Word	sh_entsize;
} Elf32_Shdr_fix;

typedef struct {
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
} Elf32_Rel;

int fix_realocations();
