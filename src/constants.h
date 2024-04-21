#pragma once
#include <string>
#include <unordered_map>
#include <vector>

// Enums for instruction categories
enum class InstructionCategory { CALL, FUNC_END, JUMP, JCC };

const std::unordered_map<std::string, InstructionCategory>
    instructionCategories = {
        {"CALL", InstructionCategory::CALL},
        {"RET", InstructionCategory::FUNC_END},
        {"RETN", InstructionCategory::FUNC_END},
        {"RETF", InstructionCategory::FUNC_END},
        {"JMP", InstructionCategory::JUMP},
        {"JZ", InstructionCategory::JCC},
        {"JNZ", InstructionCategory::JCC},
};

// Define register names
const std::vector<std::string> REGISTERS = {"eax", "ecx", "edx", "ebx",
                                            "esp", "ebp", "esi", "edi"};

// Define addressing modes (no need for square brackets)
const std::vector<std::string> addressingModes = {"reg", "reg + disp8",
                                                  "reg + disp32"};

// Define scale factors
const std::vector<std::string> SCALE = {
    "index + base", "index * 2 + base", "index * 4 + base", "index * 8 + base"};

// Structure for operand unit types
struct OperandUnit {
    std::string
        type;  // "one", "imm8", "imm32", "imm16", "reg", "rm", "eax", "moff"
};

// Structure for operand encoding information
struct OperandEncoding {
    std::string opEnc;
    bool hasModrm;  // Indicates presence of ModRM byte
};

const std::unordered_map<std::string, OperandUnit> operandUnits = {
    {"one", {"one"}},     {"imm8", {"imm8"}}, {"imm32", {"imm32"}},
    {"imm16", {"imm16"}}, {"reg", {"reg"}},   {"rm", {"rm"}},
    {"eax", {"eax"}},
    // Add definition for "moff" if needed
};

const std::unordered_map<std::string, OperandEncoding> operandEncodings = {
    {"I", {"I", false}},    {"D", {"D", false}},   {"M", {"M", true}},
    {"O", {"O", false}},    {"NP", {"NP", false}}, {"MI", {"MI", true}},
    {"M1", {"M1", true}},   {"MR", {"MR", true}},  {"RM", {"RM", true}},
    {"RMI", {"RMI", true}}, {"OI", {"OI", false}},
};

