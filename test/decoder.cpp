#include <gtest/gtest.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

#include "disassembler.h"

const std::unordered_map<long long, std::string> addr2symbol;

TEST(disas, ONE_BYTE) {
    std::vector<unsigned char> obj = {0x90, 0xC3};
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 1)], "nop ");

    disas.curIdx = 1;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(1, 2)], "ret ");
}

TEST(disas, ONE_BYTE_IMM) {
    std::vector<unsigned char> obj = {
        0xb8, 0x44, 0x33, 0x22, 0x11,  // mov  eax 0x11223344
        0xb9, 0x44, 0x33, 0x22, 0x11,  // mov  ecx 0x11223344
        0x05, 0x44, 0x33, 0x22, 0x11,  // add  eax 0x11223344
        0x2d, 0x44, 0x33, 0x22, 0x11,  // sub  eax 0x11223344
    };
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 5)],
              "mov  eax 0x11223344");

    disas.curIdx = 5;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(5, 10)],
              "mov  ecx 0x11223344");

    disas.curIdx = 10;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(10, 15)],
              "add  eax 0x11223344");

    disas.curIdx = 15;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(15, 20)],
              "sub  eax 0x11223344");
}

TEST(disas, ONE_BYTE_IMM_SIZE) {
    std::vector<unsigned char> obj = {
        0xb0, 0x11,                    // mov  0x11 al
        0x66, 0xb8, 0x22, 0x11,        // mov  0x1122 ax
        0xb8, 0x44, 0x33, 0x22, 0x11,  // mov  0x11223344 eax
        0x48, 0xb8, 0x88, 0x77, 0x66,
        0x55, 0x44, 0x33, 0x22, 0x11  // movabs 0x1122334455667788 rax
    };
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 2)],
              "mov  al 0x11");

    disas.curIdx = 2;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(2, 6)],
              "mov  ax 0x1122");

    disas.curIdx = 6;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(6, 11)],
              "mov  eax 0x11223344");

    disas.curIdx = 11;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(11, 21)],
              "mov  rax 0x1122334455667788");
}

TEST(disas, SEVERAL_ADD) {
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
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 2)],
              "add  ecx eax");

    disas.curIdx = 2;
    disas.step();

    disas.curIdx = 9;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(9, 11)],
              "add  [rax] eax");

    disas.curIdx = 11;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(11, 14)],
              "add  [rax + rax * 1] eax");

    disas.curIdx = 14;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(14, 18)],
              "add  [1 + rax + rax * 1] eax");

    disas.curIdx = 18;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(18, 25)],
              "add  [0x00008000 + rax + rax * 1] eax");
}

TEST(disas, MODRM_REG) {
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
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 2)],
              "add  [rax] eax");

    disas.curIdx = 2;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(2, 4)],
              "add  [rax] ecx");

    disas.curIdx = 4;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(4, 6)],
              "add  [rax] edx");

    disas.curIdx = 6;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(6, 8)],
              "add  [rax] ebx");

    disas.curIdx = 8;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(8, 10)],
              "add  [rax] esp");

    disas.curIdx = 10;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(10, 12)],
              "add  [rax] ebp");

    disas.curIdx = 12;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(12, 14)],
              "add  [rax] esi");

    disas.curIdx = 14;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(14, 16)],
              "add  [rax] edi");
}

TEST(disas, MODRM_MOD11) {
    std::vector<unsigned char> obj = {0x01, 0xc0,  // add eax eax
                                      0x01, 0xc1,  // add eax ecx
                                      0x01, 0xc2,  // add eax edx
                                      0x01, 0xc3,  // add eax ebx
                                      0x01, 0xc4,  // add eax esp
                                      0x01, 0xc5,  // add eax ebp
                                      0x01, 0xc6,  // add eax esi
                                      0x01, 0xc7,  // add eax edi
                                      0x03, 0xc0};
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 2)],
              "add  eax eax");

    disas.curIdx = 2;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(2, 4)],
              "add  ecx eax");

    disas.curIdx = 4;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(4, 6)],
              "add  edx eax");

    disas.curIdx = 6;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(6, 8)],
              "add  ebx eax");

    disas.curIdx = 8;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(8, 10)],
              "add  esp eax");

    disas.curIdx = 10;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(10, 12)],
              "add  ebp eax");

    disas.curIdx = 12;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(12, 14)],
              "add  esi eax");

    disas.curIdx = 14;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(14, 16)],
              "add  edi eax");

    disas.curIdx = 16;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(16, 18)],
              "add  eax eax");
}

TEST(disas, MODRM_MOD_DISP) {
    std::vector<unsigned char> obj = {
        0x8b, 0x08,                          // add eax ecx
        0x8b, 0x48, 0x01,                    // mov
        0x8b, 0x88, 0x00, 0x01, 0x00, 0x00,  // add  eax 0x0
    };
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 2)],
              "mov  ecx [rax]");

    disas.curIdx = 2;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(2, 5)],
              "mov  ecx [rax + 1]");

    disas.curIdx = 5;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(5, 11)],
              "mov  ecx [rax + 0x00000100]");
}

