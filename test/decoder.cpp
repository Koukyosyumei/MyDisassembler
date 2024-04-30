#include "decoder.h"

#include <gtest/gtest.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

TEST(decoder, ONE_BYTE) {
    std::vector<unsigned char> obj = {0x90, 0xC3};
    DecoderState state(obj);
    X86Decoder decoder(&state);

    state._currentIdx = 0;
    std::pair<std::string, uint64_t> res_nop =
        decoder.decodeSingleInstruction();
    ASSERT_EQ(res_nop.first, "nop");

    decoder.init();
    state._currentIdx = 1;
    std::pair<std::string, uint64_t> res_ret =
        decoder.decodeSingleInstruction();
    ASSERT_EQ(res_ret.first, "ret");
}

TEST(decoder, ONE_BYTE_IMM) {
    std::vector<unsigned char> obj = {
        0xb8, 0x44, 0x33, 0x22, 0x11, 0xb9, 0x44, 0x33, 0x22, 0x11,
        0x05, 0x44, 0x33, 0x22, 0x11, 0x2d, 0x44, 0x33, 0x22, 0x11,
    };
    DecoderState state(obj);
    X86Decoder decoder(&state);

    state._currentIdx = 0;
    std::pair<std::string, uint64_t> res_nop =
        decoder.decodeSingleInstruction();
    ASSERT_EQ(res_nop.first, "mov");
    // TODO: FIX the range
    ASSERT_EQ(state.instructions[std::make_pair(0, 6)], " mov  eax 0x11223344");

    state._currentIdx = 5;
    decoder.init();
    res_nop = decoder.decodeSingleInstruction();

    for (auto& item : state.instructions) {
        std::cout << item.first.first << "-" << item.first.second << ": "
                  << item.second << std::endl;
    }

    ASSERT_EQ(res_nop.first, "mov");
    // TODO: FIX the range
    ASSERT_EQ(state.instructions[std::make_pair(0, 6)], " mov  ecx 0x11223344");

    state._currentIdx = 10;
    decoder.init();
    res_nop = decoder.decodeSingleInstruction();
    ASSERT_EQ(res_nop.first, "add");
    // TODO: FIX the range
    ASSERT_EQ(state.instructions[std::make_pair(0, 6)], " add  eax 0x11223344");

    state._currentIdx = 15;
    decoder.init();
    res_nop = decoder.decodeSingleInstruction();
    ASSERT_EQ(res_nop.first, "sub");
    // TODO: FIX the range
    ASSERT_EQ(state.instructions[std::make_pair(0, 6)], " sub  eax 0x11223344");
}
