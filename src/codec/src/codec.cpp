#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>

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

// sensory codec factories
//#include "sensory_codec_factory.h"

// autoencoder headers
#include "Autoencoder.h"

// Register with the base Codec class.
bool const ElfCodec::registered = Codec::Register<ElfCodec>();

ElfCodec::ElfCodec()
    : Codec()
{
    pidx = 0;
    main_addr = 0;
    open_plt = 0;
    read_plt = 0;
    codecName = strdup("Elf");
    child_dir = NULL;
    firstPattern = true;

    sensoryCodecFactory.Register((char *)".bmp", new BitmapCodec());
}

ElfCodec::~ElfCodec()
{
    if (targetPath) free(targetPath);
    if (codecName) free(codecName);
}

bool ElfCodec::Init(
    char *target_path,
    unsigned int height,
    unsigned int width,
    float localActivity)
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
    Elf_Data *pure_sensory_data=NULL, *sensorimotor_data=NULL;
    Elf_Data *cpg_data=NULL;
    int elf_fd, elf_class;
    char *ident, *name, *func_name;
    size_t shnum, phnum, shstrndx, pure_sensory_ndx, sensorimotor_ndx;
    size_t cpg_ndx;
    size_t symtab_num, dynsym_num, reloc_num;
    size_t plt_num, gotplt_num;

    unsigned char read_sym_ndx, open_sym_ndx, strtab_ndx;
    unsigned int plt_addr;
    unsigned int read_got_offset, open_got_offset;

    std::vector<unsigned char> dynSymsIdx;

    codecHeight = height;
    codecWidth = width;
    codecActiveRatio = localActivity;

    masterMotorEncoding = (SensoryInput ***)malloc(
        sizeof(SensoryInput **) * codecHeight
    );
    // instantiate the master motor encoding.
    for (unsigned int i=0; i<codecHeight; i++) {
        masterMotorEncoding[i] = (SensoryInput **)malloc(
            sizeof(SensoryInput *) * codecWidth
        );
        for (unsigned int j=0; j<codecWidth; j++)
            masterMotorEncoding[i][j] = new SensoryInput(j, i);
    }

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
         * parse the program header table. don't actually need any
         * phdr info, but leaving the code here for educational
         * purposes.
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
         */

      /*
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
        /*
         * 1. Find the plt section and its associated relocation table
         *    section (.plt and .rel.plt), as well as the global offset table
         *    for function symbol definitions (.got.plt).
         * 2. Find the custom ELF sections created by the subcortical
         *    ACE module which represent the function API to learn.
         */
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

            if (shdr.sh_type == SHT_REL &&
                !strcmp(".rel.plt", name)) {
                reloc_data = elf_getdata(scn, reloc_data);
                reloc_num = shdr.sh_size/shdr.sh_entsize;
            }
            if (shdr.sh_type == SHT_PROGBITS) {
                if (!strcmp(".plt", name)) {
                    plt_data = elf_getdata(scn, plt_data);
                    plt_num = plt_data->d_size/0x10;
                    plt_addr = shdr.sh_addr;
                }
                if (!strcmp(".got.plt", name)) {
                    gotplt_data = elf_getdata(scn, gotplt_data);
                    gotplt_num = gotplt_data->d_size/0x4;
                }
                if (!strcmp(".pure_sensory", name)) {
                    pure_sensory_data = elf_getdata(scn, pure_sensory_data);
                    pure_sensory_ndx = elf_ndxscn(scn);
                }
                if (!strcmp(".cpg", name)) {
                    cpg_data = elf_getdata(scn, cpg_data);
                    cpg_ndx = elf_ndxscn(scn);
                }
                if (!strcmp(".sensorimotor", name)) {
                    sensorimotor_data = elf_getdata(scn, sensorimotor_data);
                    sensorimotor_ndx = elf_ndxscn(scn);
                }
            }
            if (shdr.sh_type == SHT_DYNSYM) {
                // obtain the .dynsyn dynamic linking symbol table
                dynsym_data = elf_getdata(scn, dynsym_data);
                dynsym_num = shdr.sh_size/shdr.sh_entsize;
                for (unsigned int i=0; i<dynsym_num; i++) {
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
                        dynSymsIdx.push_back((unsigned char)i);
                        if (!strcmp("read", func_name))
                            read_sym_ndx = (unsigned char)i;
                        if (!strcmp("open", func_name))
                            open_sym_ndx = (unsigned char)i;
                    }
                }
            }
            
            if (shdr.sh_type == SHT_SYMTAB) {
                // obtain the .symtab symbol table
                symtab_data = elf_getdata(scn, symtab_data);
                symtab_num = shdr.sh_size/shdr.sh_entsize;
            }
            if (shdr.sh_type == SHT_STRTAB) {
                strtab_ndx = elf_ndxscn(scn);
            }
        }

        /*
         * Pure Sensory functions
         *
         * find subcortical functions that passively receive sensory patterns
         * without generating motor behavior. these are functions that call
         * open() and read() on the sensory fd, and optionally alter their own
         * internal state by performing local operations. these are not
         * very biologically realistic.
         *
         * Sensorimotor functions
         *
         * find subcortical functions that actively generate sensory patterns
         * through motor behavior. these are functions that execute a motor
         * function through an analogue of a central pattern generator (CPG),
         * and then call open() and read() on the sensory fd. they may also
         * alter their internal state.
         *
         * Find main() function symbol,
         */
        for (unsigned int i=0; i<symtab_num; i++) {
            gelf_getsym(symtab_data, i, &sym);
            /*
            printf("st_name 0x%08x ", sym.st_name);
            printf("st_value 0x%016x ", sym.st_value);
            printf("st_size 0x%016x ", sym.st_size);
            printf("st_info 0x%02x ", sym.st_info);
            printf("st_other 0x%02x ", sym.st_other);
            printf("st_shndx 0x%04x\n", sym.st_shndx);
            */
            if (GELF_ST_TYPE(sym.st_info)==STT_FUNC) {
                func_name = elf_strptr(
                    e, strtab_ndx, sym.st_name
                );
                std::vector<unsigned int> locFuncSym;
                if (sym.st_size > 0) {
                    printf("%s at 0x%08x\n", func_name, sym.st_value);
                    locFuncSym.push_back(sym.st_value);
                    locFuncSym.push_back(sym.st_size);
                    localFuncMap[(unsigned char *)func_name] = locFuncSym;
                    locFuncSym.clear();
                }
                if (!strncmp("main", func_name, 4))
                    main_addr = sym.st_value;
                // check if function is pure sensory or sensorimotor
                if (sym.st_shndx == pure_sensory_ndx)
                    pureSensoryFunctions.push_back(sym.st_value);
                if (sym.st_shndx == cpg_ndx) {
                    cpgFunctions.push_back(sym.st_value);
                    AddNewMotorEncoding(sym.st_value);
                }
                if (sym.st_shndx == sensorimotor_ndx)
                    sensorimotorFunctions.push_back(sym.st_value);
            }
        }
        // find the relocation table entry for the open and read
        // function symbols and save the relocation offset (virtual
        // memory address)
        std::vector<unsigned int> dynFuncRelocGotAddrs;
        for (unsigned int i=0; i<reloc_num; i++) {
            gelf_getrel(reloc_data, i, &reloc);
            std::vector<unsigned char>::iterator itBeg =
                dynSymsIdx.begin();
            std::vector<unsigned char>::iterator itEnd =
                dynSymsIdx.end();
            if (find(itBeg, itEnd, (unsigned char)(reloc.r_info>>32)) != itEnd)
                dynFuncRelocGotAddrs.push_back(reloc.r_offset);
            if ((unsigned char)(reloc.r_info>>32) == open_sym_ndx)
                open_got_offset = reloc.r_offset;
            if ((unsigned char)(reloc.r_info>>32) == read_sym_ndx)
                read_got_offset = reloc.r_offset;
        }
        // find the read@plt and open@plt entries in the plt by
        // finding the above jmp offsets.
        for (int i=0; i<plt_num; i++) {
            void *pltent = plt_data->d_buf;
            unsigned int jmp_offset = 0x2+0x10*i;
            unsigned int jmp_instr =
                *(unsigned int *)((unsigned int)pltent+jmp_offset);
            std::vector<unsigned int>::iterator itBeg =
                dynFuncRelocGotAddrs.begin();
            std::vector<unsigned int>::iterator itEnd =
                dynFuncRelocGotAddrs.end();
            if (find(itBeg, itEnd, jmp_instr) != itEnd)
                pltToGotMap[plt_addr+jmp_offset-0x2] = jmp_instr;
            if (jmp_instr == open_got_offset)
                open_plt = plt_addr+jmp_offset-0x2;
            if (jmp_instr == read_got_offset)
                read_plt = plt_addr+jmp_offset-0x2;
        }
    } else {
        fprintf(stderr, "target %s not an ELF object.\n", target_path);
        return false;
    }

    elf_end(e);
    close(elf_fd);

    this->targetPath = strdup(target_path);
    if (!LoadTarget())
        return false;

    return true;
}

