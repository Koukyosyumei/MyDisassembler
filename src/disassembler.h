#pragma once
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

    size_t curIdx;  // the current index to be decoded

    std::vector<std::pair<size_t, size_t>> disassembledPositions;
    std::unordered_map<std::pair<size_t, size_t>, std::string>
        disassembledInstructions;  // mapping of index to
                                   // disassembledInstructions
    std::unordered_map<size_t, size_t> disassembledInstructionsLength;
    std::vector<size_t> errorIdxs;  // keep track of error bytes indexes

    DisAssembler(const std::vector<unsigned char> &binaryBytes,
                 const std::unordered_map<long long, std::string> &addr2symbol)
        : binaryBytes(binaryBytes), addr2symbol(addr2symbol), curIdx(0) {
        isSuccessfullyDisAssembled =
            std::vector<bool>(binaryBytes.size(), false);
    }

    virtual void disas() = 0;
    virtual bool isComplete() = 0;

    uint64_t storeInstruction(size_t startIdx, size_t instructionLength,
                              std::string mnemonicStr, std::string instruction,
                              long long nextOffset) {
        uint64_t labelAddr =
            (uint64_t)((long long)startIdx + (long long)instructionLength +
                       nextOffset);

        // skip if this has already been decoded
        for (size_t idx = startIdx; idx < startIdx + instructionLength; ++idx) {
            if (isSuccessfullyDisAssembled[idx]) {
                return labelAddr;
            }
        }

        // mark the decoded regions
        curIdx = startIdx + instructionLength;
        for (size_t idx = startIdx; idx < startIdx + instructionLength; ++idx) {
            isSuccessfullyDisAssembled[idx] = true;
        }

        // mark the regions causing errors
        if (!errorIdxs.empty()) {
            size_t startErr = errorIdxs[0];
            size_t instructionLengthErr = errorIdxs.size();
            disassembledInstructions[std::make_pair(
                startErr, startErr + instructionLengthErr)] =
                UNKNOWN_INSTRUCTION;
            disassembledPositions.push_back(
                std::make_pair(startErr, startErr + instructionLengthErr));
            errorIdxs.clear();
        }

        disassembledInstructions[std::make_pair(
            startIdx, startIdx + instructionLength)] = instruction;
        disassembledPositions.push_back(
            std::make_pair(startIdx, startIdx + instructionLength));
        disassembledInstructionsLength[startIdx] = instructionLength;

        return labelAddr;
    }

    void storeError(int startIdx, int instructionLength) {
        curIdx = startIdx + instructionLength;
        for (int i = startIdx; i < startIdx + instructionLength; i++) {
            isSuccessfullyDisAssembled[i] = false;
            errorIdxs.push_back(i);
        }
    }

    std::pair<Mnemonic, uint64_t> step() {
        State state(binaryBytes, addr2symbol);
        std::tuple<size_t, size_t, Mnemonic, std::string, size_t> instruction =
            state.step(getCurIdx());
        uint64_t targetAddr = storeInstruction(
            std::get<0>(instruction), std::get<1>(instruction),
            to_string(std::get<2>(instruction)), std::get<3>(instruction),
            std::get<4>(instruction));
        return std::make_pair(std::get<2>(instruction), targetAddr);
    }

    int getCurIdx() { return curIdx; }

    bool hasDecoded(int idx) { return isSuccessfullyDisAssembled[idx]; }

    bool hasRemainingBytes() {
        return std::count(isSuccessfullyDisAssembled.begin(),
                          isSuccessfullyDisAssembled.end(), false) != 0;
    }
};

struct LinearSweepDisAssembler : public DisAssembler {
    using DisAssembler::DisAssembler;

    bool isComplete() { return curIdx >= binaryBytes.size(); }

    void disas() {
        size_t curIdx;

        while (!isComplete()) {
            try {
                step();
            } catch (const std::exception &e) {
                std::cerr << std::to_string(curIdx) << ": " << e.what()
                          << std::endl;
                storeError(curIdx, 1);
            }
        }
    }
};

struct RecursiveDescentDisAssembler : public DisAssembler {
    std::stack<uint64_t> unvisited_addrs;

    using DisAssembler::DisAssembler;

    bool isComplete() { return unvisited_addrs.empty(); }

    void disas() {
        size_t curIdx;

        while (!isComplete()) {
            try {
                std::pair<Mnemonic, uint64_t> result = step();
                Mnemonic mnemonic = result.first;
                uint64_t addr = result.second;

                if (mnemonic == Mnemonic::RET) {
                    while (true) {
                        if (unvisited_addrs.empty()) {
                            break;
                        } else {
                            curIdx = unvisited_addrs.top();
                            unvisited_addrs.pop();
                            if (!isSuccessfullyDisAssembled[curIdx]) {
                                break;
                            }
                        }
                    }
                } else if (mnemonic == Mnemonic::JMP) {
                    curIdx = addr;
                } else if (isJCCInstruction(mnemonic)) {
                    if (curIdx < binaryBytes.size()) {
                        unvisited_addrs.push(addr);
                    }
                } else if (mnemonic == Mnemonic::CALL) {
                    if (curIdx < binaryBytes.size() &&
                        !isSuccessfullyDisAssembled[curIdx]) {
                        unvisited_addrs.push(curIdx);
                    }
                    curIdx = addr;
                } else {
                    if (curIdx < binaryBytes.size() &&
                        isSuccessfullyDisAssembled[curIdx]) {
                        curIdx += disassembledInstructionsLength[curIdx];
                    }
                }

            } catch (const std::exception &e) {
                std::cerr << std::to_string(curIdx) << ": " << e.what()
                          << std::endl;
                storeError(curIdx, 1);
            }
        }
    }
};
