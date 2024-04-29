#include <gtest/gtest.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "decoder.h"

TEST(decoder, NOP) {
    std::vector<unsigned char> obj = {0x90};
}