TEST(disas, MODRM_MOD00_RM101) {
    std::vector<unsigned char> obj = {
        0x8b, 0x4d, 0x00,                    // add eax ecx
        0x8b, 0x4d, 0x01,                    // mov
        0x8b, 0x8d, 0x00, 0x01, 0x00, 0x00,  // add  eax 0x0
        0x8b, 0x0c, 0x25, 0x00, 0x00, 0x08, 0x00};
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 3)],
              "mov  ecx [rbp + 0]");

    disas.curIdx = 3;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(3, 6)],
              "mov  ecx [rbp + 1]");

    disas.curIdx = 6;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(6, 12)],
              "mov  ecx [rbp + 0x00000100]");

    disas.curIdx = 12;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(12, 19)],
              "mov  ecx 0x00080000");
}

TEST(disas, MODRM_SIB_RSP) {
    std::vector<unsigned char> obj = {
        0x8b, 0x14, 0x08,        // add eax ecx
        0x8b, 0x54, 0x08, 0x01,  // mov
        0x8b, 0x14, 0x48,        // add  eax 0x0
        0x8b, 0x14, 0x24         // add
    };
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 3)],
              "mov  edx [rax + rcx * 1]");

    disas.curIdx = 3;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(3, 7)],
              "mov  edx [1 + rax + rcx * 1]");

    disas.curIdx = 7;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(7, 10)],
              "mov  edx [rax + rcx * 2]");

    disas.curIdx = 10;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(10, 13)],
              "mov  edx [rsp]");
}

TEST(disas, ADD_IMM) {
    std::vector<unsigned char> obj = {
        0x01, 0xc0,        // add eax ecx
        0x83, 0xc0, 0x01,  // mov
    };
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 2)],
              "add  eax eax");

    disas.curIdx = 2;

    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(2, 5)],
              "add  eax 0x01");
}

TEST(disas, MODRM_OPCODE) {
    std::vector<unsigned char> obj = {
        0x83, 0xc0, 0x01,  // add eax ecx
        0x83, 0xc8, 0x01,  // mov
        0x83, 0xd0, 0x01,  // add eax ecx
        0x83, 0xd8, 0x01,  // mov
        0x83, 0xe0, 0x01,  // add eax ecx
        0x83, 0xe8, 0x01,  // mov
        0x83, 0xf0, 0x01,  // add eax ecx
        0x83, 0xf8, 0x01,  // mov
    };
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 3)],
              "add  eax 0x01");

    disas.curIdx = 3;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(3, 6)],
              "or  eax 0x01");

    disas.curIdx = 6;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(6, 9)],
              "adc  eax 0x01");

    disas.curIdx = 9;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(9, 12)],
              "sbb  eax 0x01");

    disas.curIdx = 12;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(12, 15)],
              "and  eax 0x01");

    disas.curIdx = 15;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(15, 18)],
              "sub  eax 0x01");

    disas.curIdx = 18;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(18, 21)],
              "xor  eax 0x01");

    disas.curIdx = 21;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(21, 24)],
              "cmp  eax 0x01");
}

TEST(disas, REXW) {
    std::vector<unsigned char> obj = {
        0x83, 0xc0, 0x01,        // add eax ecx
        0x48, 0x83, 0xc0, 0x01,  // mov
    };
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 3)],
              "add  eax 0x01");

    disas.curIdx = 3;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(3, 7)],
              "add  rax 0x01");
}

TEST(disas, REXRXB) {
    std::vector<unsigned char> obj = {
        0x44, 0x01, 0x04, 0x91,  // add eax ecx
        0x42, 0x01, 0x04, 0x91,  // mov
        0x41, 0x01, 0x04, 0x91   // add
    };
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 4)],
              "add  [rcx + rdx * 4] r8d");

    disas.curIdx = 4;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(4, 8)],
              "add  [rcx + r10 * 4] eax");

    disas.curIdx = 8;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(8, 12)],
              "add  [r9 + rdx * 4] eax");
}

TEST(disas, TWOBYTE) {
    std::vector<unsigned char> obj = {
        0x0f, 0xaf, 0xc3,  // add eax ecx
        0x48, 0x0f, 0xbe, 0x04, 0x25,
        0x00, 0x00, 0x00, 0x00,  // movsx 0x0, rax, movsx bx, rax
        0x48, 0x63, 0xc3,        // movsx
    };
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 3)],
              "imul  eax ebx");

    disas.curIdx = 3;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(3, 12)],
              "movsx  rax 0x00000000");

    disas.curIdx = 12;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(12, 15)],
              "movsxd  rax ebx");
}

TEST(disas, CFInstructions) {
    std::vector<unsigned char> obj = {
        0xe8, 0x07, 0x00, 0x00, 0x00,  // call c <subroutine>
        0x74, 0x02,                    // je c
        0xeb, 0x04,                    // jmp 12
        0xeb, 0xf2,                    // jmp 0
    };
    LinearSweepDisAssembler disas(obj, addr2symbol);

    disas.curIdx = 0;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(0, 5)],
              "call 12 ; relative offset = 7");

    disas.curIdx = 5;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(5, 7)],
              "jz 9 ; relative offset = 2");

    disas.curIdx = 7;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(7, 9)],
              "jmp 13 ; relative offset = 4");

    disas.curIdx = 9;
    disas.step();
    ASSERT_EQ(disas.disassembledInstructions[std::make_pair(9, 11)],
              "jmp -3 ; relative offset = -14");
}

