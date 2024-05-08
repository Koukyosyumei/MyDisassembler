#include <iostream>
#include <unordered_set>

#include "constants.h"
#include "table.h"

    std::unordered_set<Mnemonic> mnemonicSet = {
        Mnemonic::SHLD, Mnemonic::SHRD, Mnemonic::MOV, Mnemonic::MOVSX, Mnemonic::MOVSXD,
        Mnemonic::MOVZX, Mnemonic::LEA, Mnemonic::ADD, Mnemonic::ADC, Mnemonic::SUB,
        Mnemonic::SBB, Mnemonic::MUL, Mnemonic::IMUL, Mnemonic::DIV, Mnemonic::IDIV,
        Mnemonic::INC, Mnemonic::DEC, Mnemonic::AND, Mnemonic::OR, Mnemonic::XOR,
        Mnemonic::NOT, Mnemonic::NEG, Mnemonic::CMP, Mnemonic::TEST, Mnemonic::SAL,
        Mnemonic::SHL, Mnemonic::SAR, Mnemonic::SHR, Mnemonic::RCL, Mnemonic::RCR,
        Mnemonic::ROL, Mnemonic::ROR, Mnemonic::JMP, Mnemonic::LOOP, Mnemonic::JZ,
        Mnemonic::JNZ, Mnemonic::JA, Mnemonic::JAE, Mnemonic::JB, Mnemonic::JBE,
        Mnemonic::JG, Mnemonic::JGE, Mnemonic::JL, Mnemonic::JLE, Mnemonic::JP,
        Mnemonic::JNP, Mnemonic::JO, Mnemonic::JNO, Mnemonic::JS, Mnemonic::JC,
        Mnemonic::JCXZ, Mnemonic::JECXZ, Mnemonic::JNB, Mnemonic::JNBE, Mnemonic::JNC,
        Mnemonic::JNE, Mnemonic::JNG, Mnemonic::JNGE, Mnemonic::JNL, Mnemonic::JNLE,
        Mnemonic::JNS, Mnemonic::JNAE, Mnemonic::JNA, Mnemonic::JZBE, Mnemonic::JPO,
        Mnemonic::CALL, Mnemonic::RET, Mnemonic::PUSH, Mnemonic::POP, Mnemonic::MOVSB,
        Mnemonic::MOVSW, Mnemonic::MOVSD, Mnemonic::REP, Mnemonic::REPE, Mnemonic::REPNE,
        Mnemonic::CLD, Mnemonic::STD, Mnemonic::LODSB, Mnemonic::LODSW, Mnemonic::LODSD,
        Mnemonic::STOSB, Mnemonic::STOSW, Mnemonic::STOSD, Mnemonic::SCASB, Mnemonic::SCASW,
        Mnemonic::SCASD, Mnemonic::CMPSB, Mnemonic::CMPSW, Mnemonic::CMPSD, Mnemonic::IN,
        Mnemonic::OUT, Mnemonic::INSB, Mnemonic::INSW, Mnemonic::INSD, Mnemonic::OUTSB,
        Mnemonic::OUTSW, Mnemonic::OUTSD, Mnemonic::CBW, Mnemonic::CWD, Mnemonic::CWDE,
        Mnemonic::CDQ, Mnemonic::CDQE, Mnemonic::CQO, Mnemonic::INT21, Mnemonic::LOCK,
        Mnemonic::ENTER, Mnemonic::LEAVE, Mnemonic::NOP, Mnemonic::UD2, Mnemonic::CPUID,
        Mnemonic::XCHG, Mnemonic::STC, Mnemonic::CLC, Mnemonic::BSWAP
    };

int main() {
    std::unordered_set<Mnemonic> supported_ms;
    for (const auto& kv : OPERAND_LOOKUP) {
        supported_ms.insert(std::get<1>(kv.first));
    }

    for (Mnemonic m : mnemonicSet) {
        if (supported_ms.find(m) == supported_ms.end()) {
            std::cout << to_string(m) << std::endl;
        }
    }
}
