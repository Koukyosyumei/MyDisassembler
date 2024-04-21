#pragma once
#include <iostream>
#include <unordered_map>

#include "constants.h"

// Define ModRMVal struct
struct ModRMVal {
    unsigned char mod;
    unsigned char reg;
    unsigned char rm;
};

// Define ModRMTrans struct
struct ModRMTrans {
    unsigned char reg;
    unsigned char rm;
    bool hasDisp8;
    bool hasDisp32;
    bool hasSib;
};

// Define SibVal struct
struct SibVal {
    unsigned char scale;
    unsigned char index;
    unsigned char base;
};

// Define SibTrans struct
struct SibTrans {
    bool scaledIndexBase;
    bool hasDisp8;
    bool hasDisp32;
};

// Define MODRM_PARSE_LOOKUP using a map
const std::unordered_map<unsigned char, ModRMVal> MODRM_PARSE_LOOKUP;

// Define MODRM_TRANSLATION_LOOKUP using a map
const std::unordered_map<unsigned char, ModRMTrans> MODRM_TRANSLATION_LOOKUP;

// Define SIB_PARSE_LOOKUP using a map
const std::unordered_map<unsigned char, SibVal> SIB_PARSE_LOOKUP;

// Define SIB_TRANSLATION_LOOKUP using a map
const std::unordered_map<unsigned char, SibTrans> SIB_TRANSLATION_LOOKUP;

// Function to get ModRMVal from modrm
inline ModRMVal _getModrmValues(unsigned char modrm) {
    return MODRM_PARSE_LOOKUP.at(modrm);
}

// Function to get ModRMTrans from modrm
inline ModRMTrans _getModRmTranslation(unsigned char modrm) {
    // Implement your logic here
}

// Function to get SibVal from sib
inline SibVal _getSibValues(unsigned char sib) {
    return SIB_PARSE_LOOKUP.at(sib);
}

// Function to get SibTrans from sib
inline SibTrans _getSibTranslation(unsigned char sib) {
    // Implement your logic here
}

// Function to translate modrmByte
inline std::tuple<ModRMVal, ModRMTrans> translateModRm(unsigned char modrmByte) {
    return std::make_tuple(_getModrmValues(modrmByte), _getModRmTranslation(modrmByte));
}

// Function to translate sibByte
inline std::tuple<SibVal, SibTrans> translateSib(unsigned char sibByte) {
    return std::make_tuple(_getSibValues(sibByte), _getSibTranslation(sibByte));
}

inline unsigned char assembleModRmByte(unsigned char mod, unsigned char reg, unsigned char rm) {
    return ((mod << 6) + (reg << 3) + rm);
}

inline unsigned char assembleSibByte(unsigned char scale, unsigned char index, unsigned char base) {
    return ((scale << 6) + (index << 3) + base);
}

inline unsigned char getRegVal(unsigned char modrmByte) {
    return (modrmByte >> 3) & 0x7;
}
