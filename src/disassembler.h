#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "state.h"
#include "table.h"

const std::string UNKNOWN_INSTRUCTION = "???";

struct DisAssembler {
    std::vector<bool>
        _hasDecoded;  // bool array indicating which bytes have been decoded
    const std::vector<unsigned char>
        &objectSource;   // byte array of the object source
    size_t _currentIdx;  // the current index to be decoded
    std::unordered_map<std::pair<size_t, size_t>, std::string>
        instructions;  // mapping of index to instructions
    std::vector<std::pair<size_t, size_t>> instructionKeys;
    std::unordered_map<size_t, size_t> instructionLens;
    std::vector<size_t> runningErrorIdx;  // keep track of error bytes indexes
    std::vector<std::string> labelAddresses;
    std::vector<size_t> deferAddresses;
    bool _completedRecursiveDescent;

    DisAssembler(const std::vector<unsigned char> &objectSource)
        : objectSource(objectSource),
          _currentIdx(0),
          _completedRecursiveDescent(false) {
        _hasDecoded = std::vector<bool>(objectSource.size(), false);
    }

    uint64_t markDecoded(size_t startIdx, size_t byteLen,
                         std::string &instruction) {
        size_t labelAddr = std::string::npos;

        // skip if this has already been decoded
        std::unordered_set<bool> decodedPath;
        for (size_t idx = startIdx; idx < startIdx + byteLen; ++idx) {
            decodedPath.insert(_hasDecoded[idx]);
        }
        if (decodedPath.count(true) > 0) {
            return labelAddr;
        }

        _currentIdx = startIdx + byteLen;
        for (size_t idx = startIdx; idx < startIdx + byteLen; ++idx) {
            _hasDecoded[idx] = true;
        }

        if (!runningErrorIdx.empty()) {
            size_t startErr = runningErrorIdx[0];
            size_t byteLenErr = runningErrorIdx.size();
            instructions[std::make_pair(startErr, startErr + byteLenErr)] =
                UNKNOWN_INSTRUCTION;
            instructionKeys.push_back(
                std::make_pair(startErr, startErr + byteLenErr));
            runningErrorIdx.clear();
        }

        std::string operatorStr = instruction.substr(0, instruction.find(" "));
        transform(operatorStr.begin(), operatorStr.end(), operatorStr.begin(),
                  ::tolower);
        if (operatorStr == "jmp" || operatorStr == "jz" ||
            operatorStr == "jnz" || operatorStr == "call") {
            size_t spacePos = instruction.find(" ");
            if (spacePos != std::string::npos) {
                std::string operand = instruction.substr(spacePos + 1);
                size_t offset;
                bool validOffset = false;
                try {
                    offset = stoul(operand, nullptr, 16);
                    validOffset = true;
                } catch (std::invalid_argument &) {
                    // Some calls do not have a direct offset
                }

                if (validOffset) {
                    if (operand.size() == 8) {
                        if (offset > 0x7FFFFFFF) {
                            offset -= 0x100000000;
                        }
                    } else if (operand.size() == 4) {
                        if (offset > 0x7FFF) {
                            offset -= 0x10000;
                        }
                    } else if (operand.size() == 2) {
                        if (offset > 0x7F) {
                            offset -= 0x100;
                        }
                    }

                    labelAddr = startIdx + byteLen + offset;
                    labelAddresses.push_back(std::to_string(labelAddr));
                    std::string label = "label_" + std::to_string(labelAddr);
                    // instruction = instructionOp + " " + label + " ; " +
                    //              operand + " = " + std::to_string(offset) +
                    //              " signed = addr[" +
                    //              std::to_string(labelAddr) +
                    //              "]";
                }
            }
        }

        instructions[std::make_pair(startIdx, startIdx + byteLen)] =
            instruction;
        instructionKeys.push_back(std::make_pair(startIdx, startIdx + byteLen));
        instructionLens[startIdx] = byteLen;

        return labelAddr;
    }

    std::pair<std::string, uint64_t> decodeSingleInstruction() {
        State state(objectSource);
        std::tuple<int, int, std::string, std::string> result =
            state.decodeSingleInstruction(getCurIdx());
        uint64_t targetAddr = markDecoded(
            std::get<0>(result), std::get<1>(result), std::get<3>(result));
        return std::make_pair(std::get<2>(result), targetAddr);
    }

    int getCurIdx() { return _currentIdx; }

    bool hasDecoded(int idx) { return _hasDecoded[idx]; }

    bool isComplete() {
        int countFalse =
            std::count(_hasDecoded.begin(), _hasDecoded.end(), false);
        // int countNone = std::count(_hasDecoded.begin(), _hasDecoded.end(),
        // nullptr);
        return countFalse == 0;  // && countNone == 0;
    }

    bool isSweepComplete() {
        return _currentIdx >= static_cast<int>(objectSource.size());
    }
};
