#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "disassembler.h"
#include "header.h"

const std::vector<std::string> PRINTABLE_SECTIONS = {".plt.got", ".plt.sec",
                                                     ".text", ".init", ".fini"};
const std::unordered_map<std::string, std::string> SECTION_LABEL_POSTFIX = {
    {".plt.got", "@plt"},
    {".plt.sec", "@plt"},
    {".text", ""},
    {".init", ""},
    {".fini", ""}};

inline void load(const std::string& binaryPath,
                 std::vector<unsigned char>& binaryBytes) {
    if (!binaryPath.empty()) {
        std::ifstream fh(binaryPath, std::ios::binary);
        if (fh) {
            // Read the contents of the file into a vector
            binaryBytes.assign(std::istreambuf_iterator<char>(fh),
                               std::istreambuf_iterator<char>());
        } else {
            throw std::runtime_error("Failed to open object file: " +
                                     binaryPath);
        }
    } else {
        throw std::runtime_error(
            "Must provide either a file or string containing object code.");
    }
}

inline std::string getStringFromOffset(const std::vector<unsigned char>& x,
                                       size_t i) {
    std::string result;
    while (i < x.size() && x[i] != '\0') {
        result += x[i];
        ++i;
    }
    return result;
}

struct ELFDisAssembler {
    std::string binaryPath;
    std::string strategy;

    std::vector<unsigned char> binaryBytes;
    DisAssembler* da;

    ELF64_FILE_HEADER header;
    ELF64_SECTION_HEADER shstr;
    std::unordered_map<std::string, ELF64_SECTION_HEADER> section_headers;
    std::unordered_map<uint64_t, std::string> addr2symbol;
    std::unordered_map<int, std::string> pltIdx2symbol;
    std::unordered_map<uint64_t, uint64_t> addr2roffset;
    std::unordered_map<int, uint64_t> pltIdx2roffset;

    ELFDisAssembler(std::string binaryPath, std::string strategy)
        : binaryPath(binaryPath), strategy(strategy) {
        load(binaryPath, binaryBytes);

        _parseFileHeader();
        _parseSectionHeader();
        _parseSymTabSection();
        _parseDynSymSection();
        _parsePltSecSection();

        _prepareDA();
    }

    void _prepareDA() {
        if (strategy == "ls" || strategy == "linearsweep") {
            da = new LinearSweepDisAssembler(binaryBytes, addr2symbol);
        } else if (strategy == "rd" || strategy == "recursivedescent") {
            da = new RecursiveDescentDisAssembler(binaryBytes, addr2symbol);
        } else {
            std::cerr << strategy
                      << " is not supported as a valid strategy. We currently "
                         "support [linearsweep (ls), recursivedescent (rd)].";
            std::cerr << "The default strategy (linearsweep) is used for the "
                         "following task."
                      << std::endl;
            da = new LinearSweepDisAssembler(binaryBytes, addr2symbol);
        }
    }

    void disas(std::string section_name = ".text") {
        if (section_headers.find(section_name) != section_headers.end()) {
            da->disas((uint64_t)section_headers[section_name].sh_offset,
                      (uint64_t)section_headers[section_name].sh_offset +
                          (uint64_t)section_headers[section_name].sh_size - 1);
        }
    }

    void print() {
        std::vector<std::pair<uint64_t, uint64_t>> disassembledPositionsVec(
            da->disassembledPositions.begin(), da->disassembledPositions.end());
        std::sort(disassembledPositionsVec.begin(),
                  disassembledPositionsVec.end());

        std::string postprefix = "";

        std::unordered_map<std::string, bool> done;
        for (const std::string& s : PRINTABLE_SECTIONS) {
            done.insert(std::make_pair(s, false));
        }

        for (const std::pair<size_t, size_t>& k : disassembledPositionsVec) {
            for (const std::string& s : PRINTABLE_SECTIONS) {
                if ((section_headers.find(s) != section_headers.end()) &&
                    (k.first >= section_headers[s].sh_offset) &&
                    (k.first < section_headers[s].sh_offset +
                                   section_headers[s].sh_size)) {
                    if (!done[s]) {
                        std::cout << std::endl
                                  << "section: " << s << " ----" << std::endl;
                        done[s] = true;
                        postprefix = SECTION_LABEL_POSTFIX.at(s);
                    }
                }
            }

            if (da->disassembledInstructions.find(k) !=
                da->disassembledInstructions.end()) {
                if (addr2symbol.find(k.first) != addr2symbol.end()) {
                    std::cout << std::endl
                              << std::hex << k.first << " <"
                              << addr2symbol.at(k.first) << postprefix + ">:";
                    if (addr2roffset.find(k.first) != addr2roffset.end()) {
                        std::cout << " #" << addr2roffset[k.first];
                    }

                    std::cout << std::endl;
                }
                std::cout << " " << std::hex << k.first << ": ";
                std::cout << da->disassembledInstructions.at(k);
                std::cout << std::string(
                    da->maxInstructionStrLength -
                        da->disassembledInstructions.at(k).size(),
                    ' ');

                std::cout << " ( ";
                for (int i = k.first; i < k.second; i++) {
                    std::cout << std::hex << (int)da->binaryBytes[i] << " ";
                }
                std::cout << ")" << std::endl;
            }
        }
        std::cout << "-------------------" << std::endl;
        std::cout << "Done!" << std::endl;
    }

