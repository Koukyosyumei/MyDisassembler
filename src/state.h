#pragma once
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "constants.h"
#include "error.h"
#include "bytes.h"
#include "table.h"

inline long long decodeOffset(const std::string& val) {
    long long offset;

    offset = stoul(val, nullptr, 16);

    if (val.size() <= 2 + 2) {
        if (offset > 0x7F) {
            offset -= 0x100;
        }
    } else if (val.size() <= 4 + 2) {
        if (offset > 0x7FFF) {
            offset -= 0x10000;
        }
    } else if (val.size() <= 8 + 2) {
        if (offset > 0x7FFFFFFF) {
            offset -= 0x100000000;
        }
    }

    return offset;
}

typedef struct {
    uint64_t startAddr;
    uint64_t instructionLen;
    Mnemonic mnemonic;
    std::string assemblyInstructionStr;
    long long nextOffset;
} DisassembledInstruction;

struct State {
    const std::vector<unsigned char>& objectSource;
    const std::unordered_map<uint64_t, std::string>& addr2symbol;

    bool hasInstructionPrefix, hasREX, hasSIB, hasDisp8, hasDisp32;
    uint64_t curAddr, instructionLen, prefixOffset;
    int instructionPrefixByte, opcodeByte, modrmByte, sibByte;

    std::string prefixInstructionStr;
    Mnemonic mnemonic;
    Prefix prefix;
    REX rex;
    ModRM modrm;
    SIB sib;

    OpEnc opEnc;
    std::vector<std::string> remOps;
    std::vector<Operand> operands;

    std::string disp8, disp32;

    std::vector<std::string> assemblyInstruction;
    std::vector<std::string> assemblyOperands;

    State(const std::vector<unsigned char>& objectSource,
          const std::unordered_map<uint64_t, std::string>& addr2symbol)
        : objectSource(objectSource),
          addr2symbol(addr2symbol),
          hasInstructionPrefix(false),
          hasREX(false),
          hasSIB(false),
          hasDisp8(false),
          hasDisp32(false),
          curAddr(0),
          instructionLen(0),
          prefixOffset(0),
          prefix(Prefix::NONE) {}

    void init() {
        hasInstructionPrefix = hasREX = hasSIB = hasDisp8 = hasDisp32 = false;
        curAddr = 0;
        instructionLen = 0;
        prefixOffset = 0;
        prefix = Prefix::NONE;

        opcodeByte = modrmByte = sibByte = -1;

        remOps.clear();
        remOps.shrink_to_fit();
        operands.clear();
        operands.shrink_to_fit();

        assemblyInstruction.clear();
        assemblyInstruction.shrink_to_fit();
        assemblyOperands.clear();
        assemblyOperands.shrink_to_fit();
    }

    bool parseEndBr() {
        if (curAddr + 3 < objectSource.size()) {
            if (objectSource[curAddr] == 0xF3 &&
                objectSource[curAddr + 1] == 0x0F &&
                objectSource[curAddr + 2] == 0x1E &&
                objectSource[curAddr + 3] == 0xFA) {
                assemblyInstruction.push_back(to_string(Mnemonic::ENDBR64));
                opEnc = OpEnc::NP;
                instructionLen += 4;
                curAddr += 4;
                return true;
            } else if (objectSource[curAddr] == 0xF3 &&
                       objectSource[curAddr + 1] == 0x0F &&
                       objectSource[curAddr + 2] == 0x1E &&
                       objectSource[curAddr + 3] == 0xFB) {
                assemblyInstruction.push_back(to_string(Mnemonic::ENDBR32));
                opEnc = OpEnc::NP;
                instructionLen += 4;
                curAddr += 4;
                return true;
            } else {
                return false;
            }
        }
        return false;
    }

    void parsePrefix() {
        if (objectSource[curAddr] == 0x66) {
            prefix = Prefix::P66;
            instructionLen += 1;
            curAddr += 1;
        }
    }

