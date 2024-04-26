#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class InstructionCategory { CALL, FUNC_END, JUMP, JCC };

enum class Operand { one, imm8, imm16, imm32, reg, rm, al, ax, eax, rax, moff };

enum class OpEnc { I, D, M, O, NP, MI, M1, MR, RM, RMI, OI };

enum class Prefix {
    NONE,  // without prefix
    P66,   // change the default operand size
    REXW,  // use R8-R15 registers
    REX    // use 64 bit registers
};

enum class Mnemonic {
    MOV,
    LEA,
    ADD,
    ADC,
    SUB,
    SBB,
    MUL,
    IMUL,
    DIV,
    IDIV,
    INC,
    DEC,
    AND,
    OR,
    XOR,
    NOT,
    NEG,
    CMP,
    TEST,
    SAL,
    SHL,
    SAR,
    SHR,
    RCL,
    RCR,
    ROL,
    ROR,
    JMP,
    LOOP,
    JZ,
    JNZ,
    JA,
    JAE,
    JB,
    JBE,
    JG,
    JGE,
    JL,
    JLE,
    JP,
    JNP,
    JO,
    JNO,
    JS,
    JC,
    JCXZ,
    JECXZ,
    CALL,
    RET,
    PUSH,
    POP,
    MOVSB,
    MOVSW,
    MOVSD,
    REP,
    REPE,
    REPNE,
    CLD,
    STD,
    LODSB,
    LODSW,
    LODSD,
    STOSB,
    STOSW,
    STOSD,
    SCASB,
    SCASW,
    SCASD,
    CMPSB,
    CMPSW,
    CMPSD,
    IN,
    OUT,
    INSB,
    INSW,
    INSD,
    OUTSB,
    OUTSW,
    OUTSD,
    CBW,
    CWD,
    CWDE,
    CDQ,
    INT21,
    LOCK,
    ENTER,
    LEAVE,
    NOP,
    UD2,
    CPUID,
    XCHG,
    STC,
    CLC
};

enum class Registers {
    RAX,
    RCX,
    RDX,
    RBX,
    RSP,
    RBP,
    RSI,
    RDI,
    RIP,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    SIB
};

const std::unordered_map<int, std::string> id2register = {
    {0, "RAX"},  {1, "RCX"},  {2, "RDX"},  {3, "RBX"}, {4, "RSP"},  {5, "RBP"},
    {6, "RSI"},  {7, "RDI"},  {8, "R8"},   {9, "R9"},  {10, "R10"}, {11, "R11"},
    {12, "R12"}, {13, "R13"}, {14, "R14"}, {15, "R15"}};

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
const std::vector<std::string> SCALE = {"index + base", "index * 2 + base",
                                        "index * 4 + base", "index * 8 + base"};

inline std::string to_string(Operand opu) {
    switch (opu) {
        case Operand::one:
            return "one";
        case Operand::imm8:
            return "imm8";
        case Operand::imm16:
            return "imm16";
        case Operand::imm32:
            return "imm32";
        case Operand::reg:
            return "reg";
        case Operand::rm:
            return "rm";
        case Operand::eax:
            return "eax";
        case Operand::moff:
            return "moff";
        default:
            return "unknown";
    }
}

inline bool hasModrm(OpEnc openc) {
    switch (openc) {
        case OpEnc::I:
            return false;
        case OpEnc::D:
            return false;
        case OpEnc::M:
            return true;
        case OpEnc::O:
            return false;
        case OpEnc::NP:
            return false;
        case OpEnc::MI:
            return true;
        case OpEnc::M1:
            return true;
        case OpEnc::MR:
            return true;
        case OpEnc::RM:
            return true;
        case OpEnc::RMI:
            return true;
        case OpEnc::OI:
            return false;
    }
}

inline std::string to_string(OpEnc openc) {
    switch (openc) {
        case OpEnc::I:
            return "I";
        case OpEnc::D:
            return "D";
        case OpEnc::M:
            return "M";
        case OpEnc::O:
            return "O";
        case OpEnc::NP:
            return "NP";
        case OpEnc::MI:
            return "MI";
        case OpEnc::M1:
            return "M1";
        case OpEnc::MR:
            return "MR";
        case OpEnc::RM:
            return "RM";
        case OpEnc::RMI:
            return "RMI";
        case OpEnc::OI:
            return "OI";
    }
}

// Predefined prefixes and their associated instructions
const std::unordered_map<int, std::vector<std::string>>
    BYTE2PREFIX_INSTRUCTIONS = {
        {0x0F, {"IMUL", "JZ", "JNZ"}},
        {0xF0, {"LOCK"}},
        {0xF2, {"REPNE", "REPNZ"}},
        {0xF3, {"REP", "REPE", "REPZ"}},
};

const std::unordered_set<int> PREFIX_INSTRUCTIONS_BYTES_SET = {0x0F, 0xF0, 0xF2,
                                                               0xF3};
