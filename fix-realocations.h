/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>*/


typedef unsigned int* Elf32_Addr;
typedef int Elf32_Off;
typedef int Elf32_Word;
typedef unsigned short int Elf32_Half;

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




int main_fix_realocations()
{
	/*if (argc != 2)
	{
		printf("usage: fix-relocations inout.prx\n");
		return -1;
	}*/

	FILE* elf_file = fopen("data_unsigned.psp", "r+b");
	
	if (elf_file)
	{
		int elf_size = 0;
		
		char* elf_buffer = elf;//(char*)out_buffer;//(char*)malloc(MAX_ORIGIN_FILE_SIZE);
		elf_size = fread(elf_buffer, 1, MAX_ORIGIN_FILE_SIZE, elf_file);
		
		Elf32_Ehdr_fix* elf_hdr = (Elf32_Ehdr_fix*)&elf_buffer[0];
		
		if (!((elf_hdr->e_ident[0] == 0x7F) && (strncmp((char*)&elf_hdr->e_ident[1], "ELF", 3) == 0) && (elf_hdr->e_type == 0xFFA0)))
		{
			printf("Error: not a PRX\n");

			fclose(elf_file);
			//free(elf_buffer);

			return -1;
		}
		
		Elf32_Shdr_fix* sec_hdr = NULL;
		Elf32_Rel* reloc = NULL;

		int count = 0;
		
		int i, j;
		for (i = 0; i < elf_hdr->e_shnum; i++)
		{
			sec_hdr = (Elf32_Shdr_fix*)&elf_buffer[elf_hdr->e_shoff + (i * sizeof(Elf32_Shdr_fix))];
			
			if (sec_hdr->sh_type == 0x700000A0)
			{
				int num = sec_hdr->sh_size / sizeof(Elf32_Rel);
				for (j = 0; j < num; j++)
				{
					reloc = (Elf32_Rel*)&elf_buffer[sec_hdr->sh_offset + (j * sizeof(Elf32_Rel))];
					
					if ((reloc->r_info & 0xFF) == 7)
					{
						reloc->r_info = (reloc->r_info & 0xFFFFFF00);
						count++;
					}
				}
			}
		}
		
		fseek(elf_file, 0, SEEK_SET);
		fwrite(elf_buffer, 1, elf_size, elf_file);

		//free(elf_buffer);

	}
	else
	{
		printf("Error: cannot open file\n");
		return -1;
	}

	return 0;
}
