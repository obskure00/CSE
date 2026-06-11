#include "Instructions.hpp"
#include "CPU.hpp"
#include "Bus.hpp"
#include <iostream>

bool execute(CPU& cpu, Opcode op) {
    CPUState& state = cpu.state;
    uint8_t rd, rs;

    switch (op) {

        case Opcode::NOP: {
            break;
        }

        case Opcode::MOV: {
            cpu.fetchRegPair(rd, rs);
            state.R[rd] = state.R[rs];
            break;
        }

        case Opcode::LDI: {
            rd        = cpu.fetchByte() & 0x07;
            state.R[rd]   = cpu.fetchByte();
            break;
        }

        case Opcode::LDR: {
            cpu.fetchRegPair(rd, rs);
            uint16_t addr = (static_cast<uint16_t>(state.R[rs]) << 8)
                        |  static_cast<uint16_t>(state.R[(rs + 1) & 0x07]);
            state.R[rd] = cpu.getBus().read(addr);
            break;
        }

        case Opcode::STR: {
            cpu.fetchRegPair(rs, rd);
            uint16_t addr = (static_cast<uint16_t>(state.R[rd]) << 8)
                        |  static_cast<uint16_t>(state.R[(rd + 1) & 0x07]);
            cpu.getBus().write(addr, state.R[rs]);
            break;
        }

        case Opcode::ADD: {
            cpu.fetchRegPair(rd, rs);
            uint16_t result = state.R[rd] + state.R[rs];
            cpu.setArithmeticFlags(result, state.R[rd], state.R[rs], false);
            state.R[rd] = static_cast<uint8_t>(result);
            break;
        }

        case Opcode::SUB: {
            cpu.fetchRegPair(rd, rs);
            uint16_t result = static_cast<uint16_t>(state.R[rd])
                            - static_cast<uint16_t>(state.R[rs]);
            cpu.setArithmeticFlags(result, state.R[rd], state.R[rs], true);
            state.R[rd] = static_cast<uint8_t>(result);
            break;
        }

        case Opcode::AND: {
            cpu.fetchRegPair(rd, rs);
            state.R[rd] &= state.R[rs];
            cpu.setLogicFlags(state.R[rd]);
            break;
        }

        case Opcode::OR: {
            cpu.fetchRegPair(rd, rs);
            state.R[rd] |= state.R[rs];
            cpu.setLogicFlags(state.R[rd]);
            break;
        }

        case Opcode::XOR: {
            cpu.fetchRegPair(rd, rs);
            state.R[rd] ^= state.R[rs];
            cpu.setLogicFlags(state.R[rd]);
            break;
        }

        case Opcode::NOT: {
            rd      = cpu.fetchByte() & 0x07;
            state.R[rd] = ~state.R[rd];
            cpu.setLogicFlags(state.R[rd]);
            break;
        }

        case Opcode::SHL: {
            rd           = cpu.fetchByte() & 0x07;
            bool carry   = (state.R[rd] & 0x80) != 0;
            state.R[rd]    <<= 1;
            cpu.setLogicFlags(state.R[rd]);
            state.SR.C       = carry;
            break;
        }

        case Opcode::SHR: {
            rd           = cpu.fetchByte() & 0x07;
            bool carry   = (state.R[rd] & 0x01) != 0;
            state.R[rd]    >>= 1;
            cpu.setLogicFlags(state.R[rd]);
            state.SR.C       = carry;
            break;
        }

        case Opcode::CMP: {
            cpu.fetchRegPair(rd, rs);
            uint16_t result = static_cast<uint16_t>(state.R[rd])
                            - static_cast<uint16_t>(state.R[rs]);
            cpu.setArithmeticFlags(result, state.R[rd], state.R[rs], true);
            break;
        }

        case Opcode::JMP: {
            uint8_t hi = cpu.fetchByte();
            uint8_t lo = cpu.fetchByte();
            state.PC = (static_cast<uint16_t>(hi) << 8) | lo;
            break;
        }

        case Opcode::JZ: {
            uint8_t hi = cpu.fetchByte();
            uint8_t lo = cpu.fetchByte();
            if (state.SR.Z) state.PC = (static_cast<uint16_t>(hi) << 8) | lo;
            break;
        }

        case Opcode::JNZ: {
            uint8_t hi = cpu.fetchByte();
            uint8_t lo = cpu.fetchByte();
            if (!state.SR.Z) state.PC = (static_cast<uint16_t>(hi) << 8) | lo;
            break;
        }

        case Opcode::JC: {
            uint8_t hi = cpu.fetchByte();
            uint8_t lo = cpu.fetchByte();
            if (state.SR.C) state.PC = (static_cast<uint16_t>(hi) << 8) | lo;
            break;
        }

        case Opcode::JN: {
            uint8_t hi = cpu.fetchByte();
            uint8_t lo = cpu.fetchByte();
            if (state.SR.S) state.PC = (static_cast<uint16_t>(hi) << 8) | lo;
            break;
        }

        case Opcode::JV: {
            uint8_t hi = cpu.fetchByte();
            uint8_t lo = cpu.fetchByte();
            if (state.SR.V) state.PC = (static_cast<uint16_t>(hi) << 8) | lo;
            break;
        }

        case Opcode::CALL: {
            uint8_t hi = cpu.fetchByte();
            uint8_t lo = cpu.fetchByte();
            cpu.stackPush16(state.PC);
            state.PC = (static_cast<uint16_t>(hi) << 8) | lo;
            break;
        }

        case Opcode::RET: {
            state.PC = cpu.stackPop16();
            break;
        }

        case Opcode::PUSH: {
            rs = cpu.fetchByte() & 0x07;
            cpu.stackPush(state.R[rs]);
            break;
        }

        case Opcode::POP: {
            rd = cpu.fetchByte() & 0x07;
            state.R[rd] = cpu.stackPop();
            break;
        }

        case Opcode::OUTN: {
            rs = cpu.fetchByte() & 0x07;
            std::cout << static_cast<int>(state.R[rs]);
            std::cout.flush();
            break;
        }

        case Opcode::OUT: {
            rs = cpu.fetchByte() & 0x07;
            std::cout << static_cast<char>(state.R[rs]);
            std::cout.flush();
            break;
        }

        case Opcode::HLT: {
        default:
            cpu.halted = true;
            return false;
        }
    }

    return true;
}