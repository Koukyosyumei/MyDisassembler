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

inline bool isIMM(Operand operand) {
    return operand == Operand::imm8 || operand == Operand::imm16 ||
           operand == Operand::imm32 || operand == Operand::imm64;
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

enum class OpEnc { I, D, M, O, NP, MI, M1, MR, RM, RMI, OI, FD, TD };

enum class Prefix {
    NONE,  // without prefix
    P66,   // change the default operand size
    REXW,  // use R8-R15 registers
    REX    // use 64 bit registers
};

inline std::string to_string(Prefix prefix) {
    switch (prefix) {
        case Prefix::NONE:
            return "none";
        case Prefix::P66:
            return "66";
        case Prefix::REXW:
            return "REX.W";
        case Prefix::REX:
            return "REX";
    }
}

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
            return "mov";
        case Mnemonic::MOVSX:
            return "movsx";
        case Mnemonic::MOVSXD:
            return "movsxd";
        case Mnemonic::MOVZX:
            return "movzx";
        case Mnemonic::LEA:
            return "lea";
        case Mnemonic::ADD:
            return "add";
        case Mnemonic::ADC:
            return "adc";
        case Mnemonic::SUB:
            return "sub";
        case Mnemonic::SBB:
            return "sbb";
        case Mnemonic::MUL:
            return "mul";
        case Mnemonic::IMUL:
            return "imul";
        case Mnemonic::DIV:
            return "div";
        case Mnemonic::IDIV:
            return "idiv";
        case Mnemonic::INC:
            return "inc";
        case Mnemonic::DEC:
            return "dec";
        case Mnemonic::AND:
            return "and";
        case Mnemonic::OR:
            return "or";
        case Mnemonic::XOR:
            return "xor";
        case Mnemonic::NOT:
            return "not";
        case Mnemonic::NEG:
            return "neg";
        case Mnemonic::CMP:
            return "cmp";
        case Mnemonic::TEST:
            return "test";
        case Mnemonic::SAL:
            return "sal";
        case Mnemonic::SHL:
            return "shl";
        case Mnemonic::SAR:
            return "sar";
        case Mnemonic::SHR:
            return "shr";
        case Mnemonic::RCL:
            return "rcl";
        case Mnemonic::RCR:
            return "rcr";
        case Mnemonic::ROL:
            return "rol";
        case Mnemonic::ROR:
            return "ror";
        case Mnemonic::JMP:
            return "jmp";
        case Mnemonic::LOOP:
            return "loop";
        case Mnemonic::JZ:
            return "jz";
        case Mnemonic::JNZ:
            return "jnz";
        case Mnemonic::JA:
            return "ja";
        case Mnemonic::JAE:
            return "jae";
        case Mnemonic::JB:
            return "jb";
        case Mnemonic::JBE:
            return "jbe";
        case Mnemonic::JG:
            return "jg";
        case Mnemonic::JGE:
            return "jge";
        case Mnemonic::JL:
            return "jl";
        case Mnemonic::JLE:
            return "jle";
        case Mnemonic::JP:
            return "jp";
        case Mnemonic::JNP:
            return "jnp";
        case Mnemonic::JO:
            return "jo";
        case Mnemonic::JNO:
            return "jno";
        case Mnemonic::JS:
            return "js";
        case Mnemonic::JC:
            return "jc";
        case Mnemonic::JCXZ:
            return "jcxz";
        case Mnemonic::JECXZ:
            return "jecxz";
        case Mnemonic::CALL:
            return "call";
        case Mnemonic::RET:
            return "ret";
        case Mnemonic::PUSH:
            return "push";
        case Mnemonic::POP:
            return "pop";
        case Mnemonic::MOVSB:
            return "movsb";
        case Mnemonic::MOVSW:
            return "movsw";
        case Mnemonic::MOVSD:
            return "movsd";
        case Mnemonic::REP:
            return "rep";
        case Mnemonic::REPE:
            return "repe";
        case Mnemonic::REPNE:
            return "repne";
        case Mnemonic::CLD:
            return "cld";
        case Mnemonic::STD:
            return "std";
        case Mnemonic::LODSB:
            return "lodsb";
        case Mnemonic::LODSW:
            return "lodsw";
        case Mnemonic::LODSD:
            return "lodsd";
        case Mnemonic::STOSB:
            return "stosb";
        case Mnemonic::STOSW:
            return "stosw";
        case Mnemonic::STOSD:
            return "stosd";
        case Mnemonic::SCASB:
            return "scasb";
        case Mnemonic::SCASW:
            return "scasw";
        case Mnemonic::SCASD:
            return "scasd";
        case Mnemonic::CMPSB:
            return "cmpsb";
        case Mnemonic::CMPSW:
            return "cmpsw";
        case Mnemonic::CMPSD:
            return "cmpsd";
        case Mnemonic::IN:
            return "in";
        case Mnemonic::OUT:
            return "out";
        case Mnemonic::INSB:
            return "insb";
        case Mnemonic::INSW:
            return "insw";
        case Mnemonic::INSD:
            return "insd";
        case Mnemonic::OUTSB:
            return "outsb";
        case Mnemonic::OUTSW:
            return "outsw";
        case Mnemonic::OUTSD:
            return "outsd";
        case Mnemonic::CBW:
            return "cbw";
        case Mnemonic::CWD:
            return "cwd";
        case Mnemonic::CWDE:
            return "cwde";
        case Mnemonic::CDQ:
            return "cdq";
        case Mnemonic::CDQE:
            return "cdqe";
        case Mnemonic::CQO:
            return "cqo";
        case Mnemonic::INT21:
            return "int21";
        case Mnemonic::LOCK:
            return "lock";
        case Mnemonic::ENTER:
            return "enter";
        case Mnemonic::LEAVE:
            return "leave";
        case Mnemonic::NOP:
            return "nop";
        case Mnemonic::UD2:
            return "ud2";
        case Mnemonic::CPUID:
            return "cpuid";
        case Mnemonic::XCHG:
            return "xchg";
        case Mnemonic::STC:
            return "stc";
        case Mnemonic::CLC:
            return "clc";
        default:
            return "unknown";
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

inline const std::vector<std::string>* operand2register(Operand operand) {
    if (is8Bit(operand)) {
        return &REGISTERS8;
    } else if (is16Bit(operand)) {
        return &REGISTERS16;
    } else if (is32Bit(operand)) {
        return &REGISTERS32;
    } else if (is64Bit(operand)) {
        return &REGISTERS64;
    }
    return nullptr;
}

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
        case OpEnc::FD:
            return false;
        case OpEnc::TD:
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
        case OpEnc::FD:
            return "FD";
        case OpEnc::TD:
            return "TD";
    }
}

const std::unordered_set<int> TWO_BYTES_OPCODE_PREFIX = {0x0F};

// Predefined prefixes and their associated instructions
const std::unordered_map<int, std::vector<std::string>>
    BYTE2PREFIX_INSTRUCTIONS = {
        {0xF0, {"LOCK"}},
        {0xF2, {"REPNE", "REPNZ"}},
        {0xF3, {"REP", "REPE", "REPZ"}},
};

const std::unordered_set<int> PREFIX_INSTRUCTIONS_BYTES_SET = {0xF0, 0xF2,
                                                               0xF3};

const std::vector<int> SCALE_FACTOR = {1, 2, 4, 8};
