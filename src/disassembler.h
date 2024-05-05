#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "state.h"

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
                         std::string mnemonicStr, std::string instruction,
                         long long nextOffset) {
        size_t labelAddr =
            (size_t)((long long)startIdx + (long long)byteLen + nextOffset);

        // skip if this has already been decoded
        std::unordered_set<bool> decodedPath;
        for (size_t idx = startIdx; idx < startIdx + byteLen; ++idx) {
            decodedPath.insert(_hasDecoded[idx]);
        }
        if (decodedPath.count(true) > 0) {
            return labelAddr;
        }

        // mark the decoded regions
        _currentIdx = startIdx + byteLen;
        for (size_t idx = startIdx; idx < startIdx + byteLen; ++idx) {
            _hasDecoded[idx] = true;
        }

        // mark the regions causing errors
        if (!runningErrorIdx.empty()) {
            size_t startErr = runningErrorIdx[0];
            size_t byteLenErr = runningErrorIdx.size();
            instructions[std::make_pair(startErr, startErr + byteLenErr)] =
                UNKNOWN_INSTRUCTION;
            instructionKeys.push_back(
                std::make_pair(startErr, startErr + byteLenErr));
            runningErrorIdx.clear();
        }

        instructions[std::make_pair(startIdx, startIdx + byteLen)] =
            instruction;
        instructionKeys.push_back(std::make_pair(startIdx, startIdx + byteLen));
        instructionLens[startIdx] = byteLen;

        return labelAddr;
    }

    void markError(int startIdx, int byteLen) {
        _currentIdx = startIdx + byteLen;
        for (int i = startIdx; i < startIdx + byteLen; i++) {
            _hasDecoded[i] = false;
            runningErrorIdx.push_back(i);
        }
    }

    std::pair<std::string, uint64_t> decodeSingleInstruction() {
        State state(objectSource);
        std::tuple<size_t, size_t, Mnemonic, std::string, size_t> result =
            state.decodeSingleInstruction(getCurIdx());
        uint64_t targetAddr =
            markDecoded(std::get<0>(result), std::get<1>(result),
                        to_string(std::get<2>(result)), std::get<3>(result),
                        std::get<4>(result));
        return std::make_pair(to_string(std::get<2>(result)), targetAddr);
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