// map program header type code to string representation
char* ElfCodec::ptype_str(size_t pt)
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

/*
 * ElfCodec loads the target executable by calling fork() and
 * execl() to spawn it as a child process, then ptrace() is used to
 *  perform two operations on the running process:
 * 1. For each local and dynamic function, store the binary
 *    representation to train the machine code autoencoder.
 * 2. Trace the child process until main() is called by
 *    _libc_start_main(), then pause execution.
 * First, wait for child to stop on its first instruction.
 * It will be somewhere in glibc setup/init code.
 */
bool ElfCodec::LoadTarget()
{
    std::vector< std::vector<unsigned char> > fcnMachCode;

    printf("[*] Forking/Execing target executable %s\n", targetPath);
    // in order to make it easier to obtain dynamically linked
    // function machine code, tell the dynamic linker to resolve
    // GOT entries for PLT functions during initialization.
    setenv("LD_BIND_NOW", "1", 1);
    if (!(child_pid=fork())) {
        // replace child process image with new executable and
        // allow ptracing.
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
            perror("ptrace");
            return false;
        }
        execl(targetPath, targetPath, 0);
    } else if (child_pid > 0) {
        wait(&wait_status);
        printf("[*] Running executable to main() entry.\n");
        while (WIFSTOPPED(wait_status)) {
            struct user_regs_struct regs;
            // get the current register values
            ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
            // break on main().
            if ((unsigned int)regs.eip == main_addr) {
                printf("[*] Encoding motor commands from function API.\n");
                // obtain machine code from local functions.
                std::vector<unsigned char> machCode;
                std::vector<unsigned char> *testCode;
                for (auto i : localFuncMap) {
                    printf("%s: %d bytes at 0x%08x\n",
                        i.first, i.second[1], i.second[0]
                    );
                    for (unsigned int j=0; j<i.second[1]; j++) {
                        machCode.push_back(ptrace(
                            PTRACE_PEEKTEXT, child_pid,
                            i.second[0]+j, 0
                        ));
                        //printf("%02x ", machCode[j]);
                    }
                    //if (machCode.size()<=4) {
                        //printf("\tadding to list\n");
                    if (i.second[0] == 0x08048476) {
                        printf("\tskipping\n");
                        testCode = new std::vector<unsigned char>(machCode);
                        continue;
                    }
                        fcnMachCode.push_back(machCode);
                    //}
                    machCode.clear();
                    //printf("\n");
                }
                // obtain machine code of dynamically linked functions
                // referenced in the GOT and called indirectly from the
                // PLT.
                printf("looping through plt to trace got addresses\n");
                typedef std::map<unsigned int, unsigned int>::iterator mapIt_t;
                for (mapIt_t iter=pltToGotMap.begin(); iter!=pltToGotMap.end(); iter++) {
                    printf("tracing 0x%08x => ", iter->second);
                    unsigned int c=0;
                    unsigned int relocGotAddr = ptrace(
                        PTRACE_PEEKTEXT, child_pid,
                        iter->second, 0
                    );
                    printf("0x%08x : ", relocGotAddr);
                    do {
                        machCode.push_back(ptrace(
                            PTRACE_PEEKTEXT, child_pid,
                            relocGotAddr+c, 0
                        ));
                        c++;
                        //printf("%02x ", machCode.back());
                    } while (machCode.back() != RET_OPCODE);
                    printf("%u bytes\n", machCode.size());
                    //if (machCode.size()<=4) {
                        //printf("\tadding to list\n");
                        fcnMachCode.push_back(machCode);
                    //}
                    machCode.clear();
                }
                ae = new Autoencoder(fcnMachCode);
                ae->Train(10);
                printf("AE trained.\n");
                ae->Classify(*testCode);
            }
            // Otherwise, continue; make the child execute another
            // instruction.
            if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
                perror("ptrace");
                return false;
            }
            // Wait for child to stop on its next instruction.
            wait(&wait_status);
        }
    } else {
        perror("fork");
        return false;
    }
    printf("[*] Target ready!\n");
    return true;
}

