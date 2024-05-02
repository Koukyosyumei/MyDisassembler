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
        0xb8, 0x44, 0x33, 0x22, 0x11,  // mov  eax 0x11223344
        0xb9, 0x44, 0x33, 0x22, 0x11,  // mov  ecx 0x11223344
        0x05, 0x44, 0x33, 0x22, 0x11,  // add  eax 0x11223344
        0x2d, 0x44, 0x33, 0x22, 0x11,  // sub  eax 0x11223344
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

TEST(decoder, ONE_BYTE_IMM_SIZE) {
    std::vector<unsigned char> obj = {
        0xb0, 0x11,                    // mov  0x11 al
        0x66, 0xb8, 0x22, 0x11,        // mov  0x1122 ax
        0xb8, 0x44, 0x33, 0x22, 0x11,  // mov  0x11223344 eax
        0x48, 0xb8, 0x88, 0x77, 0x66,
        0x55, 0x44, 0x33, 0x22, 0x11  // movabs 0x1122334455667788 rax
    };
    DecoderState state(obj);
    X86Decoder decoder(&state);

    state._currentIdx = 0;
    std::pair<std::string, uint64_t> res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "mov");
    ASSERT_EQ(state.instructions[std::make_pair(0, 2)], " mov  al 0x11");

    state._currentIdx = 2;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "mov");
    ASSERT_EQ(state.instructions[std::make_pair(2, 6)], " mov  ax 0x1122");

    state._currentIdx = 6;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "mov");
    ASSERT_EQ(state.instructions[std::make_pair(6, 11)],
              " mov  eax 0x11223344");

    state._currentIdx = 11;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "mov");
    ASSERT_EQ(state.instructions[std::make_pair(11, 21)],
              " mov  rax 0x1122334455667788");
}
