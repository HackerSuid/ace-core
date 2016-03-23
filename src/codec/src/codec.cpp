#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// ptrace headers
#include <sys/wait.h>
#include <sys/user.h>

// libelf headers
#include <libelf.h>
#include <gelf.h>
#include <err.h>
#include <sysexits.h>

// libhtm headers
#include "codec.h"
#include "htmregion.h"
#include "sensoryregion.h"
#include "sensoryinput.h"

// Register with the base Codec class.
bool const SensoryCodec::registered = Codec::Register<SensoryCodec>();

SensoryCodec::SensoryCodec()
    : Codec()
{
    pidx = 0;
    codecName = strdup("Sensory");
}

SensoryCodec::~SensoryCodec()
{
    if (TargetPath) free(TargetPath);
    if (codecName) free(codecName);
}

bool SensoryCodec::Init(char *target_path)
{
    Elf *e;
    GElf_Ehdr ehdr;
    GElf_Phdr phdr;
    GElf_Shdr shdr;
    GElf_Sym sym;
    GElf_Rel reloc;
    Elf_Scn *scn;
    Elf_Data *symtab_data=NULL, *dynsym_data=NULL;
    Elf_Data *reloc_data=NULL, *plt_data=NULL;
    Elf_Data *gotplt_data=NULL;
    int wait_status, elf_fd, elf_class;
    char *ident, *name, *func_name;
    size_t shnum, phnum, shstrndx;
    size_t symtab_num, dynsym_num, reloc_num;
    size_t plt_num, gotplt_num;

    unsigned char read_sym_ndx;
    unsigned int plt_addr;
    unsigned int read_got_offset, read_plt;

    if (elf_version(EV_CURRENT) == EV_NONE)
        errx(
            EX_SOFTWARE,
            "libelf initialization failed: %s",
            elf_errmsg(-1)
        );

    if ((elf_fd = open(target_path, O_RDONLY, 0)) < 0)
        err(EX_NOINPUT, "open %s failed.", target_path);

    if ((e = elf_begin(elf_fd, ELF_C_READ, NULL)) == NULL)
        errx(
            EX_SOFTWARE,
            "elf_begin() failed: %s\n",
            elf_errmsg(-1)
        );

    if (elf_kind(e) == ELF_K_ELF) {
        // parse ELF executable header
        if (gelf_getehdr(e, &ehdr) == NULL)
            errx(
                EX_SOFTWARE,
                "gelf_getehdr() failed: %s",
                elf_errmsg(-1)
            );
        // 32 or 64 bit.
        if ((elf_class = gelf_getclass(e)) == ELFCLASSNONE)
            errx(
                EX_SOFTWARE,
                "gelf_getclass() failed: %s",
                elf_errmsg(-1)
            );
        //printf("%d-bit ELF object\n", elf_class==ELFCLASS32? 32 : 64);
        if ((ident = elf_getident(e, NULL)) == NULL)
            errx(
                EX_SOFTWARE,
                "elf_getident() failed: %s",
                elf_errmsg(-1)
            );
     /*
        printf("e_type 0x%x\n", ehdr.e_type);
        printf("e_machine 0x%x\n", ehdr.e_machine);
        printf("e_version 0x%x\n", ehdr.e_version);
        printf("e_entry 0x%x\n", ehdr.e_entry);
        printf("e_phoff 0x%x\n", ehdr.e_phoff);
        printf("e_shoff 0x%x\n", ehdr.e_shoff);
        printf("e_flags 0x%x\n", ehdr.e_flags);
        printf("e_ehsize 0x%x\n", ehdr.e_ehsize);
        printf("e_phentsize 0x%x\n", ehdr.e_phentsize);
        printf("e_shentsize 0x%x\n", ehdr.e_shentsize);
      */
        if (elf_getphdrnum(e, &phnum) !=0)
            errx(
                EX_SOFTWARE,
                "elf_getphdrnum() failed: %s",
                elf_errmsg(-1)
            );
        if (elf_getshdrnum(e, &shnum) != 0)
            errx(
                EX_SOFTWARE,
                "elf_getshdrnum() failed: %s",
                elf_errmsg(-1)
            );
        if (elf_getshstrndx(e, &shstrndx) != 0)
            errx(
                EX_SOFTWARE,
                "elf_getshstrndx() failed: %s",
                elf_errmsg(-1)
            );

        /*
         * parse the program header table
         *
         *   typedef struct
         *   {
         *     Elf32_Word    p_type;     // Segment type
         *     Elf32_Off     p_offset;   // Segment file offset
         *     Elf32_Addr    p_vaddr;    // Segment virtual address
         *     Elf32_Addr    p_paddr;    // Segment physical address
         *     Elf32_Word    p_filesz;   // Segment size in file
         *     Elf32_Word    p_memsz;    // Segment size in memory
         *     Elf32_Word    p_flags;    // Segment flags
         *     Elf32_Word    p_align;    // Segment alignment
         *   } Elf32_Phdr;
        for (int i=0; i<phnum; i++) {
            if (gelf_getphdr(e, i, &phdr) != &phdr)
                errx(
                    EX_SOFTWARE,
                    "gelf_getphdr() failed: %s",
                    elf_errmsg(-1)
                );
            printf("PHdr %d:\n", i);
            printf("\tp_type 0x%x (%s)\n", phdr.p_type,
                ptype_str(phdr.p_type));
            printf("\tp_offset 0x%x\n", phdr.p_offset);
            printf("\tp_vaddr 0x%x\n", phdr.p_vaddr);
            printf("\tp_paddr 0x%x\n", phdr.p_paddr);
            printf("\tp_filesz 0x%x\n", phdr.p_filesz);
            printf("\tp_memsz 0x%x\n", phdr.p_memsz);
            printf("\tp_flags 0x%x\n", phdr.p_flags);
            printf("\tp_align 0x%x\n", phdr.p_align);
        }
         */

        /*
         * parse the section header table
         *
         *   typedef struct
         *   {
         *     Elf32_Word    sh_name;        // Section name (string tbl index)
         *     Elf32_Word    sh_type;        // Section type
         *     Elf32_Word    sh_flags;       // Section flags
         *     Elf32_Addr    sh_addr;        // Section virtual addr at execution
         *     Elf32_Off     sh_offset;      // Section file offset
         *     Elf32_Word    sh_size;        // Section size in bytes
         *     Elf32_Word    sh_link;        // Link to another section
         *     Elf32_Word    sh_info;        // Additional section information
         *     Elf32_Word    sh_addralign;   // Section alignment
         *     Elf32_Word    sh_entsize;     // Entry size if section holds table
         *   } Elf32_Shdr;
         */
        scn = NULL;
        // Find the plt section and its associated relocation table
        // section (.plt and .rel.plt), as well as the global offset table
        // for function symbol definitions (.got.plt).
        while ((scn = elf_nextscn(e, scn)) != NULL) {
            // get section table header.
            if (gelf_getshdr(scn, &shdr) != &shdr)
                errx(
                    EX_SOFTWARE,
                    "gelf_getshdr() failed: %s",
                    elf_errmsg(-1)
                );
            // resolve the section name string.
            if ((name = elf_strptr(e, shstrndx, shdr.sh_name)) == NULL)
                errx(
                    EX_SOFTWARE,
                    "elf_strptr() failed: %s",
                    elf_errmsg(-1)
                );

            //printf("Section %d: %s\n", elf_ndxscn(scn), name);
            if (shdr.sh_type == SHT_REL &&
                !strcmp(".rel.plt", name)) {
                reloc_data = elf_getdata(scn, reloc_data);
                reloc_num = shdr.sh_size/shdr.sh_entsize;
            }
            if (shdr.sh_type == SHT_PROGBITS &&
                !strcmp(".plt", name)) {
                plt_data = elf_getdata(scn, plt_data);
                plt_num = plt_data->d_size/0x10;
                plt_addr = shdr.sh_addr;
            }
            if (shdr.sh_type == SHT_PROGBITS &&
                !strcmp(".got.plt", name)) {
                gotplt_data = elf_getdata(scn, gotplt_data);
                gotplt_num = gotplt_data->d_size/0x4;
            }
            if (shdr.sh_type == SHT_DYNSYM) {
                // obtain the .dynsyn dynamic linking symbol table
                dynsym_data = elf_getdata(scn, dynsym_data);
                dynsym_num = shdr.sh_size/shdr.sh_entsize;
                for (int i=0; i<dynsym_num; i++) {
                    gelf_getsym(dynsym_data, i, &sym);
                    /*
                    printf("st_name 0x%08x ", sym.st_name);
                    printf("st_value 0x%016x ", sym.st_value);
                    printf("st_size 0x%016x ", sym.st_size);
                    printf("st_info 0x%02x ", sym.st_info);
                    printf("st_other 0x%02x ", sym.st_other);
                    printf("st_shndx 0x%04x\n", sym.st_shndx);
                    */
            
                    if (GELF_ST_TYPE(sym.st_info)==STT_FUNC) {
                        func_name = elf_strptr(e, shdr.sh_link, sym.st_name);
                        if (!strncmp("read", func_name, 4))
                            read_sym_ndx = (unsigned char)i;
                    }
                }
            }
            
            /*
            if (shdr.sh_type == SHT_SYMTAB) {
                // obtain the .symtab symbol table
                symtab_data = elf_getdata(scn, symtab_data);
                symtab_num = shdr.sh_size/shdr.sh_entsize;
                for (int i=0; i<symtab_num; i++) {
                    gelf_getsym(symtab_data, i, &sym);
            */
                    /*
                    printf("st_name 0x%08x ", sym.st_name);
                    printf("st_value 0x%016x ", sym.st_value);
                    printf("st_size 0x%016x ", sym.st_size);
                    printf("st_info 0x%02x ", sym.st_info);
                    printf("st_other 0x%02x ", sym.st_other);
                    printf("st_shndx 0x%04x\n", sym.st_shndx);
                    */
            /*
                    if (GELF_ST_TYPE(sym.st_info)==STT_FUNC)
                        printf("\t%s\n",
                            elf_strptr(e, shdr.sh_link, sym.st_name));
                }
            }
            */
            /*if (shdr.sh_type == SHT_STRTAB) {
                dynsym_data = elf_getdata(scn, dynsym_data);
                dynsym_num = shdr.sh_size/shdr.sh_entsize;
                for (int i=0; i<dynsym_num; i++) {
                    gelf_getsym(dynsym_data, i, &dyn_sym);
                    printf("%x (%x %x)\n", dyn_sym.st_info, dyn_sym.st_info&0x0000000f,
                        GELF_ST_BIND(dyn_sym.st_info));
                }
            }*/
        }
        // find the relocation table entry for the read function symbol
        // and save the relocation offset (virtual memory address)
        for (int i=0; i<reloc_num; i++) {
            gelf_getrel(reloc_data, i, &reloc);
            if ((unsigned char)(reloc.r_info>>32) == read_sym_ndx)
                read_got_offset = reloc.r_offset;
        }
        // find the read@plt entry in the plt by finding the above
        // jmp offset
        for (int i=0; i<plt_num; i++) {
            void *pltent = plt_data->d_buf;
            unsigned int jmp_offset = 0x2+0x10*i;
            unsigned int jmp_instr =
                *(unsigned int *)((unsigned int)pltent+jmp_offset);
            if (jmp_instr == read_got_offset) {
                read_plt = plt_addr+jmp_offset-0x2;
                //printf("read@plt = 0x%08x\n", read_plt);
                break;
            }
        }
    } else {
        fprintf(stderr, "target %s not an ELF object.\n", target_path);
        return false;
    }

    elf_end(e);
    close(elf_fd);

    if (!(child_pid=fork())) {
        // replace child process image with new executable and
        // allow ptracing.
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
            perror("ptrace");
            return false;
        }
        execl(target_path, target_path, 0);
    } else if (child_pid > 0) {
        // ACE will trace the child process until main().

        // Wait for child to stop on its first instruction. It will
        // be somewhere in glibc setup/init code.
        wait(&wait_status);
        printf("child process execl()ed\n");

        while (WIFSTOPPED(wait_status)) {
            struct user_regs_struct regs;
            ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
            unsigned int xi =
                ptrace(PTRACE_PEEKTEXT, child_pid, regs.eip, 0);
            // catch any software interrupts to make system calls
            //if ((xi & 0xFFFF) == SOFT_INT_OPCODE)
            //    printf("0x%08x\n", xi);
            if ((uint8_t)(xi & 0x000000FF) == CALL_OPCODE) {
                void *rel_off_addr = (void *)(regs.eip+1);
                int rel_off =
                    ptrace(PTRACE_PEEKTEXT, child_pid, rel_off_addr, 0);
                unsigned int call_addr =
                    (unsigned int)(rel_off_addr)+sizeof(unsigned int)+rel_off;
                //printf("call 0x%08x\n", call_addr);
            }
            // Make the child execute another instruction
            if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
                perror("ptrace");
                return false;
            }

            // Wait for child to stop on its next instruction
            wait(&wait_status);
        }
    } else {
        perror("fork");
        return false;
    }

    this->TargetPath = strdup(target_path);

    return true;
}