bool ElfCodec::Reset()
{
    // re-fork()/execl() the target executable.
    printf("[*] Reloading target: %s\n", targetPath);
    if (!LoadTarget()) {
        printf("Codec reset failed to reload target %s\n", targetPath);
        return false;
    }
    firstPattern = true;
    return true;
}

// single-step through executable's .text instructions until
// calling one of the addresses in the vector.
unsigned int ElfCodec::ExecuteToCall(
    std::vector<unsigned int> addrs,
    struct user_regs_struct *regs)
{
    if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
        perror("ptrace");
        return 0;
    }
    wait(&wait_status);
    while (WIFSTOPPED(wait_status)) {
        // get the current register values
        ptrace(PTRACE_GETREGS, child_pid, 0, regs);
        // get the current instruction being executed.
        unsigned int xi = ptrace(PTRACE_PEEKTEXT, child_pid, regs->eip, 0);
        if ((uint8_t)(xi & 0x000000FF) == CALL_OPCODE) {
            // get the address to the relative offset of the
            // address being called (located one byte after
            // current eip).
            void *rel_off_addr = (void *)(regs->eip+1);
            // get the relative offset.
            int rel_off =
                ptrace(PTRACE_PEEKTEXT, child_pid, rel_off_addr, 0);
            // compute the address being called using the relative
            // offset.
            unsigned int call_addr =
                (unsigned int)(rel_off_addr)+sizeof(unsigned int)+rel_off;
            std::vector<unsigned int>::iterator beg = addrs.begin();
            std::vector<unsigned int>::iterator end = addrs.end();
            if (std::find(beg, end, call_addr) != end)
                return call_addr;
        }
        // have child execute next instruction.
        if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
            perror("ptrace");
            return 0;
        }
        // Wait for child to stop on its next instruction.
        wait(&wait_status);
        if (WIFEXITED(wait_status)) {
            printf("child exited successfully. status %d\n",
                WEXITSTATUS(wait_status));
            return 0;
        }
    }
    return 0;
}

