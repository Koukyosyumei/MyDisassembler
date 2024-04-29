#include <fstream>
#include <iostream>
#include <string>
#include <vector>

void load(const std::string& objectFile,
          std::vector<unsigned char>& objectSource) {
    if (!objectFile.empty()) {
        std::ifstream fh(objectFile, std::ios::binary);
        if (fh) {
            // Read the contents of the file into a vector
            objectSource.assign(std::istreambuf_iterator<char>(fh),
                                std::istreambuf_iterator<char>());
        } else {
            throw std::runtime_error("Failed to open object file: " +
                                     objectFile);
        }
    } else {
        throw std::runtime_error(
            "Must provide either a file or string containing object code.");
    }
}

// Additional methods can be added as needed

// Example method to print objectSource
void print(std::vector<unsigned char>& objectSource) {
    std::cout << "Object Source: ";
    for (unsigned char byte : objectSource) {
        std::cout << std::hex << (int)byte << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        std::vector<unsigned char> objectSource;
        std::string objectFile = argv[1];
        load(objectFile, objectSource);
        print(objectSource);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}

