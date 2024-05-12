/**
 * @file
 * @brief Defines structures for decoding x86 ModRM and SIB bytes.
 */

#pragma once
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "constants.h"
#include "utils.h"

/**
 * @struct REX
 * @brief Represents the REX prefix byte in x86 instruction encoding.
 */
struct REX {
    bool rexB;
    bool rexX;
    bool rexR;
    bool rexW;

    /**
     * @brief Default constructor for REX.
     */
    REX() : rexB(false), rexX(false), rexR(false), rexW(false) {}

    /**
     * @brief Constructor for REX with byte parameter.
     * @param rexByte The REX prefix byte.
     */
    REX(unsigned char rexByte) {
        rexB = (rexByte & 0x1) == 0x1;
        rexX = (rexByte & 0x2) == 0x2;
        rexR = (rexByte & 0x4) == 0x4;
        rexW = (rexByte & 0x8) == 0x8;
    }
};

/**
 * @struct ModRM
 * @brief Represents the ModRM byte in x86 instruction encoding.
 */
struct ModRM {
    REX rex;
    int modByte;
    int regByte;
    int rmByte;

    // std::string addressingMode;
    // std::string reg;

    bool hasDisp8;
    bool hasDisp32;
    bool hasSib;

    /**
     * @brief Default constructor for ModRM.
     */
    ModRM()
        : modByte(0),
          regByte(0),
          rmByte(0),
          hasDisp8(false),
          hasDisp32(false),
          hasSib(false) {}

    /**
     * @brief Constructor for ModRM with byte parameter.
     * @param modrmByte The ModRM byte.
     * @param rex The associated REX prefix.
     */
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
                break;
            }
            case 1: {
                hasDisp8 = true;
                hasDisp32 = false;
                break;
            }
            case 2: {
                hasDisp8 = false;
                hasDisp32 = true;
                break;
            }
            case 3: {
                hasDisp8 = false;
                hasDisp32 = false;
                break;
            }
        }

        if (modByte == 0 && rmByte == 5) {
            hasDisp8 = false;
            hasDisp32 = true;
        }
    }

    /**
     * @brief Gets the register name based on the operand type.
     * @param operand The operand type.
     * @return The register name.
     */
    std::string getReg(Operand operand) {
        if (operand == Operand::xmm) {
            return "xmm" + std::to_string(regByte + (rex.rexR ? 8 : 0));
        } else {
            return operand2register(operand)->at(regByte + (rex.rexR ? 8 : 0));
        }
    }

    /**
     * @brief Generates the addressing mode string.
     * @param operand The operand type.
     * @param disp8 The 8-bit displacement.
     * @param disp32 The 32-bit displacement.
     * @return The addressing mode string.
     */
    std::string getAddrMode(Operand operand, std::string disp8,
                            std::string disp32) {
        std::string addrBaseReg;
        if (modByte == 3) {
            if (operand == Operand::xm128) {
                addrBaseReg =
                    "xmm" + std::to_string(rmByte + (rex.rexB ? 8 : 0));
            } else {
                addrBaseReg =
                    operand2register(operand)->at(rmByte + (rex.rexB ? 8 : 0));
            }
        } else {
            if (operand == Operand::xm128) {
                addrBaseReg =
                    "xmm" + std::to_string(rmByte + (rex.rexB ? 8 : 0));
            } else {
                addrBaseReg = REGISTERS64.at(rmByte + (rex.rexB ? 8 : 0));
            }
        }

        if (modByte < 3 && rmByte == 4) {
            addrBaseReg = "SIB";
        }

        std::string addressingMode;
        switch (modByte) {
            case 0: {
                addressingMode = "[" + addrBaseReg + "]";
                break;
            }
            case 1: {
                addressingMode = "[" + addrBaseReg + disp8 + "]";
                break;
            }
            case 2: {
                addressingMode = "[" + addrBaseReg + disp32 + "]";
                break;
            }
            case 3: {
                addressingMode = addrBaseReg;
                break;
            }
        }

        if (modByte == 0 && rmByte == 5) {
            addressingMode = "[rip" + disp32 + "]";
        }

        return addressingMode;
    }
};

/**
 * @struct SIB
 * @brief Represents the SIB (Scale-Index-Base) byte in x86 instruction
 * encoding.
 */
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

    /**
     * @brief Default constructor for SIB.
     */
    SIB() : hasDisp8(false), hasDisp32(false) {}

    /**
     * @brief Constructor for SIB with byte parameters.
     * @param sibByte The SIB byte.
     * @param modByte The associated mod field from ModRM.
     * @param rex The associated REX prefix.
     */
    SIB(unsigned char sibByte, unsigned char modByte, REX rex)
        : modByte(modByte), rex(rex) {
        scaleByte = (sibByte >> 6) & 0x3;
        indexByte = (sibByte >> 3) & 0x7;
        baseByte = sibByte & 0x7;

        hasDisp8 = baseByte == 5 && modByte == 1;
        hasDisp32 = baseByte == 5 && modByte != 1;
    }

    /**
     * @brief Generates the address string.
     * @param operand The operand type.
     * @param disp8 The 8-bit displacement.
     * @param disp32 The 32-bit displacement.
     * @return The address string.
     */
    std::string getAddr(Operand operand, std::string disp8,
                        std::string disp32) {
        std::string offset = "";

        if (baseByte == 5) {
            switch (modByte) {
                case 0: {
                    if (disp32.size() > 2) {
                        if (disp32[1] == '+') {
                            addrBaseReg = disp32.substr(3, disp32.size() - 3);
                        } else {
                            addrBaseReg =
                                "-" + disp32.substr(3, disp32.size() - 3);
                        }
                    }
                    break;
                }
                case 1: {
                    addrBaseReg = rex.rexB ? "r13" : "rbp";
                    offset = disp8;
                    break;
                }
                case 2: {
                    addrBaseReg = rex.rexB ? "r13" : "rbp";
                    offset = disp32;
                    break;
                }
            }
        } else {
            addrBaseReg = REGISTERS64.at(baseByte + (rex.rexB ? 8 : 0));
            if (modByte == 1) {
                offset = disp8;
            } else if (modByte == 2) {
                offset = disp32;
            }
        }

        if (modByte == 0 && baseByte == 5 && indexByte == 4) {
            address = addrBaseReg;
        } else if (indexByte == 4 && (!rex.rexX)) {
            address = "[" + addrBaseReg + offset + "]";
        } else {
            indexReg = REGISTERS64.at(indexByte + (rex.rexX ? 8 : 0));
            scale = SCALE_FACTOR.at(scaleByte);
            address = "[" + addrBaseReg + " + " + indexReg + " * " +
                      std::to_string(scale) + offset + "]";
        }
        return address;
    }
};

