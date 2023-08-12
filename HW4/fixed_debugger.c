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


unsigned long find_dyn_symbol(char* symbol_name, char* exe_file_name){
	FILE* f = fopen(exe_file_name, "r");
	Elf64_Ehdr elf_header;
	fread(&elf_header, sizeof(elf_header), 1, f);
	
	Elf64_Off offset = elf_header.e_shoff;
	Elf64_Word num_sections = elf_header.e_shnum;
	Elf64_Word inx_table_string = 0;
	Elf64_Word inx = 0;
	Elf64_Word offset_name = 0;
    Elf64_Word section_type = 0;
    Elf64_Word link = 0;
    Elf64_Half num_of_sections=elf_header.e_shstrndx;
    Elf64_Half section_size = elf_header.e_shentsize;
	char* section_name = "";
	Elf64_Sym* dynamic_sym = NULL;
	Elf64_Rela* reloc_table = NULL;
	int num_dyn_sym_entries = 0;
	int num_reloc_entries = 0;
	char* dynstr = NULL;
	bool dyn_symbol_exists = false;

	fseek(f, offset, SEEK_SET);
	Elf64_Shdr* headers = malloc(sizeof(*headers)*num_sections);
	fread(headers, sizeof(*headers), num_sections, f);

    Elf64_Shdr section_header;
    fseek(f, offset + (section_size*num_of_sections) , SEEK_SET);
    fread(&section_header, section_size, 1, f);

	Elf64_Xword str_table_size = section_header.sh_size;
	Elf64_Off str_table_offset = section_header.sh_offset;
	
	///reading the string table
	char* str_table = malloc(str_table_size);
	fseek(f, str_table_offset, SEEK_SET);
	fread(str_table, sizeof(char), str_table_size/sizeof(char),f);

	//finding the dynamic symbol
	Elf64_Off curr_off = 0;
	Elf64_Xword curr_size = 0;
	Elf64_Xword	curr_entry_size = 0;
	for(inx = 0; inx < num_sections; ++inx){
		offset_name = headers[inx].sh_name;
		section_name = str_table + offset_name;
		
		
		if(strcmp(section_name, ".dynstr") == 0){
			curr_off = headers[inx].sh_offset;
			curr_size = headers[inx].sh_size;
			fseek(f, curr_off, SEEK_SET);
			dynstr = (char*)malloc(curr_size);
			fread(dynstr, curr_size, 1, f);
		}
		
		else if (strcmp(section_name, ".rela.plt") == 0){
			curr_off = headers[inx].sh_offset;
			curr_size = headers[inx].sh_size;
			curr_entry_size = headers[inx].sh_entsize;
			fseek(f, curr_off, SEEK_SET);
			reloc_table = malloc(curr_size);
			fread(reloc_table, curr_size, 1, f);
			num_reloc_entries = curr_size / curr_entry_size;
		}
		
		else if(strcmp(section_name, ".dynsym") == 0){
			curr_off = headers[inx].sh_offset;
			curr_size = headers[inx].sh_size;
			curr_entry_size = headers[inx].sh_entsize;
			fseek(f, curr_off, SEEK_SET);
			dynamic_sym = malloc(curr_size);
			fread(dynamic_sym, curr_size, 1, f);
            num_dyn_sym_entries = curr_size / curr_entry_size;
		}
	}
	
	Elf64_Addr sym_offset = 0;
	char* dyn_name = "";
	for(inx = 0; inx < num_reloc_entries; ++inx){
		dyn_name = dynstr + dynamic_sym[ELF64_R_SYM(reloc_table[inx].r_info)].st_name;
		if(strcmp(dyn_name, symbol_name) == 0){
			sym_offset = reloc_table[inx].r_offset;
			dyn_symbol_exists = true;
			break;
		}
	}
	unsigned long ret_addr = 0;
	if(dyn_symbol_exists){
		ret_addr = sym_offset;
	}
	free(headers);
	free(str_table);
	free(reloc_table);
	free(dynstr);
	free(dynamic_sym);
	
	return ret_addr;
	
}



///====================================DEBUGGER======================================================



pid_t run_target(const char* programname, char* const argv[])
{
	pid_t pid;
	
	pid = fork();
	
    if (pid > 0) {
		return pid;
		
    } else if (pid == 0) {
		/* Allow tracing of this process */
		if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
			perror("ptrace");
			return -1;
		}
		/* Replace this process's image with the given program */
		execv(programname, argv);
		
	} else {
		// fork error
		perror("fork");
        return -1;
    }
}


