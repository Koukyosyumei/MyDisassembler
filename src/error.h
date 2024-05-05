#include <stdexcept>
#include <string>

class OPCODE_LOOKUP_ERROR : public std::runtime_error {
   public:
    OPCODE_LOOKUP_ERROR(const std::string& message)
        : std::runtime_error(message) {}
};

class OPERAND_LOOKUP_ERROR : public std::runtime_error {
   public:
    OPERAND_LOOKUP_ERROR(const std::string& message)
        : std::runtime_error(message) {}
};

class InvalidOperandError : public std::runtime_error {
   public:
    InvalidOperandError(const std::string& message)
        : std::runtime_error(message) {}
};

