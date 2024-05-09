#pragma once
#include <sched.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "constants.h"
#include "state.h"

const std::string UNKNOWN_INSTRUCTION = "UNKNOWN-INSTRUCTION";

struct DisAssembler {
    std::vector<bool>
        isSuccessfullyDisAssembled;  // bool array indicating which
                                     // bytes have been decoded
    const std::vector<unsigned char>
        &binaryBytes;  // byte array of the object source
    const std::unordered_map<long long, std::string> &addr2symbol;

    size_t curAddr;  // the current index to be decoded

    std::vector<std::pair<size_t, size_t>> disassembledPositions;
    std::unordered_map<std::pair<size_t, size_t>, std::string>
        disassembledInstructions;  // mapping of index to
                                   // disassembledInstructions
    std::unordered_map<size_t, size_t> disassembledInstructionsLength;
    std::vector<size_t> errorAddrs;  // keep track of error bytes indexes

    DisAssembler(const std::vector<unsigned char> &binaryBytes,
                 const std::unordered_map<long long, std::string> &addr2symbol)
        : binaryBytes(binaryBytes), addr2symbol(addr2symbol), curAddr(0) {
        isSuccessfullyDisAssembled =
            std::vector<bool>(binaryBytes.size(), false);
    }

    virtual void disas() = 0;

    void storeInstruction(DisassembledInstruction instruction) {
        size_t nextAddr = instruction.startAddr + instruction.instructionLen;

        // skip if this has already been decoded
        for (size_t idx = instruction.startAddr; idx < nextAddr; ++idx) {
            if (isSuccessfullyDisAssembled[idx]) {
                return;
            }
        }

        // mark the decoded regions
        for (size_t idx = instruction.startAddr; idx < nextAddr; ++idx) {
            isSuccessfullyDisAssembled[idx] = true;
        }

        // mark the regions causing errors
        if (!errorAddrs.empty()) {
            size_t startErr = errorAddrs[0];
            size_t instructionLengthErr = errorAddrs.size();
            disassembledInstructions[std::make_pair(
                startErr, startErr + instructionLengthErr)] =
                UNKNOWN_INSTRUCTION;
            disassembledPositions.push_back(
                std::make_pair(startErr, startErr + instructionLengthErr));
            errorAddrs.clear();
        }

        disassembledInstructions[std::make_pair(instruction.startAddr,
                                                nextAddr)] =
            instruction.assemblyInstructionStr;
        disassembledPositions.push_back(
            std::make_pair(instruction.startAddr, nextAddr));
        disassembledInstructionsLength[instruction.startAddr] =
            instruction.instructionLen;

        return;
    }

    void storeError(int startAddr, int instructionLength) {
        for (int i = startAddr; i < startAddr + instructionLength; i++) {
            isSuccessfullyDisAssembled[i] = false;
            errorAddrs.push_back(i);
        }
    }

    DisassembledInstruction step() {
        State state(binaryBytes, addr2symbol);
        DisassembledInstruction instruction = state.step(getCurAddr());
        storeInstruction(instruction);
        return instruction;
    }

    int getCurAddr() { return curAddr; }

    bool hasDecoded(int idx) { return isSuccessfullyDisAssembled[idx]; }

    bool hasRemainingBytes() {
        return std::count(isSuccessfullyDisAssembled.begin(),
                          isSuccessfullyDisAssembled.end(), false) != 0;
    }
};

struct LinearSweepDisAssembler : public DisAssembler {
    using DisAssembler::DisAssembler;

    void disas() {
        while (curAddr < binaryBytes.size()) {
            try {
                DisassembledInstruction instruction = step();
                curAddr = instruction.startAddr + instruction.instructionLen;
            } catch (const std::exception &e) {
                std::cerr << std::to_string(curAddr) << ": " << e.what()
                          << std::endl;
                storeError(curAddr, 1);
                curAddr += 1;
            }
        }
    }
};

struct RecursiveDescentDisAssembler : public DisAssembler {
    using DisAssembler::DisAssembler;

    void popAddr(std::stack<uint64_t> &stackedAddrs, bool &isDone) {
        while (true) {
            if (stackedAddrs.empty()) {
                isDone = true;
                break;
            } else {
                curAddr = stackedAddrs.top();
                stackedAddrs.pop();
                if (!isSuccessfullyDisAssembled[curAddr]) {
                    break;
                }
            }
        }
    }

    void disas() {
        bool isDone = false;
        std::stack<uint64_t> stackedAddrs;

        while (!isDone) {
            try {
                DisassembledInstruction instruction = step();
                Mnemonic mnemonic = instruction.mnemonic;

                size_t nextAddr =
                    instruction.startAddr + instruction.instructionLen;
                size_t cfAddr = (size_t)((long long)instruction.startAddr +
                                         (long long)instruction.instructionLen +
                                         instruction.nextOffset);

                if (mnemonic == Mnemonic::RET ||
                    nextAddr >= binaryBytes.size()) {
                    popAddr(stackedAddrs, isDone);
                } else if (isControlFlowInstruction(mnemonic)) {
                    if (nextAddr == cfAddr) {
                        if (nextAddr < binaryBytes.size()) {
                            curAddr = nextAddr;
                        } else if (!stackedAddrs.empty()) {
                            popAddr(stackedAddrs, isDone);
                        } else {
                            isDone = true;
                            break;
                        }
                    } else {
                        if (nextAddr < binaryBytes.size() &&
                            !isSuccessfullyDisAssembled[nextAddr]) {
                            stackedAddrs.push(nextAddr);
                        }
                        if (cfAddr < binaryBytes.size()) {
                            curAddr = cfAddr;
                        } else if (!stackedAddrs.empty()) {
                            popAddr(stackedAddrs, isDone);
                        } else {
                            isDone = true;
                            break;
                        }
                    }
                } else {
                    if (nextAddr < binaryBytes.size()) {
                        curAddr = nextAddr;
                    } else if (!stackedAddrs.empty()) {
                        popAddr(stackedAddrs, isDone);
                    } else {
                        isDone = true;
                        break;
                    }
                }

            } catch (const std::exception &e) {
                std::cerr << std::to_string(curAddr) << ": " << e.what()
                          << std::endl;
                storeError(curAddr, 1);
                if (curAddr + 1 < binaryBytes.size()) {
                    curAddr += 1;
                } else if (!stackedAddrs.empty()) {
                    popAddr(stackedAddrs, isDone);
                } else {
                    isDone = true;
                    break;
                }
            }
        }
    }
};