void run_double_breakpoint_function_debugger(pid_t child_pid, unsigned long addr, int is_func_shared)
{
    int wait_status;
    struct user_regs_struct regs;
    unsigned long func_addr = addr, old_addr = addr;
    int call_counter=1;
    long entry_data;
    unsigned long entry_trap;
    unsigned long saved_frame_sp;
    unsigned long saved_frame_ra;
    unsigned long return_addr_data, return_addr_trap;
    unsigned long first_arg, return_arg;

    /* Wait for child to stop on its first instruction */
    wait(&wait_status);   
    if(is_func_shared==1)
    {
        func_addr = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)addr, NULL);
        func_addr -= 6;
    }

    /* Look at the instruction at the entry address */
	entry_data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)func_addr, NULL);
		
	/* Write the trap instruction 'int 3' into the address */
	entry_trap = (entry_data & 0xFFFFFFFFFFFFFF00) | 0xCC;
	ptrace(PTRACE_POKETEXT, child_pid, (void*)func_addr, (void*)entry_trap);
			
	/* Let the child run to the breakpoint and wait for it to reach it */
	ptrace(PTRACE_CONT, child_pid, NULL, NULL);
	wait(&wait_status);   
    
    /* we are at function entry. */
    /* get information from the first call to the child */
	ptrace(PTRACE_GETREGS, child_pid, NULL, &regs);
	regs.rip -= 1;
    saved_frame_sp=regs.rsp;
    first_arg = regs.rdi;
	saved_frame_ra = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)saved_frame_sp, NULL);

	/* Set BP at RA instruction */
	return_addr_data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)saved_frame_ra, NULL);
	return_addr_trap = (return_addr_data & 0xFFFFFFFFFFFFFF00) | 0xCC;
	ptrace(PTRACE_POKETEXT, child_pid, (void*)saved_frame_ra, (void*)return_addr_trap);

	/* Remove the entry breakpoint */
	ptrace(PTRACE_POKETEXT, child_pid, (void*)func_addr, (void*)entry_data); //restore instruction
	
	if(	WIFSTOPPED(wait_status))
	{
		/*if we enterd the function during the code running*/
		///step 6.a
		printf("PRF:: run #%d first parameter is %d\n", call_counter, (int)first_arg);
		///end of step
	}

	
	/* The child can continue running now */
	ptrace(PTRACE_SETREGS, child_pid, 0, &regs);
	ptrace(PTRACE_CONT, child_pid, 0, 0);
	
	wait(&wait_status);

	/* get information from the current BP */
    while(WIFSTOPPED(wait_status))
    {
	/* We are at function RA */
		ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
		regs.rip -= 1;

		/* if this is BP at entry of func: */
		if(regs.rip == func_addr)
		{
			/// step 6.a				
			saved_frame_sp=regs.rsp;
			first_arg = regs.rdi;
				
			printf("PRF:: run #%d first parameter is %d\n", call_counter, (int)first_arg);
				
			saved_frame_ra = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)(saved_frame_sp), NULL);
				
			/* Set BP at RA instruction */
			return_addr_data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)(saved_frame_ra), NULL); //set ra BP below
			return_addr_trap = (return_addr_data & 0xFFFFFFFFFFFFFF00) | 0xCC;
			ptrace(PTRACE_POKETEXT, child_pid, (void*)saved_frame_ra, (void*)return_addr_trap);
			
			///end of step						
			/* remove entry BP */
			ptrace(PTRACE_POKETEXT, child_pid, (void*)func_addr, (void*)entry_data);
			ptrace(PTRACE_SETREGS, child_pid, 0, &regs);		
		}
		else //we are at RA breakpoint
		{
			/// step 6.b
			if(regs.rsp > saved_frame_sp) // if we returned from func
			{
				saved_frame_sp=regs.rsp;
				return_arg = regs.rax;
				printf("PRF:: run #%d returned with %d\n", call_counter, (int)return_arg);

				/*remove BP at RA */
				ptrace(PTRACE_POKETEXT, child_pid, (void*)saved_frame_ra, (void*)return_addr_data);
				
				/* Write the trap instruction 'int 3' into the address */
				/* if the function is part of a shared lib, need to fix the adress of BP from plt to code  */

				if(is_func_shared==1 && call_counter==1)
				{
					func_addr = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)old_addr, NULL);
            
					/* Look at the instruction at the entry address */
					entry_data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)func_addr, NULL);
	
					/* Write the trap instruction 'int 3' into the address */
					entry_trap = (entry_data & 0xFFFFFFFFFFFFFF00) | 0xCC;					
				}
				
				call_counter++;
						
				ptrace(PTRACE_POKETEXT, child_pid, (void*)func_addr, (void*)entry_trap);
				ptrace(PTRACE_SETREGS, child_pid, 0, &regs);	
			}
			else // it's recursive call or we got here by executing instructions that part of the func
			{
				/* Remove BP at RA */
				ptrace(PTRACE_POKETEXT, child_pid, (void*)saved_frame_ra, (void*)return_addr_data);
				ptrace(PTRACE_SETREGS, child_pid, 0, &regs);
								
				/* Execute single instruction */
				ptrace(PTRACE_SINGLESTEP, child_pid, NULL,NULL);
				wait(&wait_status);
	
				/* Restore BP at RA */
				ptrace(PTRACE_POKETEXT, child_pid, (void*)saved_frame_ra, (void*)return_addr_trap);	
			}
		}
		/* The child can continue running now */
		ptrace(PTRACE_CONT, child_pid, 0, 0);
		wait(&wait_status);	
	}
	
}


int main(int argc, char** argv)
{
	int err = 0;
    unsigned long addr = find_symbol(argv[1], argv[2], &err);
	//our code
	// phase 1
	if (err == -3){
		printf("PRF:: %s not an executable!\n", argv[2]);
		return 0;
	}
	//phase 2
	if (err == -1){
		printf("PRF:: %s not found! :(\n", argv[1]);
		return 0;
	}
	//phase 3
	if (err == -2){
		printf("PRF:: %s is not a global symbol!\n", argv[1]);
		return 0;
	}
	
	int is_func_shared = 0;
	//phases 4 and 5
	if (err == -4){
		addr = find_dyn_symbol(argv[1], argv[2]);
		is_func_shared = 1;
	}
	
	//phase 6
	
    pid_t child_pid;
    child_pid = run_target(argv[2],argv+2);


	// run specific "debugger"
	run_double_breakpoint_function_debugger(child_pid,addr,is_func_shared);

    return 0;
}
