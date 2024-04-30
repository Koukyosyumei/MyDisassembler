#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum class InstructionCategory { CALL, FUNC_END, JUMP, JCC };

enum class Operand {
    one,
    imm8,
    imm16,
    imm32,
    imm64,
    reg8,
    reg16,
    reg32,
    reg64,
    rm8,
    rm16,
    rm32,
    rm64,
    reg,
    sreg,
    m,
    al,
    ax,
    eax,
    rax,
    moffs8,
    moffs16,
    moffs32,
    moffs64
};

inline bool isA_REG(Operand operand) {
    return operand == Operand::al || operand == Operand::ax ||
           operand == Operand::eax || operand == Operand::rax;
}

inline bool isRM(Operand operand) {
    return operand == Operand::rm8 || operand == Operand::rm16 ||
           operand == Operand::rm32 || operand == Operand::rm64;
}

inline bool isREG(Operand operand) {
    return operand == Operand::reg8 || operand == Operand::reg16 ||
           operand == Operand::reg32 || operand == Operand::reg64;
}

inline bool is8Bit(Operand operand) {
    return operand == Operand::rm8 || operand == Operand::reg8;
}

inline bool is16Bit(Operand operand) {
    return operand == Operand::rm16 || operand == Operand::reg16;
}

inline bool is32Bit(Operand operand) {
    return operand == Operand::rm32 || operand == Operand::reg32;
}

inline bool is64Bit(Operand operand) {
    return operand == Operand::rm64 || operand == Operand::reg64;
}




enum class OpEnc { I, D, M, O, NP, MI, M1, MR, RM, RMI, OI };

enum class Prefix {
    NONE,  // without prefix
    P66,   // change the default operand size
    REXW,  // use R8-R15 registers
    REX    // use 64 bit registers
};

