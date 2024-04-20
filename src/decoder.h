#include <cstdint>
#include <iostream>
#include <map>
#include <vector>

#include "operatorTable.h"
#include "state.h"

struct X86Decoder {
    DecoderState state;

    X86Decoder(DecoderState &decoderState) : state(decoderState) {}

    std::pair<Operand, uint64_t> decodeSingleInstruction() {
        std::vector<std::string> assemblyInstruction;
        size_t startIdx = state.getCurIdx();

        size_t instructionLen = 1;
        size_t prefixOffset = 0;

        uint8_t opcodeByte = state.objectSource[startIdx];
        uint8_t prefix, modrmByte, sibByte;
        prefix = modrmByte = sibByte = 0;

        if (opcodeByte in PREFIX_SET) {
            prefix = opcodeByte;
            prefixOffset = 1;
            instructionLen += 1;
            opcodeByte = state.objectSource[startIdx + prefixOffset];
        }

        auto operatorDict = OP_LOOKUP[{prefix, opcodeByte}];

        if ((startIdx + 1 + prefixOffset) < state.objectSource.size()) {
            modrmByte = state.objectSource[startIdx + 1 + prefixOffset];
        }

        if ((startIdx + 2 + prefixOffset) < state.objectSource.size()) {
            sibByte = state.objectSource[startIdx + 2 + prefixOffset];
        }

        Operator operator;

        if (modrmByte != 0) {
            uint8_t reg = modrm::getRegVal(modrmByte);
            operator=(reg in operatorDict) ? operatorDict[reg]
                                           : operatorDict[0];
        } else {
            operator= operatorDict[0];
        }

        assemblyInstruction.push_back(operator);

            auto [opEnc, remOps, operands] = OPERAND_LOOKUP[{operator, opcodeByte}];

            std::string log =
                "   Op[" + operator+ ":" + std::to_string(opcodeByte) +
                "] Prefix[" + std::to_string(prefix) + "] Enc[" +
                std::to_string(opEnc) + "] remOps[" + std::to_string(remOps) +
                "] Operands";

            std::vector<std::string> assemblyOperands;

            if (opEnc.hasModrm) {
                if (modrmByte == 0) {
                    throw std::runtime_error(
                        "Expected ModRM byte but there aren't any bytes left.");
                }
                instructionLen += 1;
                auto [modRmVals, modRmTrans] = modrm::translate(modrmByte);
            }

            if (opEnc.hasModrm && modRmTrans.hasSib) {
                if (sibByte == 0) {
                    throw std::runtime_error(
                        "Expected SIB byte but there aren't any bytes left.");
                }
                auto [sibVals, sibTrans] = sib::translate(sibByte);
                instructionLen += 1;
            }

            // parse displacement
            uint8_t disp8 = 0;
            std::vector<uint8_t> disp32;

            if (opEnc.hasModrm && modRmTrans.hasDisp8 ||
                opEnc.hasModrm && modRmTrans.hasSib && sibTrans.hasDisp8 ||
                opEnc.hasModrm && modRmTrans.hasSib && modRmVals.mod == 1 &&
                    sibVals.base == 5) {
                disp8 = state.objectSource[startIdx + instructionLen];
                instructionLen += 1;
            }

            if (opEnc.hasModrm && modRmTrans.hasDisp32 ||
                opEnc.hasModrm && modRmTrans.hasSib && sibTrans.hasDisp32 ||
                opEnc.hasModrm && modRmTrans.hasSib && modRmVals.mod in{0, 2} &&
                    sibVals.base == 5) {
                disp32 = {
                    state.objectSource.begin() + startIdx + instructionLen,
                    state.objectSource.begin() + startIdx + instructionLen + 4};
                std::reverse(disp32.begin(), disp32.end());
                instructionLen += 4;
            }

            // parse immediate
            std::vector<uint8_t> imm;
            if (remOps.contains("id")) {
                imm = {
                    state.objectSource.begin() + startIdx + instructionLen,
                    state.objectSource.begin() + startIdx + instructionLen + 4};
                std::reverse(imm.begin(), imm.end());
                instructionLen += 4;
            }

            // Add all of the processed arguments to the assembly instruction
            for (auto operand : operands) {
                std::string decodedTranslatedValue;

                if (operand == nullptr) break;

                if (operand == OpUnit.eax) decodedTranslatedValue = "eax";

                if (operand.name == OpUnit.rm.name ||
                    operand.name == OpUnit.reg.name) {
                    if (opEnc.hasModrm)
                        decodedTranslatedValue = modRmTrans.get(operand.name);
                    else
                        decodedTranslatedValue = REGISTER[remOps[0]];
                }

                if (operand.name == OpUnit.imm32.name) {
                    imm = {
                        state.objectSource.begin() + startIdx + instructionLen,
                        state.objectSource.begin() + startIdx + instructionLen +
                            4};
                    std::reverse(imm.begin(), imm.end());
                    instructionLen += 4;
                    decodedTranslatedValue = "0x" + imm;
                }

                // More conditions for other operand types...

                assemblyOperands.push_back(decodedTranslatedValue);
            }

            if (nullptr in assemblyOperands) throw InvalidTranslationValue();

            assemblyInstruction.push_back(std::join(", ", assemblyOperands));
            std::string assemblyInstructionStr =
                std::join(" ", assemblyInstruction);

            uint64_t targetAddr = state.markDecoded(startIdx, instructionLen,
                                                    assemblyInstructionStr);

            return std::make_pair(operator, targetAddr);
    }
};
