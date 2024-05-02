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
    std::pair<std::string, uint64_t> res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "nop");

    decoder.init();
    state._currentIdx = 1;
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "ret");
}

TEST(decoder, ONE_BYTE_IMM) {
    std::vector<unsigned char> obj = {
        0xb8, 0x44, 0x33, 0x22, 0x11, 0xb9, 0x44, 0x33, 0x22, 0x11,
        0x05, 0x44, 0x33, 0x22, 0x11, 0x2d, 0x44, 0x33, 0x22, 0x11,
    };
    DecoderState state(obj);
    X86Decoder decoder(&state);

    state._currentIdx = 0;
    std::pair<std::string, uint64_t> res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "mov");
    ASSERT_EQ(state.instructions[std::make_pair(0, 5)], " mov  eax 0x11223344");

    state._currentIdx = 5;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "mov");
    ASSERT_EQ(state.instructions[std::make_pair(5, 10)],
              " mov  ecx 0x11223344");

    state._currentIdx = 10;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(10, 15)],
              " add  eax 0x11223344");

    state._currentIdx = 15;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "sub");
    ASSERT_EQ(state.instructions[std::make_pair(15, 20)],
              " sub  eax 0x11223344");
}