    void parsePrefixInstructions() {
        if (INSTRUCTION_PREFIX_SET.find(objectSource[curAddr]) !=
            INSTRUCTION_PREFIX_SET.end()) {
            hasInstructionPrefix = true;
            instructionPrefixByte = objectSource[curAddr];
            // prefixInstructionStr = PREFIX_INSTRUCTIONS_SET.at(startByte);
            // assemblyInstruction.push_back(prefixInstructionStr);
            prefixOffset = 1;
            instructionLen += 1;
            curAddr += 1;
        }
    }

    void parseREX() {
        // The format of REX prefix is 0100|W|R|X|B
        if ((objectSource[curAddr] >> 4) == 4) {
            hasREX = true;
            rex = REX(objectSource[curAddr]);
            instructionLen += 1;
            curAddr += 1;

            if (rex.rexW) {
                prefix = Prefix::REXW;
            } else {
                prefix = Prefix::REX;
            }
        }
    }

    void parseOpecode() {
        // eat opecode
        opcodeByte = objectSource[curAddr];
        instructionLen += 1;
        curAddr += 1;

        if (TWO_BYTES_OPCODE_PREFIX.find(opcodeByte) !=
            TWO_BYTES_OPCODE_PREFIX.end()) {
            opcodeByte = (opcodeByte << 8) + objectSource[curAddr];
            instructionLen += 1;
            curAddr += 1;
        }

        // (prefix, opcode) -> (reg, mnemonic)
        std::unordered_map<int, Mnemonic> reg2mnem;
        if (OP_LOOKUP.find(std::make_pair(prefix, opcodeByte)) !=
            OP_LOOKUP.end()) {
            reg2mnem = OP_LOOKUP.at(std::make_pair(prefix, opcodeByte));
        } else if (prefix == Prefix::REXW &&
                   OP_LOOKUP.find(std::make_pair(Prefix::REX, opcodeByte)) !=
                       OP_LOOKUP.end()) {
            reg2mnem = OP_LOOKUP.at(std::make_pair(Prefix::REX, opcodeByte));
            prefix = Prefix::REX;
        } else if (prefix == Prefix::REX &&
                   OP_LOOKUP.find(std::make_pair(Prefix::NONE, opcodeByte)) !=
                       OP_LOOKUP.end()) {
            reg2mnem = OP_LOOKUP.at(std::make_pair(Prefix::NONE, opcodeByte));
            prefix = Prefix::NONE;
        } else {
            std::stringstream ss;
            ss << std::hex << opcodeByte;
            throw OPCODE_LOOKUP_ERROR(
                "Unknown combination of the prefix and the opcodeByte: (" +
                to_string(prefix) + ", " + ss.str() + ")");
        }

        // We sometimes need reg of modrm to determine the opecode
        // e.g. 83 /4 -> AND
        //      83 /1 -> OR
        if (curAddr < objectSource.size()) {
            modrmByte = objectSource[curAddr];
        }

        if (modrmByte >= 0) {
            int reg = getRegVal(modrmByte);
            mnemonic = (reg2mnem.find(reg) != reg2mnem.end()) ? reg2mnem.at(reg)
                                                              : reg2mnem.at(-1);
        } else {
            mnemonic = reg2mnem.at(-1);
        }

        if (hasInstructionPrefix) {
            if (instructionPrefixByte == 0xF0) {
                assemblyInstruction.push_back("lock");
            } else if (instructionPrefixByte == 0xF2) {
                if (isControlFlowInstruction(mnemonic)) {
                    assemblyInstruction.push_back("bnd");
                } else {
                    assemblyInstruction.push_back("repne");
                }
            } else if (instructionPrefixByte == 0xF3) {
                assemblyInstruction.push_back("rep");
            }
        }

        assemblyInstruction.push_back(to_string(mnemonic));

        if (OPERAND_LOOKUP.find(std::make_tuple(
                prefix, mnemonic, opcodeByte)) != OPERAND_LOOKUP.end()) {
            std::tuple<OpEnc, std::vector<std::string>, std::vector<Operand>>
                res = OPERAND_LOOKUP.at(
                    std::make_tuple(prefix, mnemonic, opcodeByte));
            opEnc = std::get<0>(res);
            remOps = std::get<1>(res);
            operands = std::get<2>(res);
        } else {
            std::stringstream ss;
            ss << std::hex << opcodeByte;
            throw OPERAND_LOOKUP_ERROR(
                "Unknown combination of prefix, mnemonic and opcodeByte: (" +
                to_string(prefix) + ", " + to_string(mnemonic) + ", " +
                ss.str() + ")");
        }
    }

