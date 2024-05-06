#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "header.h"
#include "disassembler.h"

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

int main(int argc, char* argv[]) {
    std::vector<unsigned char> binaryBytes;
    std::string binaryPath = argv[1];
    load(binaryPath, binaryBytes);

    LinearSweepDisAssembler da(binaryBytes);
    
    ELF64_HEADER header;
    std::copy_n(binaryBytes.begin(), sizeof(header), reinterpret_cast<unsigned char*>(&header));
    for (int i = 0; i < 64; i++) {
        std::cout << i << " " << (int)binaryBytes[i] << std::endl;
    }
    std::cout << "e_phoff: " << (int)header.e_phoff << std::endl;
    std::cout << "e_phentsize: " << (int)header.e_phentsize << std::endl;

    da._currentIdx = 64;
    da.disas();

    std::cout << da.disassembledInstructions.size() << std::endl;
    for (const auto& di : da.disassembledInstructions) {
        std::cout << di.first.first << "-" << di.first.second << ": "
                  << di.second << std::endl;
    }
}
