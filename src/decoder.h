S #include<cstdint>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "constants.h"
#include "modrm.h"
#include "operatorTable.h"
#include "state.h"

    struct X86Decoder {
    DecoderState state;

    X86Decoder(DecoderState& decoderState) : state(decoderState) {}

    std::pair<std::string, uint64_t> decodeSingleInstruction() {
        std::vector<std::string> assemblyInstruction;
        size_t startIdx = state.getCurIdx();

        size_t instructionLen = 1;
        size_t prefixOffset = 0;

        int startByte = state.objectSource[startIdx];
        int opcodeByte, prefix, modrmByte, sibByte;
        prefix = modrmByte = sibByte = 0;

        // the general format of the x86-64 operations
        // |prefix|REX prefix|opecode|ModR/M|SIB|address offset|immediate|

        if (PREFIX_SET.find(startByte) != PREFIX_SET.end()) {
            prefix = startByte;
            prefixOffset = 1;
            instructionLen += 1;
            opcodeByte = state.objectSource[startIdx + prefixOffset];
        } else {
            opcodeByte = startByte;
        }

        // (prefix, opcode) -> (register, operator)
        std::unordered_map<int, std::string> operatorDict =
            OP_LOOKUP.at(std::make_pair(prefix, opcodeByte));

        // Grab the modrm (1 byte)
        if ((startIdx + 1 + prefixOffset) < state.objectSource.size()) {
            modrmByte = state.objectSource[startIdx + 1 + prefixOffset];
        }

        // Grab the sib (1 byte)
        if ((startIdx + 2 + prefixOffset) < state.objectSource.size()) {
            sibByte = state.objectSource[startIdx + 2 + prefixOffset];
        }

        std::string operator_;
        if (modrmByte != 0) {
            uint8_t reg = getRegVal(modrmByte);
            operator_ = (operatorDict.find(reg) != operatorDict.end())
                            ? operatorDict[reg]
                            : operatorDict[0];
        } else {
            operator_ = operatorDict[0];
        }

        assemblyInstruction.push_back(operator_);

        // (operator, opcode) -> (encoding, mnemonic, operands)
        std::tuple<OpEnc, std::string, std::vector<Operand>> res =
            OPERAND_LOOKUP.at(std::make_pair(operator_, opcodeByte));
        OpEnc opEnc = std::get<0>(res);
        std::string remOps = std::get<1>(res);
        std::vector<Operand> operands = std::get<2>(res);

        std::string log =
            "   Op[" + operator_ + ":" + std::to_string(opcodeByte) +
            "] Prefix[" + std::to_string(prefix) + "] Enc[" +
            getInfo(opEnc).opEnc + "] remOps[" + remOps + "] Operands";

        std::vector<std::string> assemblyOperands;
        ModRMVal modRmVals;
        SibVal sibVals;
        ModRMTrans modRmTrans;
        SibTrans sibTrans;
        
        // Process the MODRM
        if (getInfo(opEnc).hasModrm) {
            if (modrmByte == 0) {
                throw std::runtime_error(
                    "Expected ModRM byte but there aren't any bytes left.");
            }
            instructionLen += 1;
            std::tuple<ModRMVal, ModRMTrans> res = translateModRm(modrmByte);
            modRmVals = std::get<0>(res);
            modRmTrans = std::get<1>(res);
        }

        if (getInfo(opEnc).hasModrm && modRmTrans.hasSib) {
            if (sibByte == 0) {
                throw std::runtime_error(
                    "Expected SIB byte but there aren't any bytes left.");
            }
            std::tuple<SibVal, SibTrans> res = translateSib(sibByte);
            sibVals = std::get<0>(res);
            sibTrans = std::get<1>(res);
            instructionLen += 1;
        }

        // parse displacement
        uint8_t disp8 = 0;
        std::vector<uint8_t> disp32;

        if ((getInfo(opEnc).hasModrm && modRmTrans.hasDisp8) ||
            (getInfo(opEnc).hasModrm && modRmTrans.hasSib &&
             sibTrans.hasDisp8) ||
            (getInfo(opEnc).hasModrm && modRmTrans.hasSib &&
             modRmVals.mod == 1 && sibVals.base == 5)) {
            disp8 = state.objectSource[startIdx + instructionLen];
            instructionLen += 1;
        }

        if ((getInfo(opEnc).hasModrm && modRmTrans.hasDisp32) ||
            (getInfo(opEnc).hasModrm && modRmTrans.hasSib &&
             sibTrans.hasDisp32) ||
            (getInfo(opEnc).hasModrm && modRmTrans.hasSib &&
             (modRmVals.mod == 0 || modRmVals.mod == 2) && sibVals.base == 5)) {
            disp32 = {
                state.objectSource.begin() + startIdx + instructionLen,
                state.objectSource.begin() + startIdx + instructionLen + 4};
            std::reverse(disp32.begin(), disp32.end());
            instructionLen += 4;
        }

        // parse immediate
        std::vector<uint8_t> imm;
        if (remOps.find("id") != std::string::npos) {
            imm = {state.objectSource.begin() + startIdx + instructionLen,
                   state.objectSource.begin() + startIdx + instructionLen + 4};
            std::reverse(imm.begin(), imm.end());
            instructionLen += 4;
        }

        // Add all of the processed arguments to the assembly instruction
        for (Operand& operand : operands) {
            std::string decodedTranslatedValue;

            // if (operand == nullptr) break;

            if (operand.type == OpUnit::eax) {
                decodedTranslatedValue = "eax";
            }

            if (operand.type == OpUnit::rm || operand.type == OpUnit::reg) {
                if (getInfo(opEnc).hasModrm)
                    decodedTranslatedValue = modRmTrans.get(operand.name);
                else
                    decodedTranslatedValue = REGISTERS[remOps[0]];
            }

            if (operand.type == OpUnit::imm32) {
                imm = {
                    state.objectSource.begin() + startIdx + instructionLen,
                    state.objectSource.begin() + startIdx + instructionLen + 4};
                std::reverse(imm.begin(), imm.end());
                instructionLen += 4;
                decodedTranslatedValue = "0x" + imm;
            }

            // More conditions for other operand types...

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

        return std::make_pair(operator_, targetAddr);
    }
};
