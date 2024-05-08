#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
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

int main(int argc, char* argv[]) {
    std::string binaryPath = argv[1];

    std::vector<unsigned char> binaryBytes;
    ELF64_FILE_HEADER header;
    ELF64_SECTION_HEADER shstr;
    std::unordered_map<std::string, ELF64_SECTION_HEADER> section_headers;
    std::unordered_map<long long, std::string> addr2symbol;

    load(binaryPath, binaryBytes);
    std::copy_n(binaryBytes.begin(), sizeof(header),
                reinterpret_cast<unsigned char*>(&header));
    std::copy_n(binaryBytes.begin() + (int)header.e_shoff +
                    (int)header.e_shstrndx * (int)header.e_shentsize,
                sizeof(shstr), reinterpret_cast<unsigned char*>(&shstr));

    // parse the file header
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
                addr2symbol.insert(
                    std::make_pair((long long)sym.st_value, sym_name));
            }
        }
    }

    // parse the .text section
    if (section_headers.find(".text") != section_headers.end()) {
        std::vector<unsigned char> text_section_binaryBytes(
            binaryBytes.begin() + (int)section_headers[".text"].sh_offset,
            binaryBytes.begin() + (int)section_headers[".text"].sh_offset +
                (int)section_headers[".text"].sh_size);

        LinearSweepDisAssembler da(text_section_binaryBytes, addr2symbol);
        da.disas();

        std::sort(da.disassembledPositions.begin(),
                  da.disassembledPositions.end());

        std::cout << "section: .text" << std::endl;
        for (const std::pair<size_t, size_t> k : da.disassembledPositions) {
            if (da.disassembledInstructions.find(k) !=
                da.disassembledInstructions.end()) {
                if (addr2symbol.find((long long)k.first) != addr2symbol.end()) {
                    std::cout << std::endl
                              << addr2symbol.at(k.first) << std::endl;
                }
                std::cout << " " << k.first << ": "
                          << da.disassembledInstructions.at(k) << std::endl;
            }
        }
    }
}
