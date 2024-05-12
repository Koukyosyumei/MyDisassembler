#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "disassembler.h"
#include "header.h"

void load(const std::string& binaryPath,
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

std::string getStringFromOffset(const std::vector<unsigned char>& x, size_t i) {
    std::string result;
    while (i < x.size() && x[i] != '\0') {
        result += x[i];
        ++i;
    }
    return result;
}

std::string strategy = "linearsweep";

int main(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "s:")) != -1) {
        switch (opt) {
            case 's':
                strategy = std::string(optarg);
                break;
            default:
                std::cout << "unknown parameter is specified" << std::endl;
                break;
        }
    }

    std::string binaryPath = argv[optind];

    std::vector<unsigned char> binaryBytes;
    ELF64_FILE_HEADER header;
    ELF64_SECTION_HEADER shstr;
    std::unordered_map<std::string, ELF64_SECTION_HEADER> section_headers;
    std::unordered_map<uint64_t, std::string> addr2symbol;
    std::unordered_map<int, std::string> pltIdx2symbol;
    std::unordered_map<uint64_t, uint64_t> addr2roffset;
    std::unordered_map<int, uint64_t> pltIdx2roffset;

    load(binaryPath, binaryBytes);

    // parse the file header
    std::copy_n(binaryBytes.begin(), sizeof(header),
                reinterpret_cast<unsigned char*>(&header));
    std::copy_n(binaryBytes.begin() + (int)header.e_shoff +
                    (int)header.e_shstrndx * (int)header.e_shentsize,
                sizeof(shstr), reinterpret_cast<unsigned char*>(&shstr));

    std::cout << "e_entry: " << std::hex << header.e_entry << std::endl;

    for (int sid = 0; sid < (int)header.e_shnum; sid++) {
        ELF64_SECTION_HEADER sh;
        std::copy_n(binaryBytes.begin() + (int)header.e_shoff +
                        sid * (int)header.e_shentsize,
                    sizeof(sh), reinterpret_cast<unsigned char*>(&sh));
        std::string section_name = getStringFromOffset(
            binaryBytes, (int)shstr.sh_offset + (int)sh.sh_name);
        section_headers.insert(std::make_pair(section_name, sh));
    }

    // parse the .symtab section
    if (section_headers.find(".symtab") != section_headers.end() &&
        section_headers.find(".strtab") != section_headers.end()) {
        int symtab_symbol_num =
            (int)section_headers[".symtab"].sh_size / (int)(sizeof(ELF64_SYM));
        for (int sid = 0; sid < symtab_symbol_num; sid++) {
            ELF64_SYM sym;
            std::copy_n(binaryBytes.begin() +
                            (int)section_headers[".symtab"].sh_offset +
                            sid * sizeof(ELF64_SYM),
                        sizeof(sym), reinterpret_cast<unsigned char*>(&sym));

            std::string sym_name = getStringFromOffset(
                binaryBytes,
                (int)section_headers[".strtab"].sh_offset + (int)sym.st_name);

            if (sym_name.size() > 0) {
                addr2symbol.insert(std::make_pair(
                    (uint64_t)((long long)section_headers[".text"].sh_offset +
                               (long long)sym.st_value),
                    sym_name));
            }
        }
    }

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
                        sizeof(rela), reinterpret_cast<unsigned char*>(&rela));

            ELF64_SYM sym;
            std::copy_n(binaryBytes.begin() +
                            (int)section_headers[".dynsym"].sh_offset +
                            (rela.r_info >> 32) * sizeof(ELF64_SYM),
                        sizeof(sym), reinterpret_cast<unsigned char*>(&sym));

            std::string sym_name = getStringFromOffset(
                binaryBytes,
                (int)section_headers[".dynstr"].sh_offset + (int)sym.st_name);
            if (sym_name.size() > 0) {
                pltIdx2symbol.insert(std::make_pair(sid, sym_name));
                pltIdx2roffset.insert(std::make_pair(sid, rela.r_offset));
            }
        }
    }

    if (section_headers.find(".plt.sec") != section_headers.end()) {
        for (const std::pair<int, std::string>& kv : pltIdx2symbol) {
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

    // parse the .text section
    if (section_headers.find(".text") != section_headers.end()) {
        std::cout << "sh_offset: " << (int)section_headers[".text"].sh_offset
                  << std::endl;

        DisAssembler* da;

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

        da->disas((uint64_t)section_headers[".text"].sh_offset,
                  (uint64_t)section_headers[".text"].sh_offset +
                      (uint64_t)section_headers[".text"].sh_size - 1);

        std::vector<std::pair<uint64_t, uint64_t>> disassembledPositionsVec(
            da->disassembledPositions.begin(), da->disassembledPositions.end());
        std::sort(disassembledPositionsVec.begin(),
                  disassembledPositionsVec.end());

        bool enter_plt_got = false;
        bool enter_plt_sec = false;
        bool enter_text = false;
        std::string postprefix = "";

        for (const std::pair<size_t, size_t>& k : disassembledPositionsVec) {
            if (k.first >= section_headers[".plt.got"].sh_offset &&
                k.first < section_headers[".plt.got"].sh_offset +
                              section_headers[".plt.got"].sh_size) {
                if (!enter_plt_got) {
                    std::cout << std::endl
                              << "section: .plt.got ----" << std::endl;
                    enter_plt_got = true;
                    postprefix = "@plt";
                }
            }

            else if (k.first >= section_headers[".plt.sec"].sh_offset &&
                     k.first < section_headers[".plt.sec"].sh_offset +
                                   section_headers[".plt.sec"].sh_size) {
                if (!enter_plt_sec) {
                    std::cout << std::endl
                              << "section: .plt.sec ----" << std::endl;
                    enter_plt_sec = true;
                    postprefix = "@plt";
                }
            }

            else if (k.first >= section_headers[".text"].sh_offset &&
                     k.first < section_headers[".text"].sh_offset +
                                   section_headers[".text"].sh_size) {
                if (!enter_text) {
                    std::cout << std::endl
                              << "section: .text ----" << std::endl
                              << std::endl;
                    enter_text = true;
                    postprefix = "";
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
                std::cout << " " << k.first << ": "
                          << da->disassembledInstructions.at(k) << std::endl;
            }
        }
        std::cout << "-------------------" << std::endl;
        std::cout << "Done!" << std::endl;
    }
}
