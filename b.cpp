#include <iostream>

#include "src/table.h"

int main() {
    for (const auto kv : OP_LOOKUP) {
        for (const auto y : kv.second) {
            std::cout << "(" << to_string(kv.first.first) << ", "
                      << kv.first.second << ") " << y.first << " "
                      << to_string(y.second) << std::endl;
        }
    }

    std::cout << to_string(
                     (OP_LOOKUP.at(std::make_pair(Prefix::NONE, 185))).at(1))
              << std::endl;
}
