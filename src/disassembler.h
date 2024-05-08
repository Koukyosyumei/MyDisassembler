#pragma once
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "state.h"

const std::string UNKNOWN_INSTRUCTION = "UNKNOWN-INSTRUCTION";

struct DisAssembler {
    std::vector<bool>
        isSuccessfullyDisAssembled;  // bool array indicating which
                                     // bytes have been decoded
    const std::vector<unsigned char>
        &binaryBytes;  // byte array of the object source

    size_t curIdx;  // the current index to be decoded

    std::vector<std::pair<size_t, size_t>> disassembledPositions;
    std::unordered_map<std::pair<size_t, size_t>, std::string>
        disassembledInstructions;  // mapping of index to
                                   // disassembledInstructions
    std::unordered_map<size_t, size_t> disassembledInstructionsLength;
    std::vector<size_t> errorIdxs;  // keep track of error bytes indexes

    DisAssembler(const std::vector<unsigned char> &binaryBytes)
        : binaryBytes(binaryBytes), curIdx(0) {
        isSuccessfullyDisAssembled =
            std::vector<bool>(binaryBytes.size(), false);
    }

    virtual void disas() = 0;

    uint64_t storeInstruction(size_t startIdx, size_t instructionLength,
                              std::string mnemonicStr, std::string instruction,
                              long long nextOffset) {
        size_t labelAddr = (size_t)((long long)startIdx +
                                    (long long)instructionLength + nextOffset);

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

    std::pair<std::string, uint64_t> step() {
        State state(binaryBytes);
        std::tuple<size_t, size_t, Mnemonic, std::string, size_t> instruction =
            state.step(getCurIdx());
        uint64_t targetAddr = storeInstruction(
            std::get<0>(instruction), std::get<1>(instruction),
            to_string(std::get<2>(instruction)), std::get<3>(instruction),
            std::get<4>(instruction));
        return std::make_pair(to_string(std::get<2>(instruction)), targetAddr);
    }

    int getCurIdx() { return curIdx; }

    bool hasDecoded(int idx) { return isSuccessfullyDisAssembled[idx]; }

    bool isComplete() {
        return std::count(isSuccessfullyDisAssembled.begin(),
                          isSuccessfullyDisAssembled.end(), false) == 0;
    }

    bool isSweepComplete() { return curIdx >= binaryBytes.size(); }
};

struct LinearSweepDisAssembler : public DisAssembler {
    using DisAssembler::DisAssembler;

    void disas() {
        size_t curIdx;

        while (!isSweepComplete()) {
            curIdx = getCurIdx();
            try {
                State state(binaryBytes);
                std::tuple<size_t, size_t, Mnemonic, std::string, size_t>
                    instruction = state.step(curIdx);
                storeInstruction(
                    std::get<0>(instruction), std::get<1>(instruction),
                    to_string(std::get<2>(instruction)),
                    std::get<3>(instruction), std::get<4>(instruction));
            } catch (const std::exception &e) {
                std::cerr << std::to_string(curIdx) << ": " << e.what()
                          << std::endl;
                storeError(curIdx, 1);
            }
        }
    }
};