SensoryRegion* ElfCodec::GetPattern(bool Learning)
{
    long int fd = -1;
    struct user_regs_struct regs;
    unsigned int cpgAddr;
    SensoryCodecBinding binding;

    std::vector<unsigned int> allFunctions = pureSensoryFunctions;
    allFunctions.insert(
        allFunctions.end(),
        sensorimotorFunctions.begin(),
        sensorimotorFunctions.end()
    );
    unsigned int call_addr = ExecuteToCall(allFunctions, &regs);
    if (call_addr == 0)
        return NULL;
    std::vector<unsigned int>::iterator pure_b =
        pureSensoryFunctions.begin();
    std::vector<unsigned int>::iterator pure_e =
        pureSensoryFunctions.end();
    std::vector<unsigned int>::iterator cpg_b =
        cpgFunctions.begin();
    std::vector<unsigned int>::iterator cpg_e =
        cpgFunctions.end();
    std::vector<unsigned int>::iterator smotor_b =
        sensorimotorFunctions.begin();
    std::vector<unsigned int>::iterator smotor_e =
        sensorimotorFunctions.end();
    SensoryRegion *inputpattern = NULL;
    if (std::find(pure_b, pure_e, call_addr) != pure_e) {
        //printf("pure sensory function\n");
        binding = HandlePureSensory(&regs);
        inputpattern = binding.codec->GetPattern(
            binding.fd, Learning
        );
    } else if (std::find(cpg_b, cpg_e, call_addr) != cpg_e) {
        //printf("cpg function\n");
    } else if (std::find(smotor_b, smotor_e, call_addr) != smotor_e) {
        //printf("sensorimotor function.\n");
        cpgAddr = ExecuteToCall(cpgFunctions, &regs);
        call_addr = ExecuteToCall(pureSensoryFunctions, &regs);
        binding = HandlePureSensory(&regs);
        inputpattern = binding.codec->GetPattern(
            binding.fd, Learning
        );
        inputpattern->SetMotorPattern(
            motorCommandEncodings[cpgAddr]
        );
    } else {
        // this shouldn't be able to happen.
        fprintf(stderr, "[X] Error: 0x%08x not marked as an ACE" \
                " function\n", call_addr);
        return NULL;
    }

    close(binding.fd);

    if (Learning && firstPattern)
        firstPattern = false;

    return inputpattern;
}

