#include "Assembler.hpp"
 
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
 
namespace {
 
    std::string trim(const std::string& s) {
        size_t a = 0;
        while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
        size_t b = s.size();
        while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
        return s.substr(a, b - a);
    }
    
    std::string upper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                    [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return s;
    }
    
    std::string stripComment(const std::string& s) {
        auto p = s.find(';');
        return p == std::string::npos ? s : s.substr(0, p);
    }
    
    std::vector<std::string> splitOperands(const std::string& s) {
        std::vector<std::string> out;
        std::string cur;
        for (char c : s) {
            if (c == ',') {
                cur = trim(cur);
                if (!cur.empty()) out.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        cur = trim(cur);
        if (!cur.empty()) out.push_back(cur);
        return out;
    }
    
    int parseRegister(const std::string& tok) {
        std::string t = upper(trim(tok));
        if (t.size() == 2 && t[0] == 'R' && t[1] >= '0' && t[1] <= '7') return t[1] - '0';
        if (t == "A") return 0;
        if (t == "B") return 1;
        if (t == "C") return 2;
        if (t == "D") return 3;
        if (t == "E") return 4;
        if (t == "F") return 5;
        if (t == "G") return 6;
        if (t == "H") return 7;
        throw std::runtime_error("invalid register: " + tok);
    }
    
    uint16_t parseValue(const std::string& tok,
                        const std::unordered_map<std::string, uint16_t>& labels) {
        std::string t = trim(tok);
        if (t.empty()) throw std::runtime_error("empty value");
    
        std::string u = upper(t);
        auto it = labels.find(u);
        if (it != labels.end()) return it->second;
    
        if (t.size() >= 3 && t.front() == '\'' && t.back() == '\'') {
            if (t.size() == 3) return static_cast<unsigned char>(t[1]);
            if (t.size() == 4 && t[1] == '\\') {
                switch (t[2]) {
                    case 'n': return '\n';
                    case 'r': return '\r';
                    case 't': return '\t';
                    case '0': return '\0';
                    case '\\': return '\\';
                    case '\'': return '\'';
                }
            }
            throw std::runtime_error("invalid char literal: " + t);
        }
    
        if (t.size() > 2 && t[0] == '0' && (t[1] == 'x' || t[1] == 'X'))
            return static_cast<uint16_t>(std::stoul(t, nullptr, 16));
    
        if (t.size() > 2 && t[0] == '0' && (t[1] == 'b' || t[1] == 'B')) {
            uint16_t v = 0;
            for (size_t i = 2; i < t.size(); ++i) {
                if (t[i] != '0' && t[i] != '1')
                    throw std::runtime_error("invalid binary literal: " + t);
                v = static_cast<uint16_t>((v << 1) | (t[i] - '0'));
            }
            return v;
        }
    
        return static_cast<uint16_t>(std::stoul(t, nullptr, 10));
    }
    
    enum class Kind { None, OneReg, TwoReg, RegImm8, Addr16 };
    
    struct OpInfo {
        uint8_t opcode;
        Kind kind;
    };
    
    const std::unordered_map<std::string, OpInfo> ops = {
        {"NOP",  {0x00, Kind::None}},
        {"MOV",  {0x10, Kind::TwoReg}},
        {"LDI",  {0x11, Kind::RegImm8}},
        {"LDR",  {0x12, Kind::TwoReg}},
        {"STR",  {0x13, Kind::TwoReg}},
        {"ADD",  {0x20, Kind::TwoReg}},
        {"SUB",  {0x21, Kind::TwoReg}},
        {"AND",  {0x22, Kind::TwoReg}},
        {"OR",   {0x23, Kind::TwoReg}},
        {"XOR",  {0x24, Kind::TwoReg}},
        {"NOT",  {0x25, Kind::OneReg}},
        {"SHL",  {0x26, Kind::OneReg}},
        {"SHR",  {0x27, Kind::OneReg}},
        {"CMP",  {0x28, Kind::TwoReg}},
        {"JMP",  {0x30, Kind::Addr16}},
        {"JZ",   {0x31, Kind::Addr16}},
        {"JNZ",  {0x32, Kind::Addr16}},
        {"JC",   {0x33, Kind::Addr16}},
        {"JN",   {0x34, Kind::Addr16}},
        {"JV",   {0x35, Kind::Addr16}},
        {"CALL", {0x40, Kind::Addr16}},
        {"RET",  {0x41, Kind::None}},
        {"PUSH", {0x50, Kind::OneReg}},
        {"POP",  {0x51, Kind::OneReg}},
        {"OUT",  {0x60, Kind::OneReg}},
        {"OUTN", {0x61, Kind::OneReg}},
        {"HLT",  {0xFF, Kind::None}},
    };
    
    size_t instrSize(const std::string& mnem) {
        auto it = ops.find(mnem);
        if (it == ops.end()) throw std::runtime_error("unknown instruction: " + mnem);
        switch (it->second.kind) {
        case Kind::None: return 1;
        case Kind::OneReg: return 2;
        case Kind::TwoReg: return 2;
        case Kind::RegImm8: return 3;
        case Kind::Addr16: return 3;
        }
        return 0;
    }
    
    void emit(const std::string& mnem,
            const std::vector<std::string>& operands,
            const std::unordered_map<std::string, uint16_t>& labels,
            std::vector<uint8_t>& out) {
        auto it = ops.find(mnem);
        if (it == ops.end()) throw std::runtime_error("unknown instruction: " + mnem);
    
        out.push_back(it->second.opcode);
    
        switch (it->second.kind) {
            case Kind::None:
                if (!operands.empty()) throw std::runtime_error(mnem + " takes no operands");
                break;
    
            case Kind::OneReg:
                if (operands.size() != 1) throw std::runtime_error(mnem + " needs 1 operand");
                out.push_back(static_cast<uint8_t>(parseRegister(operands[0])));
                break;
    
            case Kind::TwoReg: {
                if (operands.size() != 2) throw std::runtime_error(mnem + " needs 2 operands");
                uint8_t rd = static_cast<uint8_t>(parseRegister(operands[0])) & 0x07;
                uint8_t rs = static_cast<uint8_t>(parseRegister(operands[1])) & 0x07;
                out.push_back(static_cast<uint8_t>((rd << 4) | rs));
                break;
            }
    
            case Kind::RegImm8: {
                if (operands.size() != 2) throw std::runtime_error(mnem + " needs 2 operands");
                auto imm = parseValue(operands[1], labels);
                if (imm > 0xFF) throw std::runtime_error("8-bit immediate out of range");
                out.push_back(static_cast<uint8_t>(parseRegister(operands[0])));
                out.push_back(static_cast<uint8_t>(imm));
                break;
            }
    
            case Kind::Addr16: {
                if (operands.size() != 1) throw std::runtime_error(mnem + " needs 1 operand");
                auto addr = parseValue(operands[0], labels);
                out.push_back(static_cast<uint8_t>((addr >> 8) & 0xFF));
                out.push_back(static_cast<uint8_t>(addr & 0xFF));
                break;
            }
        }
    }
    
    } // anonymous namespace
    
    namespace Assembler {
    
    std::vector<uint8_t> assemble(const std::string& path) {
        std::ifstream in(path);
        if (!in) throw std::runtime_error("could not open input file: " + path);
    
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(in, line)) lines.push_back(line);
    
        std::unordered_map<std::string, uint16_t> labels;
        uint16_t pc = 0;
    
        // Pass 1: collect label addresses
        for (const auto& raw : lines) {
            std::string text = trim(stripComment(raw));
            if (text.empty()) continue;
    
            while (true) {
                auto pos = text.find(':');
                if (pos == std::string::npos) break;
                std::string label = upper(trim(text.substr(0, pos)));
                if (label.empty()) break;
                labels[label] = pc;
                text = trim(text.substr(pos + 1));
                if (text.empty()) break;
            }
    
            if (text.empty()) continue;
    
            std::istringstream iss(text);
            std::string mnem;
            iss >> mnem;
            mnem = upper(mnem);
    
            if (mnem == ".ORG") {
                std::string v;
                iss >> v;
                pc = parseValue(v, labels);
            } else if (mnem == ".DB") {
                std::string rest;
                std::getline(iss, rest);
                pc = static_cast<uint16_t>(pc + splitOperands(rest).size());
            } else {
                pc = static_cast<uint16_t>(pc + instrSize(mnem));
            }
        }
    
        // Pass 2: emit bytes
        std::vector<uint8_t> out;
        pc = 0;
    
        for (const auto& raw : lines) {
            std::string text = trim(stripComment(raw));
            if (text.empty()) continue;
    
            while (true) {
                auto pos = text.find(':');
                if (pos == std::string::npos) break;
                text = trim(text.substr(pos + 1));
                if (text.empty()) break;
            }
    
            if (text.empty()) continue;
    
            std::istringstream iss(text);
            std::string mnem;
            iss >> mnem;
            mnem = upper(mnem);
    
            std::string rest;
            std::getline(iss, rest);
            auto operands = splitOperands(rest);
    
            if (mnem == ".ORG") {
                if (operands.size() != 1) throw std::runtime_error(".ORG needs 1 operand");
                auto newPc = parseValue(operands[0], labels);
                if (newPc < out.size()) throw std::runtime_error(".ORG cannot move backwards");
                out.resize(newPc, 0x00);
                pc = newPc;
            } else if (mnem == ".DB") {
                for (const auto& op : operands) {
                    auto v = parseValue(op, labels);
                    if (v > 0xFF) throw std::runtime_error(".DB value out of range");
                    out.push_back(static_cast<uint8_t>(v));
                    ++pc;
                }
            } else {
                emit(mnem, operands, labels, out);
            }
        }
    
        return out;
    }
 
}