//
//  elf_test.cpp
//  hardware-control
//
//  Created by Julian Becker on 30.01.16.
//  Copyright (c) 2016 Julian Becker. All rights reserved.
//

#include <catch.h>
#include <vector>
#include <iostream>
#include <fstream>

constexpr size_t EI_NIDENT = 16u;
using Elf32_Addr = unsigned int;
using Elf32_Half = unsigned short;
using Elf32_Off = unsigned int;
using Elf32_Sword = unsigned int;
using Elf32_Word = unsigned int;



enum class EType : Elf32_Half {
    ET_NONE = 0u,
    ET_REL = 1u,
    ET_EXEC = 2u,
    ET_DYN = 3u,
    ET_CORE = 4u,
    ET_LOPROC = 0xff00u,
    ET_HIPROC = 0xffffu,
};



enum class EMachine : Elf32_Half {
    EM_NONE = 0u,
    EM_M32 = 1u,
    EM_SPARC = 2u,
    EM_386 = 3u,
    EM_68K = 4u,
    EM_88K = 5u,
    EM_860 = 7u,
    EM_MIPS = 8u,
};

enum class EVersion : Elf32_Word {
    EV_NONE = 0u,
    EV_CURRENT = 1u,
};

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    EType e_type;
    EMachine e_machine;
    EVersion e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff; // program header table's file offset
    Elf32_Off e_shoff; // section header table's file offset
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
} Elf32_Ehdr;


/// Indices into Elf32_Ehdr.e_ident
constexpr size_t EI_MAG0 = 0u;
constexpr size_t EI_MAG1 = 1u;
constexpr size_t EI_MAG2 = 2u;
constexpr size_t EI_MAG3 = 3u;
constexpr size_t EI_CLASS = 4u;
constexpr size_t EI_DATA = 5u;
constexpr size_t EI_VERSION = 6u;
constexpr size_t EI_PAD = 7u;


/// valid values for Elf32_Ehdr.e_ident[ELFMAG0]:
constexpr unsigned char ELFMAG0 = 0x7F;
/// valid values for Elf32_Ehdr.e_ident[ELFMAG1]:
constexpr unsigned char ELFMAG1 = 'E';
/// valid values for Elf32_Ehdr.e_ident[ELFMAG2]:
constexpr unsigned char ELFMAG2 = 'L';
/// valid values for Elf32_Ehdr.e_ident[ELFMAG3]:
constexpr unsigned char ELFMAG3 = 'F';

/// valid values for Elf32_Ehdr.e_ident[EI_CLASS]:
constexpr unsigned char ELFCLASSNONE = 0u;
constexpr unsigned char ELFCLASS32 = 1u;
constexpr unsigned char ELFCLASS64 = 2u;

/// valid values for Elf32_Ehdr.e_ident[EI_DATA]:
constexpr unsigned char ELFDATANONE = 0u;
constexpr unsigned char ELFDATA2LSB = 1u;
constexpr unsigned char ELFDATA2MSB = 2u;

/// valid values for Elf32_Ehdr.e_ident[EI_VERSION]: EV_CURRENT
constexpr unsigned char ELFVERSION = static_cast<unsigned char>(EVersion::EV_CURRENT);


/// ----------------------------------

enum class EPType : Elf32_Word {
    PT_NULL = 0u,
    PT_LOAD = 1u,
    PT_DYNAMIC = 2u,
    PT_INTERP = 3u,
    PT_NOTE = 4u,
    PT_SHLIB = 5u,
    PT_PHDR = 6u,
    PT_LOPROC = 0x70000000u,
    PT_HIPROC = 0x7fffffffu,
};



/// ELF program header
typedef struct {
    EPType p_type;
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
} Elf32_Phdr;



enum class ESHType : Elf32_Word {
    SHT_NULL = 0u,
    SHT_PROGBITS = 1u,
    SHT_SYMTAB = 2u,
    SHT_STRTAB = 3u,
    SHT_RELA = 4u,
    SHT_HASH = 5u,
    SHT_DYNAMIC = 6u,
    SHT_NOTE = 7u,
    SHT_NOBITS = 8u,
    SHT_REL = 9u,
    SHT_SHLIB = 10u,
    SHT_DYNSYM = 11u,
    SHT_LOPROC = 0x70000000u,
    SHT_HIPROC = 0x7fffffffu,
    SHT_LOUSER = 0x80000000u,
    SHT_HIUSER = 0xffffffffu,
};


typedef struct {
    Elf32_Word sh_name;
    ESHType sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} Elf32_Shdr;


static constexpr Elf32_Word SHF_WRITE = 0x1u;
static constexpr Elf32_Word SHF_ALLOC = 0x2u;
static constexpr Elf32_Word SHF_EXECINSTR = 0x4u;
static constexpr Elf32_Word SHF_MASKPROC = 0xf0000000u;


