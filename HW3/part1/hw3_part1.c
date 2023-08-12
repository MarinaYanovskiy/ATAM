#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include "elf64.h"

#define	ET_NONE	0	//No file type 
#define	ET_REL	1	//Relocatable file 
#define	ET_EXEC	2	//Executable file 
#define	ET_DYN	3	//Shared object file 
#define	ET_CORE	4	//Core file 


/* symbol_name		- The symbol (maybe function) we need to search for.
 * exe_file_name	- The file where we search the symbol in.
 * error_val		- If  1: A global symbol was found, and defined in the given executable.
 * 			- If -1: Symbol not found.
 *			- If -2: Only a local symbol was found.
 * 			- If -3: File is not an executable.
 * 			- If -4: The symbol was found, it is global, but it is not defined in the executable.
 * return value		- The address which the symbol_name will be loaded to, if the symbol was found and is global.
 */
unsigned long find_symbol(char* symbol_name, char* exe_file_name, int* error_val) {
	//reading the file
	Elf64_Ehdr elf_header;
	FILE* f = fopen(exe_file_name, "r");
	fread(&elf_header, sizeof(elf_header), 1, f);
	
	//checking if the file is executable
	//if the field e_type of the elf header is 2, the file is executable
	if(elf_header.e_type != ET_EXEC){
		*error_val = -3;
		return 0;
	}
	
	//when the file is executable (till the end of the function)
	
	Elf64_Off offset = elf_header.e_shoff;
	Elf64_Word num_sections = elf_header.e_shnum;
	Elf64_Word inx_table_string = 0;
	Elf64_Word inx_table_symbol = 0;
	Elf64_Word inx = 0;
	Elf64_Word section_type = 0;
	Elf64_Word link = 0;
	bool symbol_table_exists = false;
	
	//finding the index of the string table and of the symbol table
	fseek(f, offset, SEEK_SET);
	Elf64_Shdr* headers = malloc(sizeof(Elf64_Shdr)*num_sections);
	fread(headers, sizeof(Elf64_Shdr), num_sections, f);
	
	for(inx = 0; inx < num_sections; ++inx){
		section_type = headers[inx].sh_type;
		link = headers[inx].sh_link;
		if(section_type == 2){
			inx_table_string = link;
			inx_table_symbol = inx;
			symbol_table_exists = true;
			break;
		}
	}
	if(inx == num_sections && !symbol_table_exists){
		*error_val = -1;
		return 0;
	}
	
	Elf64_Xword str_table_size = headers[inx_table_string].sh_size;
	Elf64_Off str_table_offset = headers[inx_table_string].sh_offset;
	Elf64_Xword sym_table_size = headers[inx_table_symbol].sh_size;
	Elf64_Off sym_table_offset = headers[inx_table_symbol].sh_offset;
	int num_symbols = sym_table_size/sizeof(Elf64_Sym);
	bool symbol_exists = false;
	bool symbol_is_global = false;
	Elf64_Addr virtual_addr;
	Elf64_Word info = 0;
	
	//reading the string table
	char* str_table = malloc(headers[inx_table_string].sh_size);
	fseek(f, headers[inx_table_string].sh_offset, SEEK_SET);
	fread(str_table, sizeof(char), (str_table_size/sizeof(char)),f);
	
	//reading the symbol table
	Elf64_Sym* sym_table = malloc(headers[inx_table_symbol].sh_size);
	fseek(f, headers[inx_table_symbol].sh_offset, SEEK_SET);
	fread(sym_table, sizeof(Elf64_Sym), num_symbols,f);
	
	free(headers);
	char* name = "";
	for(int sym=0; sym < num_symbols; ++sym){
		name = str_table + sym_table[sym].st_name;
		
		if(strcmp(symbol_name, name) == 0){
			info = sym_table[sym].st_info;
			//if symbol is global
			if(ELF64_ST_BIND(info) == 1){
				symbol_is_global = true;
			}
			symbol_exists = true;
			//when the symbol is global and in an executable file
			virtual_addr = sym_table[sym].st_value;
			if(symbol_is_global && virtual_addr != 0){
				free(str_table);
				free(sym_table);
				*error_val = 1;
				return virtual_addr;
			}
		}
		
	}
	
	if(symbol_exists && !symbol_is_global){
		*error_val = -2;
	}
	if(symbol_exists && symbol_is_global){
		*error_val = -4;
	}
	if(!symbol_exists){
		*error_val = -1;
	}
	
	free(str_table);
	free(sym_table);
	return 0;
}

int main(int argc, char *const argv[]) {
	int err = 0;
	unsigned long addr = find_symbol(argv[1], argv[2], &err);

	if (err >= 0)
		printf("%s will be loaded to 0x%lx\n", argv[1], addr);
	else if (err == -2)
		printf("%s is not a global symbol! :(\n", argv[1]);
	else if (err == -1)
		printf("%s not found!\n", argv[1]);
	else if (err == -3)
		printf("%s not an executable! :(\n", argv[2]);
	else if (err == -4)
		printf("%s is a global symbol, but will come from a shared library\n", argv[1]);
	return 0;
}
