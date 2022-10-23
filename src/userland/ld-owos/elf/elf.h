#pragma once

#include <stdint.h>

// The definitions in this file are adapted from the Linux Foundation ELF Specification Version 1.2
// See: https://refspecs.linuxfoundation.org/elf/elf.pdf

// Elf header identification
#define EI_MAG0    0  // File identification
#define EI_MAG1    1  // File identification
#define EI_MAG2    2  // File identification
#define EI_MAG3    3  // File identification
#define EI_CLASS   4  // File class
#define EI_DATA    5  // Data encoding
#define EI_VERSION 6  // File version
#define EI_PAD     7  // Start of padding bytes
#define EI_NIDENT  16 // Size of e_ident[]

// ELF signature
#define ELFMAG0 0x7f // e_ident[EI_MAG0]
#define ELFMAG1 0x45 // e_ident[EI_MAG1]
#define ELFMAG2 0x4C // e_ident[EI_MAG2]
#define ELFMAG3 0x46 // e_ident[EI_MAG3]

// File type specified in elf32_ehdr->e_type
#define ET_NONE   0      // No file type
#define ET_REL    1      // Relocatable file
#define ET_EXEC   2      // Executable file
#define ET_DYN    3      // Shared object file
#define ET_CORE   4      // Core file
#define ET_LOPROC 0xff00 // Processor-specific
#define ET_HIPROC 0xffff // Processor-specific

// Machine specified in elf32_ehdr->e_machine
#define ET_NONE        0  // No machine
#define EM_M32         1  // AT&T WE 32100
#define EM_SPARC       2  // SPARC
#define EM_386         3  // Intel Architecture
#define EM_68K         4  // Motorola 68000
#define EM_88K         5  // Motorola 88000
#define EM_860         7  // Intel 80860
#define EM_MIPS        8  // MIPS RS3000 Big-Endian
#define EM_MIPS_RS4_BE 10 // MIPS RS4000 Big-Endian

// File class specified in elf32_ehdr->e_indent[EI_CLASS]
#define ELFCLASSNONE 0 // Invalid class
#define ELFCLASS32   1 // 32-bit objects
#define ELFCLASS64   2 // 64-bit objects

// File encoding specified in elf32_ehdr->e_indent[EI_DATA]
#define ELFDATANONE 0 // Invalid data encoding
#define ELFDATA2LSB 1 // See below
#define ELFDATA2MSB 2 // See below

// Physical header types
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7fffffff

// Physical header segment flags
#define PF_X        0x1        // Execute
#define PF_W        0x2        // Write
#define PF_R        0x4        // Read
#define PF_MASKPROC 0xf0000000 // Unspecified

// Dynamic Array Tags
#define DT_NULL 0
#define DT_NEEDED 1
#define DT_PLTRELSZ 2
#define DT_PLTGOT 3
#define DT_HASH 4
#define DT_STRTAB 5
#define DT_SYMTAB 6
#define DT_RELA 7
#define DT_RELASZ 8
#define DT_RELAENT 9
#define DT_STRSZ 10
#define DT_SYMENT 11
#define DT_INIT 12
#define DT_FINI 13
#define DT_SONAME 14
#define DT_RPATH 15
#define DT_SYMBOLIC 16
#define DT_REL 17
#define DT_RELSZ 18
#define DT_RELENT 19
#define DT_PLTREL 20
#define DT_DEBUG 21
#define DT_TEXTREL 22
#define DT_JMPREL 23
#define DT_BIND_NOW 24
#define DT_LOPROC 0x70000000
#define DT_HIPROC 0x7fffffff

// Parse Elf32_Rel->r_info
#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

#define R_386_GOT32 3
#define R_386_PLT32 4
#define R_386_COPY 5
#define R_386_GLOB_DAT 6
#define R_386_JMP_SLOT 7
#define R_386_RELATIVE 8
#define R_386_GOTOFF 9
#define R_386_GOTPC 10

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

typedef struct
{
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} __attribute__((packed)) Elf32_Ehdr;

typedef struct
{
	Elf32_Word p_type;
	Elf32_Off p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} __attribute__((packed)) Elf32_Phdr;

typedef struct {
	Elf32_Sword d_tag;
	union {
		Elf32_Word d_val;
		Elf32_Addr d_ptr;
	} d_un;
} __attribute__((packed)) Elf32_Dyn;

typedef struct {
	Elf32_Addr r_offset;
	Elf32_Word r_info;
} __attribute__((packed)) Elf32_Rel;

typedef struct {
	Elf32_Addr r_offset;
	Elf32_Word r_info;
	Elf32_Sword r_addend;
} __attribute__((packed)) Elf32_Rela;