static constexpr Elf32_Half SHN_UNDEF = 0u;
static constexpr Elf32_Half SHN_LORESERVE = 0xff00u;
static constexpr Elf32_Half SHN_LOPROC = 0xff00u;
static constexpr Elf32_Half SHN_HIPROC = 0xff1fu;
static constexpr Elf32_Half SHN_ABS = 0xfff1u;
static constexpr Elf32_Half SHN_COMMON = 0xfff2u;
static constexpr Elf32_Half SHN_HIRESERVE = 0xffffu;

enum  	{ PF_X = 0x1, PF_W = 0x2, PF_R = 0x4 };






namespace Assert {
    
    struct test_passed { static constexpr bool passed = true; };
    struct test_failed { static constexpr bool passed = false; };
    
    template <bool pass> struct check_true : test_failed {};
    template <> struct check_true<true> : test_passed {};
    
#pragma GCC diagnostic push
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
    auto handle_test = [](auto shouldFail) {
        static_assert(decltype(shouldFail)::passed,"assertion failed");
    };
#pragma clang diagnostic pop
#pragma GCC diagnostic pop

    
    template <bool x> using
    is_true = decltype(handle_test(check_true<x>{}));
    
    template <bool x> using
    is_false = decltype(handle_test(check_true<!x>{}));


    template <typename,typename> struct check_equal : test_failed {};
    template <typename A> struct check_equal<A,A> : test_passed {};

    template <typename A, typename B> using
    are_equal = decltype(handle_test(check_equal<A,B>{}));

    template <typename,typename> struct check_different : test_passed {};
    template <typename A> struct check_different<A,A> : test_failed {};

    template <typename A, typename B> using
    are_different = decltype(handle_test(check_different<A,B>{}));
};



TEST_CASE("Make simple ELF file","") {
    using intEqualInt = Assert::are_equal<int, int>;
    using intNotEqualDouble = Assert::are_different<int,double>;
    using isTrue = Assert::is_true<true>;
    using isFalse = Assert::is_false<false>;

    Elf32_Ehdr header;
    header.e_ident[EI_MAG0] = ELFMAG0;
    header.e_ident[EI_MAG1] = ELFMAG1;
    header.e_ident[EI_MAG2] = ELFMAG2;
    header.e_ident[EI_MAG3] = ELFMAG3;
    
    header.e_ident[EI_CLASS] = ELFCLASS32;
    header.e_ident[EI_DATA] = ELFDATA2LSB;
    header.e_ident[EI_VERSION] = ELFVERSION;
    
    for(size_t i = EI_PAD; i < EI_NIDENT; i++)
        header.e_ident[i] = 0u;
    
    
    header.e_machine = EMachine::EM_386;
    header.e_version = EVersion::EV_CURRENT;
    
    header.e_phoff = 0u;
    header.e_shoff = 0u;
    
    header.e_flags = 0u;
    
    header.e_ehsize = sizeof(Elf32_Ehdr);
    
    header.e_phentsize = sizeof(Elf32_Phdr);
    header.e_phnum = 1u; // one program header entry
    header.e_shentsize = 0u; // section header entry size
    header.e_shnum = 0u; // no section headers
    header.e_shstrndx = SHN_UNDEF;
   
    
    Elf32_Phdr phead;
    phead.p_type = EPType::PT_LOAD;
    phead.p_offset = 0u;
    phead.p_vaddr = 0u;
    phead.p_paddr = 0u;
    phead.p_filesz = sizeof(Elf32_Ehdr) + sizeof(Elf32_Ehdr) + 1024u;
    phead.p_memsz = phead.p_filesz;
    phead.p_flags = PF_R | PF_X;
    phead.p_align = 0x1000u;

    header.e_entry = sizeof(Elf32_Ehdr)+sizeof(Elf32_Phdr); // entry point
    
    
    unsigned char prog[6];
    prog[ 0]=0xb8;
    prog[ 1]=0x0d;
    prog[ 2]=0x00;
    prog[ 3]=0x00;
    prog[ 4]=0x00;
    prog[ 5]=0xc3;
    
    unsigned char exe[sizeof(Elf32_Ehdr)+sizeof(Elf32_Phdr)+sizeof(prog)];

    memcpy(&exe[0],&header,sizeof(Elf32_Ehdr));
    memcpy(&exe[sizeof(Elf32_Ehdr)],&phead,sizeof(Elf32_Phdr));

    memcpy(&exe[sizeof(Elf32_Ehdr)+sizeof(Elf32_Phdr)],&prog,sizeof(prog));
    
    std::ofstream output( "/Users/julian/test", std::ios::binary );
    std::copy( 
        std::begin(exe),
        std::end(exe),
        std::ostreambuf_iterator<char>(output));
}


