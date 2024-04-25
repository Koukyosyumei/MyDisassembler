#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "constants.h"
#include "utils.h"

// Global lookup table for instructions
// (prefix, opcode) -> (register -> operator)
const std::unordered_map<std::pair<Prefix, int>,
                         std::unordered_map<int, Mnemonic>>
    OP_LOOKUP = {
        {{Prefix::NONE, u2d("\x05")}, {{-1, Mnemonic::ADD}}},
        {{Prefix::NONE, u2d("\x81")}, {{u2d("\x00"), Mnemonic::ADD}}},
        {{Prefix::NONE, u2d("\x83")}, {{u2d("\x00"), Mnemonic::ADD}}},
        {{Prefix::NONE, u2d("\x01")}, {{-1, Mnemonic::ADD}}},
        {{Prefix::NONE, u2d("\x03")}, {{-1, Mnemonic::ADD}}}
    };

// Lookup table for operand information
// (prefix, operator, opcode) -> (encoding, return value, operands)
const std::unordered_map<std::tuple<Prefix, Mnemonic, int>,
                         std::tuple<OpEnc, std::string, std::vector<Operand>>>
    OPERAND_LOOKUP = {
        // ADD
        {{Prefix::NONE, Mnemonic::ADD, u2d("\x05")}, {OpEnc::I, "id", {Operand::eax, Operand::imm32}}},
        {{Prefix::REXW, Mnemonic::ADD, u2d("\x05")}, {OpEnc::I, "id", {Operand::rax, Operand::imm32}}},
        {{Prefix::NONE, Mnemonic::ADD, u2d("\x81")}, {OpEnc::MI, "id", {Operand::rm, Operand::imm32}}},
        {{Prefix::NONE, Mnemonic::ADD, u2d("\x83")}, {OpEnc::MI, "ib", {Operand::rm, Operand::imm8}}},
        {{Prefix::NONE, Mnemonic::ADD, u2d("\x01")}, {OpEnc::MR, "/r", {Operand::rm, Operand::reg}}},
        {{Prefix::NONE, Mnemonic::ADD, u2d("\x03")}, {OpEnc::RM, ".r", {Operand::reg, Operand::rm}}},
        // AND
        {{Prefix::NONE, Mnemonic::AND, u2d("\x25")}, {OpEnc::I, "id", {Operand::eax, Operand::imm32}}},
        {{Prefix::NONE, Mnemonic::AND, u2d("\x81")}, {OpEnc::MI, "id", {Operand::rm, Operand::imm32}}},
        {{Prefix::NONE, Mnemonic::AND, u2d("\x83")}, {OpEnc::MI, "ib", {Operand::rm, Operand::imm8}}},
        {{Prefix::NONE, Mnemonic::AND, u2d("\x21")}, {OpEnc::MR, "/r", {Operand::rm, Operand::reg}}},
        {{Prefix::NONE, Mnemonic::AND, u2d("\x23")}, {OpEnc::RM, ".r", {Operand::reg, Operand::rm}}},
        // CMP
        {{Prefix::NONE, Mnemonic::CMP, u2d("\x3D")}, {OpEnc::I, "id", {Operand::eax, Operand::imm32}}},
        {{Prefix::NONE, Mnemonic::CMP, u2d("\x81")}, {OpEnc::MI, "id", {Operand::rm, Operand::imm32}}},
        {{Prefix::NONE, Mnemonic::CMP, u2d("\x83")}, {OpEnc::MI, "ib", {Operand::rm, Operand::imm8}}},
        {{Prefix::NONE, Mnemonic::CMP, u2d("\x39")}, {OpEnc::MR, "/r", {Operand::rm, Operand::reg}}},
        {{Prefix::NONE, Mnemonic::CMP, u2d("\x3B")}, {OpEnc::RM, ".r", {Operand::reg, Operand::rm}}},
        // OR
        {{Prefix::NONE, Mnemonic::OR, u2d("\x0D")}, {OpEnc::I, "id", {Operand::eax, Operand::imm32}}},
        {{Prefix::NONE, Mnemonic::OR, u2d("\x81")}, {OpEnc::MI, "id", {Operand::rm, Operand::imm32}}},
        {{Prefix::NONE, Mnemonic::OR, u2d("\x83")}, {OpEnc::MI, "ib", {Operand::rm, Operand::imm8}}},
        {{Prefix::NONE, Mnemonic::OR, u2d("\x09")}, {OpEnc::MR, "/r", {Operand::rm, Operand::reg}}},
        {{Prefix::NONE, Mnemonic::OR, u2d("\x0B")}, {OpEnc::RM, ".r", {Operand::reg, Operand::rm}}},
    };  

// Supported operators set
const std::unordered_set<std::string> SUPPORTED_OPERATORS;
