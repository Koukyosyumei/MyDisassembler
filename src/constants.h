#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
    xmm,
    ymm,
    xm128,
    ym256,
    m,
    m32fp,
    m64fp,
    al,
    ax,
    eax,
    rax,
    moffs8,
    moffs16,
    moffs32,
    moffs64,
    cl,
    dx,
    st0,
    sti,
};

inline bool isA_REG(Operand operand) {
    return operand == Operand::al || operand == Operand::ax ||
           operand == Operand::eax || operand == Operand::rax;
}

inline bool isRM(Operand operand) {
    return operand == Operand::rm8 || operand == Operand::rm16 ||
           operand == Operand::rm32 || operand == Operand::rm64 ||
           operand == Operand::xm128;
}

inline bool isM(Operand operand) {
    return operand == Operand::m || operand == Operand::m32fp ||
           operand == Operand::m64fp;
}

inline bool isREG(Operand operand) {
    return operand == Operand::reg8 || operand == Operand::reg16 ||
           operand == Operand::reg32 || operand == Operand::reg64 ||
           operand == Operand::xmm || operand == Operand::ymm;
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

enum class OpEnc {
    I,
    D,
    M,
    O,
    NP,
    MC,
    MI,
    M1,
    MR,
    RM,
    RMI,
    MRI,
    MRC,
    OI,
    FD,
    TD,
    S,
    A,
    B,
    C,
};

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
        default:
            return "UNKNOWN";
    }
}

enum class Mnemonic {
    SETNE,
    FXCH,
    FADD,
    ENDBR64,
    ENDBR32,
    SHLD,
    SHRD,
    MOV,
    CMOVE,
    MOVSX,
    MOVSXD,
    MOVZX,
    MOVAPS,
    SCASQ,
    LODSQ,
    STOSQ,
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
    LOOPE,
    LOOPNE,
    JZ,
    JNZ,
    JP,
    JO,
    JNO,
    JS,
    JECXZ,
    JNB,
    JNBE,
    JNG,
    JNGE,
    JNL,
    JNLE,
    JNS,
    JNAE,
    JNA,
    JPO,
    CALL,
    RET,
    PUSH,
    POP,
    MOVSB,
    MOVSW,
    MOVSD,
    MOVSQ,
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
    CMPSQ,
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
    ENTER,
    LEAVE,
    NOP,
    UD2,
    CPUID,
    XCHG,
    STC,
    CLC,
    BSWAP,
};

inline std::string to_string(Mnemonic mnemonic) {
    switch (mnemonic) {
        case Mnemonic::SETNE:
            return "setne";
        case Mnemonic::FADD:
            return "fadd";
        case Mnemonic::FXCH:
            return "fxch";
        case Mnemonic::ENDBR64:
            return "endbr64";
        case Mnemonic::ENDBR32:
            return "endbr32";
        case Mnemonic::SHLD:
            return "shld";
        case Mnemonic::SHRD:
            return "shrd";
        case Mnemonic::MOV:
            return "mov";
        case Mnemonic::CMOVE:
            return "cmove";
        case Mnemonic::MOVSX:
            return "movsx";
        case Mnemonic::MOVSXD:
            return "movsxd";
        case Mnemonic::MOVZX:
            return "movzx";
        case Mnemonic::MOVAPS:
            return "movaps";
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
        case Mnemonic::LODSQ:
            return "lodsq";
        case Mnemonic::SCASQ:
            return "scasq";
        case Mnemonic::STOSQ:
            return "stosq";
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
        case Mnemonic::LOOPE:
            return "loope";
        case Mnemonic::LOOPNE:
            return "loopne";
        case Mnemonic::JZ:
            return "jz";
        case Mnemonic::JNZ:
            return "jnz";
        case Mnemonic::JP:
            return "jp";
        case Mnemonic::JO:
            return "jo";
        case Mnemonic::JNO:
            return "jno";
        case Mnemonic::JS:
            return "js";
        case Mnemonic::JECXZ:
            return "jecxz";
        case Mnemonic::JNB:
            return "jnb";
        case Mnemonic::JNBE:
            return "jnbe";
        case Mnemonic::JNG:
            return "jng";
        case Mnemonic::JNGE:
            return "jnge";
        case Mnemonic::JNL:
            return "jnl";
        case Mnemonic::JNLE:
            return "jnle";
        case Mnemonic::JNS:
            return "jns";
        case Mnemonic::JNAE:
            return "jnae";
        case Mnemonic::JNA:
            return "jna";
        case Mnemonic::JPO:
            return "jpo";
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
        case Mnemonic::MOVSQ:
            return "movsq";
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
        case Mnemonic::CMPSQ:
            return "cmpsq";
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
        case Mnemonic::BSWAP:
            return "bswap";
        default:
            return "unknown";
    }
}

