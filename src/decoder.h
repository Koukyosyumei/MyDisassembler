#pragma once
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "constants.h"
#include "modrm.h"
#include "table.h"
#include "state.h"

struct X86Decoder {
    DecoderState state;
    bool hasREX;
    // bool hasModRM;
    bool hasSIB;

    size_t startIdx;
    size_t curIdx;
    size_t instructionLen;
    size_t prefixOffset;

    int prefixInstructionByte, opcodeByte, modrmByte, sibByte;

    Mnemonic mnemonic;
    Prefix prefix;
    REX rex;
    ModRM modrm;
    SIB sib;

    OpEnc opEnc;
    std::vector<std::string> remOps;
    std::vector<Operand> operands;

    uint8_t disp8 = -1;
    std::vector<uint8_t> disp32;

    std::vector<std::string> assemblyInstruction;
    std::vector<std::string> assemblyOperands;

    X86Decoder(DecoderState& decoderState)
        : state(decoderState), hasREX(false), hasSIB(false) {}

    void parsePrefixInstructions() {
        int startByte = state.objectSource[curIdx];
        if (PREFIX_INSTRUCTIONS_BYTES_SET.find(startByte) !=
            PREFIX_INSTRUCTIONS_BYTES_SET.end()) {
            // eat prefix
            prefixInstructionByte = startByte;
            prefixOffset = 1;
            instructionLen += 1;
            curIdx += 1;
        }
    }

    void parseREX() {
        // The format of REX prefix is 0100|W|R|X|B
        if ((state.objectSource[curIdx] >> 4) == 4) {
            hasREX = true;
            rex = REX(state.objectSource[curIdx]);
            instructionLen += 1;
            curIdx += 1;
        }
    }

    void parseOpecode() {
        // eat opecode
        opcodeByte = state.objectSource[curIdx];
        instructionLen += 1;
        curIdx += 1;

        // (prefix, opcode) -> (reg, mnemonic)
        std::unordered_map<int, Mnemonic> reg2mnem =
            OP_LOOKUP.at(std::make_pair(prefix, opcodeByte));

        // We sometimes need reg of modrm to determine the opecode
        // e.g. 83 /4 -> AND
        //      83 /1 -> OR
        if (curIdx < state.objectSource.size()) {
            modrmByte = state.objectSource[curIdx];
        }

        if (modrmByte != 0) {
            uint8_t reg = getRegVal(modrmByte);
            mnemonic = (reg2mnem.find(reg) != reg2mnem.end()) ? reg2mnem[reg]
                                                              : reg2mnem[-1];
        } else {
            mnemonic = reg2mnem[-1];
        }

        assemblyInstruction.push_back(to_string(mnemonic));
        std::tuple<OpEnc, std::vector<std::string>, std::vector<Operand>> res =
            OPERAND_LOOKUP.at(std::make_tuple(prefix, mnemonic, opcodeByte));
        opEnc = std::get<0>(res);
        remOps = std::get<1>(res);
        operands = std::get<2>(res);
    }

    void parseModRM() {
        if (hasModrm(opEnc)) {
            if (modrmByte == 0) {
                throw std::runtime_error(
                    "Expected ModRM byte but there aren't any bytes left.");
            }
            instructionLen += 1;
            curIdx += 1;
            modrm = ModRM(modrmByte);
        }
    }

    void parseSIB() {
        if (hasModrm(opEnc) && modrm.hasSib) {
            // eat the sib (1 byte)
            if (curIdx < state.objectSource.size()) {
                sibByte = state.objectSource[curIdx];
            }
            if (sibByte == 0) {
                throw std::runtime_error(
                    "Expected SIB byte but there aren't any bytes left.");
            }
            sib = SIB(sibByte);
            instructionLen += 1;
            curIdx += 1;
        }
    }

    void parseAddressOffset() {
        if ((hasModrm(opEnc) && modrm.hasDisp8) ||
            (hasModrm(opEnc) && modrm.hasSib && sib.hasDisp8) ||
            (hasModrm(opEnc) && modrm.hasSib && modrm.modByte == 1 &&
             sib.baseByte == 5)) {
            disp8 = state.objectSource[curIdx];
            instructionLen += 1;
            curIdx += 1;
        }

        if ((hasModrm(opEnc) && modrm.hasDisp32) ||
            (hasModrm(opEnc) && modrm.hasSib && sib.hasDisp32) ||
            (hasModrm(opEnc) && modrm.hasSib &&
             (modrm.modByte == 0 || modrm.modByte == 2) && sib.baseByte == 5)) {
            disp32 =
                std::vector<uint8_t>(state.objectSource.begin() + curIdx,
                                     state.objectSource.begin() + curIdx + 4);
            std::reverse(disp32.begin(), disp32.end());
            instructionLen += 4;
            curIdx += 4;
        }
    }