    void parseModRM() {
        if (hasModrm(opEnc)) {
            if (modrmByte < 0) {
                throw std::runtime_error(
                    "Expected ModRM byte but there aren't any bytes left.");
            }
            instructionLen += 1;
            curAddr += 1;
            modrm = ModRM(modrmByte, rex);
        }
    }

    void parseSIB() {
        if (hasModrm(opEnc) && modrm.hasSib) {
            // eat the sib (1 byte)
            if (curAddr < objectSource.size()) {
                sibByte = objectSource[curAddr];
            }
            if (sibByte < 0) {
                throw std::runtime_error(
                    "Expected SIB byte but there aren't any bytes left.");
            }
            sib = SIB(sibByte, modrm.modByte, rex);
            instructionLen += 1;
            curAddr += 1;
        }
    }

    void parseAddressOffset() {
        if ((hasModrm(opEnc) && modrm.hasDisp8) ||
            (hasModrm(opEnc) && modrm.hasSib && sib.hasDisp8) ||
            (hasModrm(opEnc) && modrm.hasSib && modrm.modByte == 1 &&
             sib.baseByte == 5)) {
            
            std::stringstream ss1;
            ss1 << std::hex << (int)objectSource[curAddr];
            disp8 = "0x" + ss1.str();

            long long decoded_disp8 = decodeOffset(disp8);
            if (decoded_disp8 < 0) {
                std::stringstream ss2;
                ss2 << std::hex << (-1 * decoded_disp8);
                disp8 = " - 0x" + ss2.str();
            } else {
                disp8 = " + " + disp8;
            }

            hasDisp8 = true;
            instructionLen += 1;
            curAddr += 1;
        }

        if ((hasModrm(opEnc) && modrm.hasDisp32) ||
            (hasModrm(opEnc) && modrm.hasSib && sib.hasDisp32) ||
            (hasModrm(opEnc) && modrm.hasSib &&
             (modrm.modByte == 0 || modrm.modByte == 2) && sib.baseByte == 5)) {
            std::vector<uint8_t> _disp32 =
                std::vector<uint8_t>(objectSource.begin() + curAddr,
                                     objectSource.begin() + curAddr + 4);
            std::reverse(_disp32.begin(), _disp32.end());
            std::stringstream ss;
            ss << "0x";
            for (unsigned char x : _disp32) {
                ss << std::hex << std::setw(2) << std::setfill('0')
                   << static_cast<int>(x);
            }
            disp32 = ss.str();

            long long decoded_disp32 = decodeOffset(disp32);
            if (decoded_disp32 < 0) {
                std::stringstream ss;
                ss << std::hex << (-1 * decoded_disp32);
                disp32 = " - 0x" + ss.str();
            } else {
                disp32 = " + " + disp32;
            }

            hasDisp32 = true;
            instructionLen += 4;
            curAddr += 4;
        }
    }

