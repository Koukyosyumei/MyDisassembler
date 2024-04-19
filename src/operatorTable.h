#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "constants.h"

template <class T>
size_t HashCombine(const size_t seed, const T& v) {
    return seed ^ (std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}
template <class T, class S>
struct std::hash<std::pair<T, S>> {
    size_t operator()(const std::pair<T, S>& keyval) const noexcept {
        return HashCombine(std::hash<T>()(keyval.first), keyval.second);
    }
};

// Enums for operand units and encoding
enum class OpUnit {
    eax,
    imm8,
    imm32,
    reg,
    rm,
};

enum class OpEnc {
    I,   // Immediate
    MI,  // Memory with immediate
    MR,  // Memory with register
    RM,  // Register with memory
};

// Structure for operand information
struct Operand {
    OpUnit type;
    std::string value;  // Can be empty for registers (e.g., "eax")
};

inline int utf8ToDecimal(const std::string& utf8_str) {
    int result = 0;
    for (unsigned char c : utf8_str) {
        result = (result << 8) + c;
    }
    return result;
}

// Function to populate OP_LOOKUP based on operator, opcode, and register (if
// applicable)
inline void updateOPLOOKUP(
    std::unordered_map<std::pair<int, int>,
                       std::unordered_map<int, std::string>>& op_lookup,
    const std::string& operator_, int prefix, int opcode, int reg) {
    std::pair<int, int> key = {prefix, opcode};

    if (op_lookup.count(key) == 0) {
        op_lookup[key] = {};
    }

    if (reg != -1) {
        op_lookup[key][reg] = operator_;
    }
}

// inline void updateOPERANDLOOKUP()

inline std::unordered_map<std::pair<int, int>,
                          std::unordered_map<int, std::string>>
initializeOPLOOKUP() {
    // (prefix, opcode) -> (register, operator)
    std::unordered_map<std::pair<int, int>,
                       std::unordered_map<int, std::string>>
        op_lookup;

    // (operator, opcode) -> (encoding, mnemonic, operands)
    std::unordered_map<std::pair<std::string, int>,
                         std::tuple<OpEnc, std::string, std::vector<Operand>>>
    operand_lookup;      

    /*
    # ADD
        Opcode              Instruction         Op/En   64-Bit  Compat  Description
        05 id               ADD EAX, imm32      I       Valid   Valid   Add imm32 to EAX.
        81 /0 id            ADD r/m32, imm32    MI      Valid   Valid   Add imm32 to r/m32.
        83 /0 ib            ADD r/m32, imm8     MI      Valid   Valid   Add sign-extended imm8 to r/m32.
        01 /r               ADD r/m32, r32      MR      Valid   Valid   Add r32 to r/m32.
        03 /r               ADD r32, r/m32      RM      Valid   Valid   Add r/m32 to r32.
    */
    updateOPLOOKUP(op_lookup, "ADD", -1, utf8ToDecimal("\x05"), -1);
    updateOPLOOKUP(op_lookup, "ADD", -1, utf8ToDecimal("\x81"), utf8ToDecimal("\x00"));
    updateOPLOOKUP(op_lookup, "ADD", -1, utf8ToDecimal("\x83"), utf8ToDecimal("\x00"));
    updateOPLOOKUP(op_lookup, "ADD", -1, utf8ToDecimal("\x01"), -1);
    updateOPLOOKUP(op_lookup, "ADD", -1, utf8ToDecimal("\x03"), -1);
    return op_lookup;
}

// Global lookup table for instructions
// (prefix, opcode) -> (register, operator)
const std::unordered_map<std::pair<int, int>,
                         std::unordered_map<int, std::string>>
    OP_LOOKUP = initializeOPLOOKUP();

// Lookup table for operand information
const std::unordered_map<std::pair<std::string, int>,
                         std::tuple<OpEnc, std::string, std::vector<Operand>>>
    OPERAND_LOOKUP;  // (operator, opcode) -> (encoding, mnemonic, operands)

// Supported operators set
const std::unordered_set<std::string> SUPPORTED_OPERATORS;

// Predefined prefixes and their associated instructions
const std::unordered_map<int, std::vector<std::string>> PREFIX_OP = {
    {0x0F, {"IMUL", "JZ", "JNZ"}},
    {0xF0, {"LOCK"}},
    {0xF2, {"REPNE", "REPNZ"}},
    {0xF3, {"REP", "REPE", "REPZ"}},
};

const std::unordered_set<int> PREFIX_SET = {0x0F, 0xF0, 0xF2, 0xF3};

