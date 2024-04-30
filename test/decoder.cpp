#include <gtest/gtest.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "decoder.h"

TEST(decoder, ONE_BYTE) {
    std::vector<unsigned char> obj = {0x90, 0xC3};
    DecoderState state(obj);
    X86Decoder decoder(&state);

    state._currentIdx = 0;
    std::pair<std::string, uint64_t> res_nop = decoder.decodeSingleInstruction();
    ASSERT_EQ(res_nop.first, "nop");
    
    decoder.init();
    state._currentIdx = 1;
    std::pair<std::string, uint64_t> res_ret = decoder.decodeSingleInstruction();
    ASSERT_EQ(res_ret.first, "ret");
}

TEST(decoder, ONE_BYTE_IMM) {
    std::vector<unsigned char> obj = {0xb8, 0x44, 0x33, 0x22, 0x11};
    DecoderState state(obj);
    X86Decoder decoder(&state);

    state._currentIdx = 0;
    std::pair<std::string, uint64_t> res_nop = decoder.decodeSingleInstruction();
    ASSERT_EQ(res_nop.first, "mov");

    // TODO: FIX the range
    ASSERT_EQ(state.instructions[std::make_pair(0, 6)], " mov  eax 0x11223344");
}
