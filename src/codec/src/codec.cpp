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
#include "htm.h"
#include "htmsublayer.h"
#include "column.h"
#include "sensoryregion.h"
#include "sensoryinput.h"

// sensory codec factories
//#include "sensory_codec_factory.h"

// autoencoder headers
//#include "Autoencoder.h"

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

    motorLayer = NULL;
}

ElfCodec::~ElfCodec()
{
    if (targetPath) free(targetPath);
    if (codecName) free(codecName);
}

bool ElfCodec::Init(char *target_path, HtmSublayer *sensoryLayer)
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

    this->sensoryLayer = sensoryLayer;

    std::map<unsigned char, unsigned char *> dynSymsIdxMap;

    codecHeight = 144;//sensoryLayer->GetHeight();
    codecWidth = 144;//sensoryLayer->GetWidth();
    codecActiveRatio = sensoryLayer->GetLocalActivity();

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
                        dynSymsIdxMap[(unsigned char)i] =
                            (unsigned char *)func_name;
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
                    //printf("%s at 0x%08x\n", func_name, sym.st_value);
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
        // memory address), then find their plt entries.
        std::map<unsigned char, unsigned char *>::iterator dSymsIdxMapIt;
        std::map<unsigned int, unsigned char *> dynFuncRelocGotAddrs;
        std::map<unsigned int, unsigned char *>::iterator
            dFuncRelGotAddrsIt;
        for (unsigned int i=0; i<reloc_num; i++) {
            gelf_getrel(reloc_data, i, &reloc);
            dSymsIdxMapIt = dynSymsIdxMap.find(
                (unsigned char)(reloc.r_info>>32)
            );
            if (dSymsIdxMapIt != dynSymsIdxMap.end())
                dynFuncRelocGotAddrs[reloc.r_offset] = dSymsIdxMapIt->second;
            if ((unsigned char)(reloc.r_info>>32) == open_sym_ndx)
                open_got_offset = reloc.r_offset;
            if ((unsigned char)(reloc.r_info>>32) == read_sym_ndx)
                read_got_offset = reloc.r_offset;
        }
        for (unsigned int i=0; i<plt_num; i++) {
            void *pltent = plt_data->d_buf;
            unsigned int jmp_offset = 0x2+0x10*i;
            unsigned int jmp_instr =
                *(unsigned int *)((unsigned int)pltent+jmp_offset);
            //if (find(itBeg, itEnd, jmp_instr) != itEnd)
            dFuncRelGotAddrsIt = dynFuncRelocGotAddrs.find(
                jmp_instr
            );
            if (dFuncRelGotAddrsIt != dynFuncRelocGotAddrs.end()) {
                std::vector<unsigned int> pltGotVect;
                pltGotVect.push_back(plt_addr+jmp_offset-0x2);
                pltGotVect.push_back(jmp_instr);
                pltToGotMap[dFuncRelGotAddrsIt->second] =
                    pltGotVect;
            }
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
 * perform two operations on the running process:
 *
 * 1. For each local and dynamic function, store the binary
 *    representation and initialize the spatial pooling motor layer.
 * 2. Trace the child process until main() is called by
 *    _libc_start_main(), then pause execution.
 *
 * First, wait for child to stop on its first instruction. It will
 * be somewhere in glibc setup/init code.
 */
bool ElfCodec::LoadTarget()
{
    printf("[*] Forking/Execing target executable %s\n", targetPath);
    /*
     * in order to make it easier to obtain dynamically linked
     * function machine code, tell the dynamic linker to resolve
     * GOT entries for PLT functions during initialization.
     */
    setenv("LD_BIND_NOW", "1", 1);
    if (!(child_pid=fork())) {
        /*
         * replace child process image with new executable and
         * allow ptracing.
         */
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
            perror("ptrace");
            return false;
        }
        execl(targetPath, targetPath, 0);
    } else if (child_pid > 0) {
        wait(&wait_status);
        printf("[*] Running executable to main() function.\n");
        while (WIFSTOPPED(wait_status)) {
            struct user_regs_struct regs;
            std::vector<unsigned char> machCode;
            //std::map<unsigned char *,
                     //std::vector<unsigned char> > classifyMap;
            std::map<unsigned char *,
                     std::vector<unsigned char> > fcnStrCodeMap;
            /*
             * get the current register values, and stop tracing if
             * it has reached main().
             */
            ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
            if ((unsigned int)regs.eip == main_addr) {
                printf("[*] Encoding motor commands from function API.\n");
                /* obtain machine code of local functions. */
                for (auto i : localFuncMap) {
                    //printf("%s: %d bytes at 0x%08x\n",
                        //i.first, i.second[1], i.second[0]
                    //);
                    for (unsigned int j=0; j<i.second[1]; j++) {
                        machCode.push_back(ptrace(
                            PTRACE_PEEKTEXT, child_pid,
                            i.second[0]+j, 0
                        ));
                        //printf("%02x ", machCode[j]);
                    }
                    //if (machCode.size()>=0&&machCode.size()<=93) {
                    //if (i.second[0] == 0x080484a0) {
                        //printf("\tskipping\n");
                    //classifyMap[(unsigned char *)strdup(
                        //(const char *)i.first)] = machCode;
                        //machCode.clear();
                        //continue;
                    //}
                    fcnStrCodeMap[(unsigned char *)strdup(
                        (const char *)i.first)] =  machCode;
                    //}
                    fcnMachCodeMap[i.second[0]] = fcnStrCodeMap;
                    machCode.clear();
                    fcnStrCodeMap.clear();
                    //printf("\n");
                }
                /*
                 * obtain machine code of dynamically linked functions
                 * referenced in the GOT and called indirectly from the
                 * PLT.
                 */
                //printf("looping through plt to trace got addresses\n");
                for (auto iter : pltToGotMap) {
                    //printf("%s: ",iter.first);
                    unsigned int c=0;
                    unsigned int relocGotAddr = ptrace(
                        PTRACE_PEEKTEXT, child_pid,
                        iter.second[1], 0
                    );
                    //printf("0x%08x -> 0x%08x : ", iter.second[0],
                        //relocGotAddr);
                    do {
                        machCode.push_back(ptrace(
                            PTRACE_PEEKTEXT, child_pid,
                            relocGotAddr+c, 0
                        ));
                        c++;
                        //printf("%02x ", machCode.back());
                    } while (machCode.back() != RET_OPCODE);
                    //printf("\n");
                    //printf("%u bytes\n", machCode.size());
                    //if (machCode.size()>=0&&machCode.size()<=93) {
                    //classifyMap[(unsigned char *)strdup(
                        //(const char *)iter.first)] = machCode;
                    fcnStrCodeMap[(unsigned char *)strdup(
                        (const char *)iter.first)] = machCode;
                    //}
                    fcnMachCodeMap[iter.second[0]] = fcnStrCodeMap;
                    machCode.clear();
                    fcnStrCodeMap.clear();
                }

                /*
                 * normalize function machine code definitions by
                 * injecting NOPs into every function smaller than the
                 * largest. this is needed because of how the proximal
                 * dendrite is initialized in the spatial pooler.
                 */
                unsigned int targetDimension = FindFcnLargestSz();
                NormalizeFcnMachCodeMap(targetDimension);
                /*
                 * create mapping from motor function memory address
                 * to SensoryRegion of input bits using the
                 * normalized opcode array.
                 */
                unsigned int totalBits = targetDimension*8;
                codecHeight = (unsigned int)sqrt(totalBits);
                codecWidth = (unsigned int)sqrt(totalBits);
                unsigned int leftover =
                    totalBits - (codecHeight * codecWidth);
                TranslateOpcodeArrayToSensoryInput(
                    codecWidth,
                    codecHeight,
                    leftover
                );

                // initialize the spatial pooling motor layer of
                // columns
                codecRfSz = sensoryLayer->GetRecFieldSz();
                codecActiveRatio = sensoryLayer->GetLocalActivity();
                codecColComplexity = sensoryLayer->GetColComplexity();

                motorLayer = new HtmSublayer(
                    codecHeight,
                    codecWidth,
                    1, NULL, false
                );
                motorLayer->AllocateColumns(
                    codecRfSz,
                    codecActiveRatio,
                    codecColComplexity,
                    false,
                    100
                );
                // initialize an "empty" set of sensory input bits
                SensoryInput ***initbits = (SensoryInput ***)malloc(
                    sizeof(SensoryInput **) * codecHeight
                );
                for (unsigned int h=0; h<codecHeight; h++) {
                    initbits[h] = (SensoryInput **)malloc(
                        sizeof(SensoryInput *) * codecWidth
                    );
                    for (unsigned int w=0; w<codecWidth; w++)
                        initbits[h][w] = new SensoryInput(w, h);
                }
                // initialize an "empty" motor input region based
                // on empty set of input bits.
                SensoryRegion *initPatt = new SensoryRegion(
                    initbits, 0, codecWidth, codecHeight, 0, NULL
                );
                motorLayer->setlower(initPatt);
                motorLayer->InitializeProximalDendrites();

/*
                ae = new Autoencoder(fcnMachCodeMap);
                ae->Train(1);
                printf("AE trained.\n");
                for (auto p : classifyMap) {
                    std::map<unsigned char *, std::vector<unsigned char> > classifyEntry;
                    classifyEntry[p.first] = p.second;
                    ae->Classify(classifyEntry);
                    classifyEntry.clear();
                }
*/
                break;
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

unsigned int ElfCodec::FindFcnLargestSz()
{
    unsigned int highestSz = 0;
 
    for (auto f : fcnMachCodeMap)
        for (auto s : f.second)
            if (s.second.size() > highestSz)
                highestSz = s.second.size();
 
    return highestSz;
}

void ElfCodec::NormalizeFcnMachCodeMap(unsigned int numBytes)
{
    unsigned int bytePos, normBytePos, opCode;
    std::map<unsigned char *, std::vector<unsigned char> > normFcnMap;
    std::vector<unsigned char> normFcnVect;

    for (auto m=fcnMachCodeMap.begin(); m!=fcnMachCodeMap.end(); ++m) {
        bytePos = 0, normBytePos = 0;
        for (auto f : m->second) {
            while (normBytePos < numBytes) {
                if (f.second[bytePos]==RET_OPCODE&&normBytePos<numBytes-1)
                    opCode = NOP_OPCODE;
                else
                    opCode = f.second[bytePos++];
                normFcnVect.push_back(opCode);
                normBytePos++;
            }
            // replace the non-normalized vector with the new,
            // normalized one.
            normFcnMap[f.first] = normFcnVect;
            m->second = normFcnMap;
            normFcnMap.clear();
            normFcnVect.clear();
        }
    }
}

void ElfCodec::TranslateOpcodeArrayToSensoryInput(
    unsigned int width,
    unsigned int height,
    unsigned int leftover)
{
    std::map<unsigned char *, SensoryRegion *> sensoryMap;

    for (auto fcn : fcnMachCodeMap) {
        unsigned int numActiveInputs = 0;
        SensoryInput ***motorInputs = (SensoryInput ***)malloc(
            sizeof(SensoryInput **) * height
        );
        std::vector<unsigned char> opcodeArray =
            fcn.second.begin()->second;
        unsigned int totalBits = opcodeArray.size()*8;
        // adjust width and height with leftover?
        unsigned int bitPos = 0, bytePos = 0;
        for (unsigned int h=0; h<height; h++) {
            motorInputs[h] = (SensoryInput **)malloc(
                sizeof(SensoryInput *) * width
            );
            for (unsigned int w=0; w<width; w++, bitPos++) {
                unsigned int bit = bitPos%8;
                if (bitPos>0 && (bit==0||bitPos==totalBits-1)) {
                    if (bitPos==totalBits-1) {
                        //printf("%d",
                            //((1<<bit)&opcodeArray[bytePos])>>bit);
                        motorInputs[h][w] = new SensoryInput(w, h);
                        motorInputs[h][w]->SetActive(
                            ((1<<bit)&opcodeArray[bytePos])>>bit
                        );
                        if (motorInputs[h][w]->IsActive())
                            numActiveInputs++;
                    }
                    //printf(" %02x\n", opcodeArray[bytePos]);
                    bytePos++;
                }
                if (bitPos < totalBits-1) {
                    //printf("%d",
                        //((1<<bit)&opcodeArray[bytePos])>>bit);
                    motorInputs[h][w] = new SensoryInput(w, h);
                    motorInputs[h][w]->SetActive(
                        ((1<<bit)&opcodeArray[bytePos])>>bit
                    );
                    if (motorInputs[h][w]->IsActive())
                        numActiveInputs++;
                }
            }
        }
        // account for leftover next?

        sensoryMap[fcn.second.begin()->first] = new SensoryRegion(
            motorInputs,
            numActiveInputs,
            width, height,
            0, NULL
        );
        motorEncodingMap[fcn.first] = sensoryMap;
        sensoryMap.clear();
    }
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
    unsigned int senseAddr, cpgAddr;
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
        //printf("\t\tpure sensory function\n");
        binding = HandlePureSensory(&regs);
        inputpattern = binding.codec->GetPattern(
            binding.fd, Learning
        );
    } else if (std::find(cpg_b, cpg_e, call_addr) != cpg_e) {
        //printf("\t\tcpg function\n");
    } else if (std::find(smotor_b, smotor_e, call_addr) != smotor_e) {
        //printf("\t\tsensorimotor function.\n");
        // step through code until motor function call.
        cpgAddr = ExecuteToCall(cpgFunctions, &regs);

        // step through code until sensory function call.
        senseAddr = ExecuteToCall(pureSensoryFunctions, &regs);

        // duplicate open fd and determine corresponding sensory
        // codec with which to obtain sensory pattern.
        binding = HandlePureSensory(&regs);
        inputpattern = binding.codec->GetPattern(
            binding.fd, Learning
        );

        // obtain the associated motor command pattern.
        // ~~ deprecated method ~~
        //inputpattern->SetMotorPattern(
        //    motorCommandEncodings[cpgAddr]
        //);
        // ~~ SP method ~~
        SensoryRegion *motorPattern =
            motorEncodingMap[cpgAddr].begin()->second;
        motorLayer->setlower(motorPattern);
        motorLayer->RefreshLowerSynapses();

        /*
         * Disable learning and boosting. I don't want the motor
         * pattern spatial pooler to modify its synapses or ever
         * alter its representation.
         *
         * Learning=false, AllowBoosting=false
         */
        motorLayer->SpatialPooler(false, false);
        SensoryRegion *sparseMotorPattern = GenerateSparseMotorRep(
            motorLayer->GetInput(),
            motorLayer->GetWidth(),
            motorLayer->GetHeight()
        );
        inputpattern->SetMotorPattern(sparseMotorPattern);
    } else {
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

SensoryRegion* ElfCodec::GenerateSparseMotorRep(
    Column ***cols, int width, int height)
{
    int numActive = 0;
    SensoryRegion *motorPattern = NULL;
    SensoryInput ***motorInputs = (SensoryInput ***)malloc(
        sizeof(SensoryInput **) * height
    );

    for (unsigned int h=0; h<height; h++) {
        motorInputs[h] = (SensoryInput **)malloc(
            sizeof(SensoryInput *) * width
        );
        for (unsigned int w=0; w<width; w++) {
            motorInputs[h][w] = new SensoryInput(w, h);
            numActive += (int)cols[h][w]->IsActive();
            motorInputs[h][w]->SetActive(cols[h][w]->IsActive());
        }
    }

    motorPattern = new SensoryRegion(
        motorInputs, numActive, width, height, 0, NULL
    );

    return motorPattern;
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