SensoryCodecBinding ElfCodec::HandlePureSensory(
    struct user_regs_struct *regs
) {
    std::vector<unsigned int> open_read_vect;
    unsigned int call_addr;
    unsigned int open_path, mode;
    char *path_str = (char *)malloc(sizeof(char)*128);
    unsigned char c;
    int fd, j=0;
    SensoryCodecBinding bind;

    open_read_vect.push_back(open_plt);
    call_addr = ExecuteToCall(open_read_vect, regs);
    open_path =
        ptrace(PTRACE_PEEKTEXT, child_pid, regs->esp);
    do {
        c = ptrace(PTRACE_PEEKTEXT, child_pid, open_path+j);
        path_str[j] = c;
        j++;
    } while (c != 0x00);
    path_str[j] = 0;
    mode = ptrace(PTRACE_PEEKTEXT, child_pid, regs->esp+0x4);
    //printf("open(%s, %d)\n", path_str, mode);
    fd = open(path_str, mode);
    bind.fd = fd;
    bind.codec = sensoryCodecFactory.Get(
        (const char *)strrchr((char *)path_str, '.')
    );
    open_read_vect.clear();
    open_read_vect.push_back(read_plt);
    call_addr = ExecuteToCall(open_read_vect, regs);

    free(path_str);
    return bind;
}

// Encode the function address of the motor command as a random, unique
// pattern in a SensoryRegion
void ElfCodec::AddNewMotorEncoding(unsigned int motorCallAddr)
{
    SensoryInput ***motorInputs=(SensoryInput ***)malloc(
        sizeof(SensoryInput **) * codecHeight
    );
    int numActiveMotorInputs = 0;
    unsigned int codecArea = codecHeight * codecWidth;
    unsigned int numActiveBits = codecArea * codecActiveRatio;

    for (unsigned int i=0; i<codecHeight; i++) {
        motorInputs[i] = (SensoryInput **)malloc(
            sizeof(SensoryInput *) * codecWidth
        );
        for (unsigned int j=0; j<codecWidth; j++) {
            motorInputs[i][j] = new SensoryInput(j, i);
            // check that this bit is not already set in another encoding
            // within the master encoding union pattern. if it is not, randomly
            // set it to inactive or active.
            if (masterMotorEncoding[i][j]->IsActive())
                motorInputs[i][j]->SetActive(0);
            else {
                bool rndActive =
                    ((rand() % (codecArea+1)) <= numActiveBits) ? 1 : 0;
                motorInputs[i][j]->SetActive(rndActive);
                masterMotorEncoding[i][j]->SetActive(rndActive);
                if (rndActive)
                    numActiveMotorInputs++;
            }
        }
    }
    motorCommandEncodings[motorCallAddr] = new SensoryRegion(
        motorInputs, numActiveMotorInputs, codecWidth, codecHeight, 0, NULL
    );
}

int ElfCodec::GetRewardSignal()
{
    return 1;
}

// whether or not the codec is processing the first pattern of a pre-determined
// sequence.
bool ElfCodec::FirstPattern()
{
    return firstPattern;
}