// map program header type code to string representation
char* SensoryCodec::ptype_str(size_t pt)
{
    char *s;
#define C(V) case  PT_##V: s = #V; break
    switch (pt) {
        C(NULL);    C(LOAD);        C(DYNAMIC);
        C(INTERP);  C(NOTE);        C(SHLIB);
        C(PHDR);    C(TLS);         C(SUNWBSS);
        C(SUNWSTACK);
        default:
            s = "unknown";
        break;
    }
    return s;
#undef   C
}

// Take a bitmap file and convert to an SensoryRegion
SensoryRegion* SensoryCodec::GetPattern()
{
    BITMAPHEADER *bmh;
    FILE *fp = NULL;
    char filename[128];
    Pixel rgb;
    SensoryRegion *pattern=NULL;
    SensoryInput ***input=NULL;

    snprintf(filename, sizeof(filename), "%s/%d.bmp", TargetPath, pidx++);
    if ((bmh = ReadBitmapHeader(filename, &fp)) == NULL)
        return NULL;

    // read each pixel value and map to a bit vector
    //   black => 0x00 (active)
    //   white => 0xFF (inactive)
    input = (SensoryInput ***)malloc(sizeof(SensoryInput **) * bmh->bmih.height);
    for (int i=0; i<bmh->bmih.height; i++) {
        // flip the image to display correctly on the unit grid.
        input[(bmh->bmih.height-1)-i] = (SensoryInput **)malloc(sizeof(SensoryInput *) * bmh->bmih.width);
        for (int j=0; j<bmh->bmih.width; j++) {
            fread(&rgb, sizeof(rgb), 1, fp);
            input[(bmh->bmih.height-1)-i][j] = new SensoryInput;
            input[(bmh->bmih.height-1)-i][j]->SetActive(!(rgb.r|rgb.g|rgb.b));
        }
    }
    fclose(fp);

    // input patterns will have zero cells.
    pattern = new SensoryRegion(filename, input, bmh->bmih.width, bmh->bmih.height, 0);

    return pattern;
}

// whether or not the codec is processing the first pattern of a pre-determined
// sequence.
bool SensoryCodec::FirstPattern(SensoryRegion *InputPattern)
{
    char *filename = InputPattern->GetFilename();

    if (strchr(filename, '/'))
        filename = strrchr(filename, '/')+1;

    char *ext = strrchr(filename, '.');
    ext = 0;

    int patt_idx = atoi(filename);
    
    return patt_idx? false : true;
}

void SensoryCodec::Reset()
{
    pidx = 0;
}

BITMAPHEADER* SensoryCodec::ReadBitmapHeader(const char *filename, FILE **fp)
{
    BITMAPHEADER *bmh = (BITMAPHEADER *)malloc(sizeof(BITMAPHEADER));

    if ((*fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "%s: %s\n", filename, strerror(errno));
        return NULL;
    }

    fread((BITMAPFILEHEADER *)&(bmh->bmfh), sizeof(BITMAPFILEHEADER), 1, *fp);
    fread((BITMAPINFOHEADER *)&(bmh->bmih), sizeof(BITMAPINFOHEADER), 1, *fp);

    return bmh;
}