    std::pair<std::string, uint64_t> decodeSingleInstruction() {
        // ############### Initialize ##############################
        startIdx = curIdx = state.getCurIdx();
        instructionLen = 1;
        prefixOffset = 0;
        opcodeByte = modrmByte = sibByte = 0;

        // the general format of the x86-64 operations
        // |prefix|REX prefix|opecode|ModR/M|SIB|address offset|immediate|

        parsePrefixInstructions();
        parseREX();
        parseOpecode();
        parseModRM();
        parseSIB();
        parseAddressOffset();

        // ############### Process Operands ################
        std::vector<uint8_t> imm;
        for (Operand& operand : operands) {
            std::string decodedTranslatedValue;

            // if (operand == nullptr) break;

            if (operand == Operand::eax) {
                decodedTranslatedValue = "eax";
            } else if (operand == Operand::rax) {
                decodedTranslatedValue = "rax";
            } else if (operand == Operand::rm || operand == Operand::reg) {
                if (hasModrm(opEnc))
                    if (operand == Operand::rm)
                        decodedTranslatedValue = modrm.addressingMode;
                    else
                        decodedTranslatedValue = modrm.reg;
                else
                    decodedTranslatedValue =
                        id2register.at(std::stoi(remOps[0]));
            } else if (operand == Operand::imm32) {
                imm = std::vector<uint8_t>(
                    state.objectSource.begin() + curIdx,
                    state.objectSource.begin() + curIdx + 4);
                std::reverse(imm.begin(), imm.end());
                instructionLen += 4;
                curIdx += 4;

                std::stringstream ss;
                ss << "0x";
                for (unsigned char x : imm) {
                    ss << std::hex << std::setw(2) << std::setfill('0')
                       << static_cast<int>(x);
                }
                decodedTranslatedValue = "0x" + ss.str();
            } else if (operand == Operand::imm16) {
                imm = std::vector<uint8_t>(
                    state.objectSource.begin() + curIdx,
                    state.objectSource.begin() + curIdx + 2);
                std::reverse(imm.begin(), imm.end());
                instructionLen += 2;
                curIdx += 2;

                std::stringstream ss;
                ss << "0x";
                for (unsigned char x : imm) {
                    ss << std::hex << std::setw(2) << std::setfill('0')
                       << static_cast<int>(x);
                }
                decodedTranslatedValue = "0x" + ss.str();
            } else if (operand == Operand::imm8) {
                imm = std::vector<uint8_t>(
                    state.objectSource.begin() + curIdx,
                    state.objectSource.begin() + curIdx + 1);
                std::reverse(imm.begin(), imm.end());
                instructionLen += 1;
                curIdx += 1;

                std::stringstream ss;
                ss << "0x";
                for (unsigned char x : imm) {
                    ss << std::hex << std::setw(2) << std::setfill('0')
                       << static_cast<int>(x);
                }
                decodedTranslatedValue = "0x" + ss.str();
            }

            if (hasModrm(opEnc) && modrm.hasSib) {
                // sib.address
            }

            if (disp8 > 0) {
                decodedTranslatedValue += " disp8=" + std::to_string(disp8);
            }

            if (disp32.size() > 0) {
                decodedTranslatedValue += " disp32=<UNIMPLEMENTED>";
            }

            assemblyOperands.push_back(decodedTranslatedValue);
        }

        // if (nullptr in assemblyOperands) throw InvalidTranslationValue();

        std::string ao = "";
        for (std::string& a : assemblyOperands) {
            ao += " " + a;
        }
        assemblyInstruction.push_back(ao);

        std::string assemblyInstructionStr = "";
        for (std::string& a : assemblyInstruction) {
            assemblyInstructionStr += " " + a;
        }

        uint64_t targetAddr =
            state.markDecoded(startIdx, instructionLen, assemblyInstructionStr);

        return std::make_pair(to_string(mnemonic), targetAddr);
    }
};