enum class Mnemonic {
    MOV,
    MOVSX,
    MOVSXD,
    MOVZX,
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
    CDQE,
    CQO,
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

inline std::string to_string(Mnemonic mnemonic) {
    switch (mnemonic) {
        case Mnemonic::MOV:
            return "MOV";
        case Mnemonic::LEA:
            return "LEA";
        case Mnemonic::ADD:
            return "ADD";
        case Mnemonic::ADC:
            return "ADC";
        case Mnemonic::SUB:
            return "SUB";
        case Mnemonic::SBB:
            return "SBB";
        case Mnemonic::MUL:
            return "MUL";
        case Mnemonic::IMUL:
            return "IMUL";
        case Mnemonic::DIV:
            return "DIV";
        case Mnemonic::IDIV:
            return "IDIV";
        case Mnemonic::INC:
            return "INC";
        case Mnemonic::DEC:
            return "DEC";
        case Mnemonic::AND:
            return "AND";
        case Mnemonic::OR:
            return "OR";
        case Mnemonic::XOR:
            return "XOR";
        case Mnemonic::NOT:
            return "NOT";
        case Mnemonic::NEG:
            return "NEG";
        case Mnemonic::CMP:
            return "CMP";
        case Mnemonic::TEST:
            return "TEST";
        case Mnemonic::SAL:
            return "SAL";
        case Mnemonic::SHL:
            return "SHL";
        case Mnemonic::SAR:
            return "SAR";
        case Mnemonic::SHR:
            return "SHR";
        case Mnemonic::RCL:
            return "RCL";
        case Mnemonic::RCR:
            return "RCR";
        case Mnemonic::ROL:
            return "ROL";
        case Mnemonic::ROR:
            return "ROR";
        case Mnemonic::JMP:
            return "JMP";
        case Mnemonic::LOOP:
            return "LOOP";
        case Mnemonic::JZ:
            return "JZ";
        case Mnemonic::JNZ:
            return "JNZ";
        case Mnemonic::JA:
            return "JA";
        case Mnemonic::JAE:
            return "JAE";
        case Mnemonic::JB:
            return "JB";
        case Mnemonic::JBE:
            return "JBE";
        case Mnemonic::JG:
            return "JG";
        case Mnemonic::JGE:
            return "JGE";
        case Mnemonic::JL:
            return "JL";
        case Mnemonic::JLE:
            return "JLE";
        case Mnemonic::JP:
            return "JP";
        case Mnemonic::JNP:
            return "JNP";
        case Mnemonic::JO:
            return "JO";
        case Mnemonic::JNO:
            return "JNO";
        case Mnemonic::JS:
            return "JS";
        case Mnemonic::JC:
            return "JC";
        case Mnemonic::JCXZ:
            return "JCXZ";
        case Mnemonic::JECXZ:
            return "JECXZ";
        case Mnemonic::CALL:
            return "CALL";
        case Mnemonic::RET:
            return "RET";
        case Mnemonic::PUSH:
            return "PUSH";
        case Mnemonic::POP:
            return "POP";
        case Mnemonic::MOVSB:
            return "MOVSB";
        case Mnemonic::MOVSW:
            return "MOVSW";
        case Mnemonic::MOVSD:
            return "MOVSD";
        case Mnemonic::REP:
            return "REP";
        case Mnemonic::REPE:
            return "REPE";
        case Mnemonic::REPNE:
            return "REPNE";
        case Mnemonic::CLD:
            return "CLD";
        case Mnemonic::STD:
            return "STD";
        case Mnemonic::LODSB:
            return "LODSB";
        case Mnemonic::LODSW:
            return "LODSW";
        case Mnemonic::LODSD:
            return "LODSD";
        case Mnemonic::STOSB:
            return "STOSB";
        case Mnemonic::STOSW:
            return "STOSW";
        case Mnemonic::STOSD:
            return "STOSD";
        case Mnemonic::SCASB:
            return "SCASB";
        case Mnemonic::SCASW:
            return "SCASW";
        case Mnemonic::SCASD:
            return "SCASD";
        case Mnemonic::CMPSB:
            return "CMPSB";
        case Mnemonic::CMPSW:
            return "CMPSW";
        case Mnemonic::CMPSD:
            return "CMPSD";
        case Mnemonic::IN:
            return "IN";
        case Mnemonic::OUT:
            return "OUT";
        case Mnemonic::INSB:
            return "INSB";
        case Mnemonic::INSW:
            return "INSW";
        case Mnemonic::INSD:
            return "INSD";
        case Mnemonic::OUTSB:
            return "OUTSB";
        case Mnemonic::OUTSW:
            return "OUTSW";
        case Mnemonic::OUTSD:
            return "OUTSD";
        case Mnemonic::CBW:
            return "CBW";
        case Mnemonic::CWD:
            return "CWD";
        case Mnemonic::CWDE:
            return "CWDE";
        case Mnemonic::CDQE:
            return "CDQE";
        case Mnemonic::INT21:
            return "INT21";
        case Mnemonic::LOCK:
            return "LOCK";
        case Mnemonic::ENTER:
            return "ENTER";
        case Mnemonic::LEAVE:
            return "LEAVE";
        case Mnemonic::NOP:
            return "NOP";
        case Mnemonic::UD2:
            return "UD2";
        case Mnemonic::CPUID:
            return "CPUID";
        case Mnemonic::XCHG:
            return "XCHG";
        case Mnemonic::STC:
            return "STC";
        case Mnemonic::CLC:
            return "CLC";
        default:
            return "UNKNOWN";
    }
}

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

/*
const std::unordered_map<int, std::string> id2register = {
    {0, "RAX"},  {1, "RCX"},  {2, "RDX"},  {3, "RBX"}, {4, "RSP"},  {5, "RBP"},
    {6, "RSI"},  {7, "RDI"},  {8, "R8"},   {9, "R9"},  {10, "R10"}, {11, "R11"},
    {12, "R12"}, {13, "R13"}, {14, "R14"}, {15, "R15"}};
*/

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
const std::vector<std::string> REGISTERS8 = {
    "al",  "cl",  "dl",   "bpl",  "spl",  "bpl",  "sil",  "dil",
    "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"};
const std::vector<std::string> REGISTERS16 = {
    "ax",  "cx",  "dx",   "bx",   "sp",   "bp",   "si",   "di",
    "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w"};
const std::vector<std::string> REGISTERS32 = {
    "eax", "ecx", "edx",  "ebx",  "esp",  "ebp",  "esi",  "edi",
    "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"};
const std::vector<std::string> REGISTERS64 = {
    "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
    "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"};

// Define addressing modes (no need for square brackets)
const std::vector<std::string> addressingModes = {"reg", "reg + disp8",
                                                  "reg + disp32"};

// Define scale factors
// const std::vector<std::string> SCALE = {"index + base", "index * 2 + base",
//                                        "index * 4 + base", "index * 8 +
//                                        base"};

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
        case Operand::imm64:
            return "imm64";
        case Operand::reg8:
            return "reg8";
        case Operand::reg16:
            return "reg16";
        case Operand::reg32:
            return "reg32";
        case Operand::reg64:
            return "reg64";
        case Operand::rm8:
            return "rm8";
        case Operand::rm16:
            return "rm16";
        case Operand::rm32:
            return "rm32";
        case Operand::rm64:
            return "rm64";
        case Operand::eax:
            return "eax";
        case Operand::moffs8:
            return "moffs8";
        case Operand::moffs16:
            return "moffs8";
        case Operand::moffs32:
            return "moffs8";
        case Operand::moffs64:
            return "moffs8";
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

const std::vector<int> SCALE_FACTOR = {1, 2, 4, 8};