    void _parseFileHeader() {
        std::copy_n(binaryBytes.begin(), sizeof(header),
                    reinterpret_cast<unsigned char*>(&header));
        std::copy_n(binaryBytes.begin() + (int)header.e_shoff +
                        (int)header.e_shstrndx * (int)header.e_shentsize,
                    sizeof(shstr), reinterpret_cast<unsigned char*>(&shstr));
    }

    void _parseSectionHeader() {
        for (int sid = 0; sid < (int)header.e_shnum; sid++) {
            ELF64_SECTION_HEADER sh;
            std::copy_n(binaryBytes.begin() + (int)header.e_shoff +
                            sid * (int)header.e_shentsize,
                        sizeof(sh), reinterpret_cast<unsigned char*>(&sh));
            std::string section_name = getStringFromOffset(
                binaryBytes, (int)shstr.sh_offset + (int)sh.sh_name);
            section_headers.insert(std::make_pair(section_name, sh));
        }
    }

    void _parseSymTabSection() {
        // parse the .symtab section
        if (section_headers.find(".symtab") != section_headers.end() &&
            section_headers.find(".strtab") != section_headers.end()) {
            int symtab_symbol_num = (int)section_headers[".symtab"].sh_size /
                                    (int)(sizeof(ELF64_SYM));
            for (int sid = 0; sid < symtab_symbol_num; sid++) {
                ELF64_SYM sym;
                std::copy_n(binaryBytes.begin() +
                                (int)section_headers[".symtab"].sh_offset +
                                sid * sizeof(ELF64_SYM),
                            sizeof(sym),
                            reinterpret_cast<unsigned char*>(&sym));

                std::string sym_name = getStringFromOffset(
                    binaryBytes, (int)section_headers[".strtab"].sh_offset +
                                     (int)sym.st_name);

                if (sym_name.size() > 0) {
                    addr2symbol.insert(std::make_pair(
                        (uint64_t)((long long)section_headers[".text"]
                                       .sh_offset +
                                   (long long)sym.st_value),
                        sym_name));
                }
            }
        }
    }

    void _parseDynSymSection() {
        if (section_headers.find(".rela.plt") != section_headers.end() &&
            section_headers.find(".dynsym") != section_headers.end() &&
            section_headers.find(".dynstr") != section_headers.end()) {
            int rela_num = (int)section_headers[".rela.plt"].sh_size /
                           (int)(sizeof(ELF64_RELA));
            for (int sid = 0; sid < rela_num; sid++) {
                ELF64_RELA rela;
                std::copy_n(binaryBytes.begin() +
                                (int)section_headers[".rela.plt"].sh_offset +
                                sid * sizeof(ELF64_RELA),
                            sizeof(rela),
                            reinterpret_cast<unsigned char*>(&rela));

                ELF64_SYM sym;
                std::copy_n(binaryBytes.begin() +
                                (int)section_headers[".dynsym"].sh_offset +
                                (rela.r_info >> 32) * sizeof(ELF64_SYM),
                            sizeof(sym),
                            reinterpret_cast<unsigned char*>(&sym));

                std::string sym_name = getStringFromOffset(
                    binaryBytes, (int)section_headers[".dynstr"].sh_offset +
                                     (int)sym.st_name);
                if (sym_name.size() > 0) {
                    pltIdx2symbol.insert(std::make_pair(sid, sym_name));
                    pltIdx2roffset.insert(std::make_pair(sid, rela.r_offset));
                }
            }
        }
    }

    void _parsePltSecSection() {
        if (section_headers.find(".plt.sec") != section_headers.end()) {
            for (const std::pair<int, std::string> kv : pltIdx2symbol) {
                addr2symbol.insert(
                    std::make_pair(section_headers[".plt.sec"].sh_offset +
                                       kv.first * PLT_SEC_ENTRY_SIZE,
                                   kv.second));
                addr2roffset.insert(
                    std::make_pair(section_headers[".plt.sec"].sh_offset +
                                       kv.first * PLT_SEC_ENTRY_SIZE,
                                   pltIdx2roffset[kv.first]));
            }
        }
    }
};
