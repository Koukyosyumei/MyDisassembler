#pragma once
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "state.h"

const std::string UNKNOWN_INSTRUCTION = "UNKNOWN-INSTRUCTION";

struct DisAssembler {
    std::vector<bool>
        isDone;  // bool array indicating which bytes have been decoded
    const std::vector<unsigned char>
        &binaryBytes;    // byte array of the object source
    size_t _currentIdx;  // the current index to be decoded
    std::unordered_map<std::pair<size_t, size_t>, std::string>
        disassembledInstructions;  // mapping of index to
                                   // disassembledInstructions
    std::vector<std::pair<size_t, size_t>> instructionKeys;
    std::unordered_map<size_t, size_t> instructionLens;
    std::vector<size_t> runningErrorIdx;  // keep track of error bytes indexes
    std::vector<std::string> labelAddresses;

    DisAssembler(const std::vector<unsigned char> &binaryBytes)
        : binaryBytes(binaryBytes), _currentIdx(0) {
        isDone = std::vector<bool>(binaryBytes.size(), false);
    }

    virtual void disas() = 0;

    uint64_t storeInstruction(size_t startIdx, size_t instructionLength,
                              std::string mnemonicStr, std::string instruction,
                              long long nextOffset) {
        size_t labelAddr = (size_t)((long long)startIdx +
                                    (long long)instructionLength + nextOffset);

        // skip if this has already been decoded
        for (size_t idx = startIdx; idx < startIdx + instructionLength; ++idx) {
            if (isDone[idx]) {
                return labelAddr;
            }
        }

        // mark the decoded regions
        _currentIdx = startIdx + instructionLength;
        for (size_t idx = startIdx; idx < startIdx + instructionLength; ++idx) {
            isDone[idx] = true;
        }

        // mark the regions causing errors
        if (!runningErrorIdx.empty()) {
            size_t startErr = runningErrorIdx[0];
            size_t instructionLengthErr = runningErrorIdx.size();
            disassembledInstructions[std::make_pair(
                startErr, startErr + instructionLengthErr)] =
                UNKNOWN_INSTRUCTION;
            instructionKeys.push_back(
                std::make_pair(startErr, startErr + instructionLengthErr));
            runningErrorIdx.clear();
        }

        disassembledInstructions[std::make_pair(
            startIdx, startIdx + instructionLength)] = instruction;
        instructionKeys.push_back(
            std::make_pair(startIdx, startIdx + instructionLength));
        instructionLens[startIdx] = instructionLength;

        return labelAddr;
    }

    void storeError(int startIdx, int instructionLength) {
        _currentIdx = startIdx + instructionLength;
        for (int i = startIdx; i < startIdx + instructionLength; i++) {
            isDone[i] = false;
            runningErrorIdx.push_back(i);
        }
    }

    std::pair<std::string, uint64_t> decodeSingleInstruction() {
        State state(binaryBytes);
        std::tuple<size_t, size_t, Mnemonic, std::string, size_t> result =
            state.decodeSingleInstruction(getCurIdx());
        uint64_t targetAddr =
            storeInstruction(std::get<0>(result), std::get<1>(result),
                             to_string(std::get<2>(result)),
                             std::get<3>(result), std::get<4>(result));
        return std::make_pair(to_string(std::get<2>(result)), targetAddr);
    }

    int getCurIdx() { return _currentIdx; }

    bool hasDecoded(int idx) { return isDone[idx]; }

    bool isComplete() {
        int countFalse = std::count(isDone.begin(), isDone.end(), false);
        // int countNone = std::count(isDone.begin(), isDone.end(),
        // nullptr);
        return countFalse == 0;  // && countNone == 0;
    }

    bool isSweepComplete() { return _currentIdx >= binaryBytes.size(); }
};

struct LinearSweepDisAssembler : public DisAssembler {
    using DisAssembler::DisAssembler;

    void disas() {
        size_t instCount = 1;
        size_t curIdx;
        std::string message;

        while (!isSweepComplete()) {
            curIdx = getCurIdx();
            try {
                State state(binaryBytes);
                std::tuple<size_t, size_t, Mnemonic, std::string, size_t>
                    result = state.decodeSingleInstruction(curIdx);
                storeInstruction(std::get<0>(result), std::get<1>(result),
                                 to_string(std::get<2>(result)),
                                 std::get<3>(result), std::get<4>(result));
                instCount++;
            } catch (InvalidOperandError &) {
                std::cerr << "Unable to parse byte as an operand @ position " +
                                 std::to_string(curIdx)
                          << std::endl;
                storeError(curIdx, 1);
            } catch (OPCODE_LOOKUP_ERROR &) {
                std::string message;
                try {
                    message = "Unable to parse byte as an opcode @ position " +
                              std::to_string(curIdx) + " (byte:" +
                              std::to_string(binaryBytes.at(getCurIdx())) +
                              ").";
                } catch (...) {
                    message = "Unable to parse byte as an opcode @ position " +
                              std::to_string(curIdx) + " (byte:).";
                }
                std::cerr << message << std::endl;
                storeError(curIdx, 1);
            } catch (...) {
                std::string message;
                try {
                    message = "Unable to parse byte @ position " +
                              std::to_string(curIdx) + " (byte:" +
                              std::to_string(binaryBytes.at(getCurIdx())) +
                              ").";
                } catch (...) {
                    message = "Unable to parse byte @ position " +
                              std::to_string(curIdx) + " (byte).";
                }
                std::cerr << message << std::endl;
                break;
            }
        }
    }
};
