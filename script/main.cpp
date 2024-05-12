#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "elfdisas.h"

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

    ELFDisAssembler eda(binaryPath, strategy);

    eda.disas(".plt.got");
    eda.disas(".plt.sec");
    eda.disas(".text");
    eda.disas(".init");
    eda.disas(".fini");

    eda.print();
}