    // startAddr, targetLen, mnemonic, assemblyStr, nextOffset
    DisassembledInstruction step(uint64_t startAddr) {
        // ############### Initialize ##############################
        init();
        curAddr = startAddr;

        // the general format of the x86-64 operations
        // |prefix|REX prefix|opecode|ModR/M|SIB|address offset|immediate|

        if (!parseEndBr()) {
            parsePrefixInstructions();
            parsePrefix();
            parseREX();
            parseOpecode();
            parseModRM();
            parseSIB();
            parseAddressOffset();
        }

        // ############### Process Operands ################
        std::vector<uint8_t> imm;
        for (Operand operand : operands) {
            std::string decodedTranslatedValue;

            if (isA_REG(operand) || operand == Operand::cl ||
                operand == Operand::dx) {
                decodedTranslatedValue = to_string(operand);
            } else if (isRM(operand) || isREG(operand)) {
                if (hasModrm(opEnc)) {
                    if (isRM(operand)) {
                        decodedTranslatedValue =
                            modrm.getAddrMode(operand, disp8, disp32);
                    } else {
                        decodedTranslatedValue = modrm.getReg(operand);
                    }
                } else {
                    if (is8Bit(operand)) {
                        decodedTranslatedValue =
                            REGISTERS8.at(std::stoi(remOps[0]));
                    } else if (is16Bit(operand)) {
                        decodedTranslatedValue =
                            REGISTERS16.at(std::stoi(remOps[0]));
                    } else if (is32Bit(operand)) {
                        decodedTranslatedValue =
                            REGISTERS32.at(std::stoi(remOps[0]));
                    } else if (is64Bit(operand)) {
                        decodedTranslatedValue =
                            REGISTERS64.at(std::stoi(remOps[0]));
                    }
                }

                if (isRM(operand) && hasModrm(opEnc) && modrm.hasSib) {
                    decodedTranslatedValue =
                        sib.getAddr(operand, disp8, disp32);
                }
            } else if (isIMM(operand)) {
                int immSize = 0;
                if (operand == Operand::imm64) {
                    immSize = 8;
                } else if (operand == Operand::imm32) {
                    immSize = 4;
                } else if (operand == Operand::imm16) {
                    immSize = 2;
                } else if (operand == Operand::imm8) {
                    immSize = 1;
                }
                imm = std::vector<uint8_t>(
                    objectSource.begin() + curAddr,
                    objectSource.begin() + curAddr + immSize);
                std::reverse(imm.begin(), imm.end());
                instructionLen += immSize;
                curAddr += immSize;

                std::stringstream ss;
                ss << "0x";
                for (unsigned char x : imm) {
                    ss << std::hex << std::setw(2) << std::setfill('0')
                       << static_cast<int>(x);
                }
                decodedTranslatedValue = ss.str();
            }

            assemblyOperands.push_back(decodedTranslatedValue);
        }

        std::string ao = "";
        for (std::string& a : assemblyOperands) {
            ao += " " + a;
        }
        assemblyInstruction.push_back(ao);

        long long nextOffset = 0;
        std::string assemblyInstructionStr = "";

        if (isControlFlowInstruction(mnemonic) && operands.size() == 1 &&
            isIMM(operands[0])) {
            nextOffset = decodeOffset(assemblyOperands[0]);
            uint64_t labelAddr =
                (uint64_t)(((long long)startAddr) +
                           ((long long)instructionLen) + nextOffset);
            std::stringstream ss;
            ss << std::hex << labelAddr;
            std::string labelName = ss.str();
            if (addr2symbol.find(labelAddr) != addr2symbol.end()) {
                labelName += " <" + addr2symbol.at(labelAddr) + ">";
            }
            assemblyInstructionStr =
                to_string(mnemonic) + " " + labelName +
                " ; relative offset = " + std::to_string(nextOffset);
        } else {
            assemblyInstructionStr = assemblyInstruction[0];
            for (int i = 1; i < assemblyInstruction.size(); i++) {
                assemblyInstructionStr += " " + assemblyInstruction[i];
            }
        }

        return {startAddr, instructionLen, mnemonic, assemblyInstructionStr,
                nextOffset};
    }
};
