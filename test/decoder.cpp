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

TEST(decoder, SEVERAL_ADD) {
    std::vector<unsigned char> obj = {
        0x01, 0xc1,  // add eax ecx
        0x01, 0x4,  0x25, 0x00,
        0x00, 0x00, 0x00,        // add  eax 0x0
        0x01, 0x00,              // add  eax (rax)
        0x01, 0x04, 0x00,        // add  rax (rax, rax, 1)
        0x01, 0x44, 0x00, 0x01,  // add  eax 0x1 (rax, rax, 1)
        0x01, 0x84, 0x00, 0x00,
        0x80, 0x00, 0x00  // add  eax, 0x8000(rax, rax, 1)
    };
    DecoderState state(obj);
    X86Decoder decoder(&state);

    state._currentIdx = 0;
    std::pair<std::string, uint64_t> res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(0, 2)], " add  ecx eax");

    state._currentIdx = 2;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(2, 9)],
              " add  [0x00000000 + rsp * 1] eax");

    state._currentIdx = 9;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(9, 11)], " add  [rax] eax");

    state._currentIdx = 11;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(11, 14)],
              " add  [rax + rax * 1] eax");

    state._currentIdx = 14;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(14, 18)],
              " add  [1 + rax + rax * 1] eax");

    state._currentIdx = 18;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(18, 25)],
              " add  [0x00008000 + rax + rax * 1] eax");
}

TEST(decoder, MODRM_REG) {
    std::vector<unsigned char> obj = {
        0x01, 0x00,  // add rax eax
        0x01, 0x08,  // add rax ecx
        0x01, 0x10,  // add rax edx
        0x01, 0x18,  // add rax ebx
        0x01, 0x20,  // add rax esp
        0x01, 0x28,  // add rax ebp
        0x01, 0x30,  // add rax esi
        0x01, 0x38   // add rax edi
    };
    DecoderState state(obj);
    X86Decoder decoder(&state);

    state._currentIdx = 0;
    std::pair<std::string, uint64_t> res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(0, 2)], " add  [rax] eax");

    state._currentIdx = 2;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(2, 4)], " add  [rax] ecx");

    state._currentIdx = 4;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(4, 6)], " add  [rax] edx");

    state._currentIdx = 6;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(6, 8)], " add  [rax] ebx");

    state._currentIdx = 8;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(8, 10)], " add  [rax] esp");

    state._currentIdx = 10;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(10, 12)], " add  [rax] ebp");

    state._currentIdx = 12;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(12, 14)], " add  [rax] esi");

    state._currentIdx = 14;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(14, 16)], " add  [rax] edi");
}

TEST(decoder, MODRM_MOD11) {
    std::vector<unsigned char> obj = {0x01, 0xc0,  // add eax eax
                                      0x01, 0xc1,  // add eax ecx
                                      0x01, 0xc2,  // add eax edx
                                      0x01, 0xc3,  // add eax ebx
                                      0x01, 0xc4,  // add eax esp
                                      0x01, 0xc5,  // add eax ebp
                                      0x01, 0xc6,  // add eax esi
                                      0x01, 0xc7,  // add eax edi
                                      0x03, 0xc0};
    DecoderState state(obj);
    X86Decoder decoder(&state);

    state._currentIdx = 0;
    std::pair<std::string, uint64_t> res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(0, 2)], " add  eax eax");

    state._currentIdx = 2;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(2, 4)], " add  ecx eax");

    state._currentIdx = 4;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(4, 6)], " add  edx eax");

    state._currentIdx = 6;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(6, 8)], " add  ebx eax");

    state._currentIdx = 8;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(8, 10)], " add  esp eax");

    state._currentIdx = 10;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(10, 12)], " add  ebp eax");

    state._currentIdx = 12;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(12, 14)], " add  esi eax");

    state._currentIdx = 14;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(14, 16)], " add  edi eax");

    state._currentIdx = 16;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "add");
    ASSERT_EQ(state.instructions[std::make_pair(16, 18)], " add  eax eax");
}

TEST(decoder, MODRM_MOD_DISP) {
    std::vector<unsigned char> obj = {
        0x8b, 0x08,                          // add eax ecx
        0x8b, 0x48, 0x01,                    // mov
        0x8b, 0x88, 0x00, 0x01, 0x00, 0x00,  // add  eax 0x0
    };
    DecoderState state(obj);
    X86Decoder decoder(&state);

    state._currentIdx = 0;
    std::pair<std::string, uint64_t> res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "mov");
    ASSERT_EQ(state.instructions[std::make_pair(0, 2)], " mov  ecx [rax]");

    state._currentIdx = 2;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "mov");
    ASSERT_EQ(state.instructions[std::make_pair(2, 5)],
              " mov  ecx [rax + 1]");

    state._currentIdx = 5;
    decoder.init();
    res = decoder.decodeSingleInstruction();
    ASSERT_EQ(res.first, "mov");
    ASSERT_EQ(state.instructions[std::make_pair(5, 11)],
              " mov  ecx [rax + 0x00000100]");
}

