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
    const std::unordered_map<uint64_t, std::string> &addr2symbol;

    uint64_t curAddr;  // the current index to be decoded

    std::unordered_set<std::pair<uint64_t, uint64_t>> disassembledPositions;
    std::unordered_map<std::pair<uint64_t, uint64_t>, std::string>
        disassembledInstructions;  // mapping of index to
                                   // disassembledInstructions
    std::unordered_map<uint64_t, uint64_t> disassembledInstructionsLength;
    std::vector<uint64_t> errorAddrs;  // keep track of error bytes indexes

    DisAssembler(const std::vector<unsigned char> &binaryBytes,
                 const std::unordered_map<uint64_t, std::string> &addr2symbol)
        : binaryBytes(binaryBytes), addr2symbol(addr2symbol), curAddr(0) {
        isSuccessfullyDisAssembled =
            std::vector<bool>(binaryBytes.size(), false);
    }

    virtual void disas(uint64_t startAddr, uint64_t endAddr = -1) = 0;

    void storeInstruction(DisassembledInstruction instruction) {
        uint64_t nextAddr = instruction.startAddr + instruction.instructionLen;

        // skip if this has already been decoded
        for (uint64_t idx = instruction.startAddr; idx < nextAddr; ++idx) {
            if (isSuccessfullyDisAssembled[idx]) {
                return;
            }
        }

        // mark the decoded regions
        for (uint64_t idx = instruction.startAddr; idx < nextAddr; ++idx) {
            isSuccessfullyDisAssembled[idx] = true;
        }

        // mark the regions causing errors
        if (!errorAddrs.empty()) {
            uint64_t startErr = errorAddrs[0];
            uint64_t instructionLengthErr = errorAddrs.size();
            disassembledInstructions[std::make_pair(
                startErr, startErr + instructionLengthErr)] =
                UNKNOWN_INSTRUCTION;
            disassembledPositions.emplace(
                std::make_pair(startErr, startErr + instructionLengthErr));
            errorAddrs.clear();
        }

        disassembledInstructions[std::make_pair(instruction.startAddr,
                                                nextAddr)] =
            instruction.assemblyInstructionStr;
        disassembledPositions.emplace(
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

    void disas(uint64_t startAddr, uint64_t endAddr = -1) {
        curAddr = startAddr;
        endAddr = (endAddr < 0) ? binaryBytes.size() - 1 : endAddr;

        while (curAddr <= endAddr) {
            try {
                DisassembledInstruction instruction = step();
                curAddr = instruction.startAddr + instruction.instructionLen;
            } catch (const std::exception &e) {
                std::stringstream ss;
                ss << std::hex << curAddr;
                std::cerr << ss.str() << ": " << e.what() << std::endl;
                curAddr += 1;
            }
        }
    }
};

struct RecursiveDescentDisAssembler : public DisAssembler {
    using DisAssembler::DisAssembler;

    void popAddr(std::stack<uint64_t> &stackedAddrs, std::vector<bool> &visited,
                 bool &isDone) {
        while (true) {
            if (stackedAddrs.empty()) {
                isDone = true;
                break;
            } else {
                curAddr = stackedAddrs.top();
                stackedAddrs.pop();
                if (!isSuccessfullyDisAssembled[curAddr] && !visited[curAddr]) {
                    break;
                }
            }
        }
    }

    void disas(uint64_t startAddr, uint64_t endAddr = -1) {
        bool isDone = false;
        std::stack<uint64_t> stackedAddrs;
        std::vector<bool> visited(binaryBytes.size(), false);

        curAddr = startAddr;
        endAddr = (endAddr < 0) ? binaryBytes.size() - 1 : endAddr;

        while (!isDone) {
            try {
                DisassembledInstruction instruction = step();
                visited[curAddr] = true;
                Mnemonic mnemonic = instruction.mnemonic;

                uint64_t nextAddr =
                    instruction.startAddr + instruction.instructionLen;
                uint64_t cfAddr =
                    (uint64_t)((long long)instruction.startAddr +
                               (long long)instruction.instructionLen +
                               instruction.nextOffset);

                if (mnemonic == Mnemonic::RET || nextAddr > endAddr) {
                    // return to the callee
                    popAddr(stackedAddrs, visited, isDone);
                } else if (isControlFlowInstruction(mnemonic)) {
                    if (nextAddr == cfAddr) {
                        if (nextAddr <= endAddr && !visited[nextAddr]) {
                            curAddr = nextAddr;
                        } else {
                            popAddr(stackedAddrs, visited, isDone);
                        }
                    } else {
                        if (nextAddr <= endAddr &&
                            !isSuccessfullyDisAssembled[nextAddr] &&
                            !visited[nextAddr]) {
                            stackedAddrs.push(nextAddr);
                        }
                        if (cfAddr <= endAddr && !visited[cfAddr]) {
                            curAddr = cfAddr;
                        } else {
                            popAddr(stackedAddrs, visited, isDone);
                        }
                    }
                } else {
                    if (nextAddr <= endAddr && !visited[nextAddr]) {
                        curAddr = nextAddr;
                    } else {
                        popAddr(stackedAddrs, visited, isDone);
                    }
                }

            } catch (const std::exception &e) {
                visited[curAddr] = true;

                std::stringstream ss;
                ss << std::hex << curAddr;
                std::cerr << ss.str() << ": " << e.what() << std::endl;
                storeError(curAddr, 1);

                if (!visited[curAddr + 1] && curAddr + 1 <= endAddr) {
                    curAddr += 1;
                } else {
                    popAddr(stackedAddrs, visited, isDone);
                }
            }
        }
    }
};
