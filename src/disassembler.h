/**
 * @file
 * @brief Defines structures and functions for disassembling x86 instructions.
 */

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

#include "state.h"

/**
 * @brief The string representation for unknown instructions.
 */
const std::string UNKNOWN_INSTRUCTION = "UNKNOWN-INSTRUCTION";

/**
 * @struct DisAssembler
 * @brief Represents a disassembler for x86 instructions.
 */
struct DisAssembler {
    std::vector<bool> isSuccessfullyDisAssembled; /**< Array indicating which
                                                     bytes have been decoded */
    const std::vector<unsigned char>
        &binaryBytes; /**< Byte array of the object source */
    const std::unordered_map<uint64_t, std::string>
        &addr2symbol; /**< Mapping of addresses to symbols */

    uint64_t curAddr; /**< The current index to be decoded */

    std::unordered_set<std::pair<uint64_t, uint64_t>>
        disassembledPositions; /**< Set of disassembled positions */
    std::unordered_map<std::pair<uint64_t, uint64_t>, std::string>
        disassembledInstructions; /**< Mapping of index to disassembled
                                     instructions */
    std::unordered_map<uint64_t, uint64_t>
        disassembledInstructionsLength; /**< Mapping of address to instruction
                                           length */
    std::vector<uint64_t> errorAddrs; /**< Keeps track of error bytes indexes */
    size_t maxInstructionStrLength =
        0; /**< The maximum length of the instruction string */

    /**
     * @brief Constructor for DisAssembler.
     * @param binaryBytes The byte array of the object source.
     * @param addr2symbol Mapping of addresses to symbols.
     */
    DisAssembler(const std::vector<unsigned char> &binaryBytes,
                 const std::unordered_map<uint64_t, std::string> &addr2symbol)
        : binaryBytes(binaryBytes), addr2symbol(addr2symbol), curAddr(0) {
        isSuccessfullyDisAssembled =
            std::vector<bool>(binaryBytes.size(), false);
    }

    /**
     * @brief Disassembles instructions within the specified range.
     * @param startAddr The starting address.
     * @param endAddr The ending address.
     */
    virtual void disas(uint64_t startAddr, uint64_t endAddr = -1) = 0;

    /**
     * @brief Stores the disassembled instruction.
     * @param instruction The disassembled instruction.
     */
    void storeInstruction(DisassembledResult instruction) {
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
            instruction.disassembledInstructionStr;
        maxInstructionStrLength =
            std::max(maxInstructionStrLength,
                     instruction.disassembledInstructionStr.size());
        disassembledPositions.emplace(
            std::make_pair(instruction.startAddr, nextAddr));
        disassembledInstructionsLength[instruction.startAddr] =
            instruction.instructionLen;

        return;
    }

    /**
     * @brief Stores an error in decoding.
     * @param startAddr The starting address of the error.
     * @param instructionLength The length of the error.
     */
    void storeError(int startAddr, int instructionLength) {
        for (int i = startAddr; i < startAddr + instructionLength; i++) {
            isSuccessfullyDisAssembled[i] = false;
            errorAddrs.emplace_back(i);
        }
    }

    /**
     * @brief Executes a step in disassembling the instruction.
     * @return The disassembled result.
     */
    DisassembledResult step() {
        State state(binaryBytes, addr2symbol);
        DisassembledResult instruction = state.step(getCurAddr());
        storeInstruction(instruction);
        return instruction;
    }

    /**
     * @brief Gets the current address being decoded.
     * @return The current address.
     */
    int getCurAddr() { return curAddr; }
};

/**
 * @struct LinearSweepDisAssembler
 * @brief Represents a disassembler using linear sweep algorithm.
 */
struct LinearSweepDisAssembler : public DisAssembler {
    using DisAssembler::DisAssembler;

    /**
     * @brief Disassembles instructions using linear sweep algorithm.
     * @param startAddr The starting address.
     * @param endAddr The ending address.
     */
    void disas(uint64_t startAddr, uint64_t endAddr = -1) {
        curAddr = startAddr;
        endAddr = (endAddr < 0) ? binaryBytes.size() - 1 : endAddr;

        while (curAddr <= endAddr) {
            try {
                DisassembledResult instruction = step();
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

/**
 * @struct RecursiveDescentDisAssembler
 * @brief Represents a disassembler using recursive descent algorithm.
 */
struct RecursiveDescentDisAssembler : public DisAssembler {
    using DisAssembler::DisAssembler;

    /**
     * @brief Pops an address from the stack until a valid address is found.
     * @param stackedAddrs The stack of addresses.
     * @param visited The visited bytes.
     * @param isDone Flag indicating if the disassembly is done.
     */
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

    /**
     * @brief Disassembles instructions using recursive descent algorithm.
     * @param startAddr The starting address.
     * @param endAddr The ending address.
     */
    void disas(uint64_t startAddr, uint64_t endAddr = -1) {
        bool isDone = false;
        std::stack<uint64_t> stackedAddrs;
        std::vector<bool> visited(binaryBytes.size(), false);

        curAddr = startAddr;
        endAddr = (endAddr < 0) ? binaryBytes.size() - 1 : endAddr;

        while (!isDone) {
            try {
                DisassembledResult instruction = step();
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
