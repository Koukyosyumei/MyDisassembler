#include <fstream>
#include <iostream>
#include <string>
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
    std::vector<unsigned char> binaryBytes;
    std::string binaryPath = argv[1];
    load(binaryPath, binaryBytes);

    ELF64_FILE_HEADER header;
    std::copy_n(binaryBytes.begin(), sizeof(header),
                reinterpret_cast<unsigned char*>(&header));

    ELF64_SECTION_HEADER shstr;
    std::copy_n(binaryBytes.begin() + (int)header.e_shoff +
                    (int)header.e_shstrndx * (int)header.e_shentsize,
                sizeof(shstr), reinterpret_cast<unsigned char*>(&shstr));

    std::vector<ELF64_SECTION_HEADER> section_headers;
    int text_section_offset = 0;
    int text_section_size = 0;

    for (int sid = 0; sid < (int)header.e_shnum; sid++) {
        ELF64_SECTION_HEADER sh;
        std::copy_n(binaryBytes.begin() + (int)header.e_shoff +
                        sid * (int)header.e_shentsize,
                    sizeof(sh), reinterpret_cast<unsigned char*>(&sh));
        std::string section_name = getStringFromOffset(
            binaryBytes, (int)shstr.sh_offset + (int)sh.sh_name);
        section_headers.emplace_back(sh);
        if (section_name == ".text") {
            text_section_offset = (int)sh.sh_offset;
            text_section_size = (int)sh.sh_size;
        }
    }

    std::vector<unsigned char> text_section_binaryBytes(
        binaryBytes.begin() + text_section_offset,
        binaryBytes.begin() + text_section_offset + text_section_size);

    LinearSweepDisAssembler da(text_section_binaryBytes);
    da.disas();

    std::sort(da.instructionKeys.begin(), da.instructionKeys.end());

    for (const std::pair<size_t, size_t> k : da.instructionKeys) {
        if (da.disassembledInstructions.find(k) !=
            da.disassembledInstructions.end()) {
            std::cout << k.first << ": " << da.disassembledInstructions.at(k)
                      << std::endl;
        }
    }
}
