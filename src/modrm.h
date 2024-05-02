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

    REX() : rexB(false), rexX(false), rexR(false), rexW(false) {}
    REX(unsigned char rexByte) {
        rexB = (rexByte & 0x1) == 0x1;
        rexX = (rexByte & 0x2) == 0x2;
        rexR = (rexByte & 0x4) == 0x4;
        rexW = (rexByte & 0x8) == 0x8;
    }
};

struct ModRM {
    REX rex;
    int modByte;
    int regByte;
    int rmByte;

    std::string addressingMode;
    std::string reg;

    bool hasDisp8;
    bool hasDisp32;
    bool hasSib;

    ModRM()
        : modByte(0),
          regByte(0),
          rmByte(0),
          hasDisp8(false),
          hasDisp32(false),
          hasSib(false) {}
    ModRM(unsigned char modrmByte, REX rex) : rex(rex) {
        rmByte = modrmByte & 0x7;
        regByte = (modrmByte >> 3) & 0x7;
        modByte = (modrmByte >> 6) & 0x3;
        hasSib = false;

        if (modByte < 3 && rmByte == 4) {
            hasSib = true;
        }
        switch (modByte) {
            case 0: {
                hasDisp8 = false;
                hasDisp32 = false;
            }
            case 1: {
                hasDisp8 = true;
                hasDisp32 = false;
            }
            case 2: {
                hasDisp8 = false;
                hasDisp32 = true;
            }
            case 3: {
                hasDisp8 = false;
                hasDisp32 = false;
            }
        }

        if (modByte == 0 && rmByte == 5) {
            hasDisp8 = false;
            hasDisp32 = true;
        }

        std::cout << rmByte << " - " << regByte << " - " << modByte << " "
                  << std::endl;
    }

    std::string getReg(Operand operand) {
        return operand2register(operand)->at(regByte + (rex.rexR ? 8 : 0));
    }

    std::string getAddrMode(Operand operand) {
        std::string addrBaseReg =
            operand2register(operand)->at(rmByte + (rex.rexB ? 8 : 0));

        if (modByte < 3 && rmByte == 4) {
            addrBaseReg = "SIB";
        }

        switch (modByte) {
            case 0: {
                addressingMode = "[" + addrBaseReg + "]";
            }
            case 1: {
                addressingMode = "[" + addrBaseReg + " + disp8]";
            }
            case 2: {
                addressingMode = "[" + addrBaseReg + " + disp32]";
            }
            case 3: {
                addressingMode = addrBaseReg;
            }
        }

        std::cout << addressingMode << " ? " << hasDisp8 << " ? " << hasDisp32
                  << std::endl;

        if (modByte == 0 && rmByte == 5) {
            addressingMode = "[RIP + disp32]";
        }

        return addressingMode;
    }
};

struct SIB {
    unsigned char scaleByte;
    unsigned char indexByte;
    unsigned char baseByte;
    unsigned char modByte;
    REX rex;

    std::string address;

    std::string addrBaseReg, indexReg;
    int scale;
    bool hasDisp8;
    bool hasDisp32;

    SIB() {}
    SIB(unsigned char sibByte, unsigned char modByte, REX rex)
        : modByte(modByte), rex(rex) {
        scaleByte = (sibByte >> 6) & 0x3;
        indexByte = (sibByte >> 3) & 0x7;
        baseByte = sibByte & 0x7;
    }

    std::string getAddr(Operand operand, std::string disp8,
                        std::string disp32) {
        if (baseByte == 5) {
            switch (modByte) {
                case 0:
                    addrBaseReg = disp32;
                case 1:
                    addrBaseReg =
                        rex.rexB ? "RBP + " + disp8 : "R13 + " + disp8;
                case 2:
                    addrBaseReg =
                        rex.rexB ? "RBP + " + disp32 : "R13 + " + disp32;
            }
        } else {
            addrBaseReg =
                operand2register(operand)->at(baseByte + (rex.rexB ? 8 : 0));
        }

        indexReg =
            operand2register(operand)->at(indexByte + (rex.rexB ? 8 : 0));
        scale = SCALE_FACTOR.at(scaleByte);
        address = "[" + addrBaseReg + " + " + indexReg + " * " +
                  std::to_string(scale) + "]";
        return address;
    }
};

