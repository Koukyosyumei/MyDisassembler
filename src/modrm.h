#pragma once
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "constants.h"
#include "utils.h"

inline unsigned char assembleModRmByte(unsigned char modByte,
                                       unsigned char regByte,
                                       unsigned char rmByte) {
    return ((modByte << 6) + (regByte << 3) + rmByte);
}

inline unsigned char assembleSibByte(unsigned char scaleByte,
                                     unsigned char indexByte,
                                     unsigned char baseByte) {
    return ((scaleByte << 6) + (indexByte << 3) + baseByte);
}

inline unsigned char getRegVal(unsigned char modrmByte) {
    return (modrmByte >> 3) & 0x7;
}

struct REX {
    bool rexB;
    bool rexX;
    bool rexR;
    bool rexW;

    REX() {}
    REX(unsigned char rexByte) {
        rexB = (rexByte & 0x1) == 0x1;
        rexX = (rexByte & 0x2) == 0x2;
        rexR = (rexByte & 0x4) == 0x4;
        rexW = (rexByte & 0x8) == 0x8;
    }
};

struct ModRM {
    unsigned char modByte;
    unsigned char regByte;
    unsigned char rmByte;

    std::string addressingMode;
    std::string reg;

    bool hasDisp8;
    bool hasDisp32;
    bool hasSib;

    ModRM() {}
    ModRM(unsigned char modrmByte, bool rexb = false) {
        rmByte = modrmByte & 0x7;
        regByte = (modrmByte >> 3) & 0x7;
        modByte = (modrmByte >> 6) & 0x3;

        reg = id2register.at(regByte + (rexb ? 0 : 8));

        std::string baseReg = reg;
        if (modByte < 3 && rmByte == 4) {
            baseReg = "SIB";
        }

        switch (modByte) {
            case 0x00: {
                addressingMode = "[" + baseReg + "]";
                hasDisp8 = false;
                hasDisp32 = false;
            }
            case 0x01: {
                addressingMode = "[" + baseReg + " + disp8]";
                hasDisp8 = true;
                hasDisp32 = false;
            }
            case 0x10: {
                addressingMode = "[" + baseReg + " + disp32]";
                hasDisp8 = false;
                hasDisp32 = true;
            }
            case 0x11: {
                addressingMode = baseReg;
                hasDisp8 = false;
                hasDisp32 = false;
            }
        }

        if (modByte == 0 && rmByte == 5) {
            addressingMode = "[RIP + disp32]";
            hasDisp8 = false;
            hasDisp32 = true;
        }
    }
};

struct SIB {
    unsigned char scaleByte;
    unsigned char indexByte;
    unsigned char baseByte;

    std::string address;
    bool hasDisp8;
    bool hasDisp32;

    SIB() {}
    SIB(unsigned char sibByte, bool rexb = false, unsigned char modByte = 0) {
        scaleByte = (sibByte >> 6) & 0x3;
        indexByte = (sibByte >> 3) & 0x7;
        baseByte = sibByte & 0x7;
    }
};