inline bool isJCCInstruction(Mnemonic mnemonic) {
    return mnemonic == Mnemonic::JO || mnemonic == Mnemonic::JNO ||
           mnemonic == Mnemonic::JNAE || mnemonic == Mnemonic::JNB ||
           mnemonic == Mnemonic::JZ || mnemonic == Mnemonic::JNZ ||
           mnemonic == Mnemonic::JNA || mnemonic == Mnemonic::JNBE ||
           mnemonic == Mnemonic::JS || mnemonic == Mnemonic::JNS ||
           mnemonic == Mnemonic::JP || mnemonic == Mnemonic::JPO ||
           mnemonic == Mnemonic::JNGE || mnemonic == Mnemonic::JNL ||
           mnemonic == Mnemonic::JNG || mnemonic == Mnemonic::JNLE;
}

inline bool isLOOPInstruction(Mnemonic mnemonic) {
    return mnemonic == Mnemonic::LOOP || mnemonic == Mnemonic::LOOPE ||
           mnemonic == Mnemonic::LOOPNE;
}

inline bool isControlFlowInstruction(Mnemonic mnemonic) {
    return mnemonic == Mnemonic::CALL || mnemonic == Mnemonic::JMP ||
           isJCCInstruction(mnemonic) || isLOOPInstruction(mnemonic);
}

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
        case Operand::al:
            return "al";
        case Operand::ax:
            return "ax";
        case Operand::eax:
            return "eax";
        case Operand::rax:
            return "rax";
        case Operand::moffs8:
            return "moffs8";
        case Operand::moffs16:
            return "moffs8";
        case Operand::moffs32:
            return "moffs8";
        case Operand::moffs64:
            return "moffs8";
        case Operand::cl:
            return "cl";
        case Operand::dx:
            return "dx";
        default:
            return "unknown";
    }
}

inline bool hasModrm(OpEnc openc) {
    switch (openc) {
        case OpEnc::I:
            return false;
        case OpEnc::A:
            return true;
        case OpEnc::B:
            return true;
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
        case OpEnc::MC:
            return true;
        case OpEnc::MR:
            return true;
        case OpEnc::RM:
            return true;
        case OpEnc::RMI:
            return true;
        case OpEnc::MRI:
            return true;
        case OpEnc::MRC:
            return true;
        case OpEnc::OI:
            return false;
        case OpEnc::FD:
            return false;
        case OpEnc::TD:
            return false;
        case OpEnc::S:
            return false;
        default:
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
        case OpEnc::MC:
            return "MC";
        case OpEnc::RM:
            return "RM";
        case OpEnc::RMI:
            return "RMI";
        case OpEnc::MRI:
            return "MRI";
        case OpEnc::MRC:
            return "MRC";
        case OpEnc::OI:
            return "OI";
        case OpEnc::FD:
            return "FD";
        case OpEnc::TD:
            return "TD";
        case OpEnc::S:
            return "S";
        default:
            return "UNKNOWN";
    }
}

const std::unordered_set<int> TWO_BYTES_OPCODE_PREFIX = {0x0F, 0xD8, 0xD9,
                                                         0xDC};

// Predefined prefixes and their associated instructions
const std::unordered_set<int> INSTRUCTION_PREFIX_SET = {
    0xF0,
    0xF2,
    0xF3,
    0x3E,
};

const std::vector<int> SCALE_FACTOR = {1, 2, 4, 8};

const size_t PLT_SEC_ENTRY_SIZE = 16;
