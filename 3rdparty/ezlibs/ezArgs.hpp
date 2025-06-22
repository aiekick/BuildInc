#pragma once

/*
MIT License

Copyright (c) 2014-2024 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// ezArgs is part of the ezLibs project : https://github.com/aiekick/ezLibs.git

#include "ezOS.hpp"

#include <vector>
#include <string>
#include <cstdio>   // FILENAME_MAX
#include <cstdint>  // int32_t
#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "ezStr.hpp"

#include "ezLog.hpp"

namespace ez {

class Args {
private:
    class Argument {
        friend class Args;

    private:
        std::vector<std::string> m_base_args;  // base args
        std::set<std::string> m_full_args;  // full args
        char one_char_arg = 0;
        std::string m_help_text;
        std::string m_help_var_name;
        std::string m_type;
        bool m_required = false;
        char m_delimiter = 0;  // delimiter used for arguments : toto a, toto=a, toto:a, etc...
        bool m_is_present = false;  // found during parsing
        bool m_has_value = false;  // found during parsing
        std::string m_value;  // value

    public:
        Argument() = default;

        Argument &help(const std::string &vHelp, const std::string &vVarName = {}) {
            m_help_text = vHelp;
            m_help_var_name = vVarName;
            return *this;
        }

        Argument &def(const std::string &vDefValue) {
            m_value = vDefValue;
            return *this;
        }

        Argument &type(const std::string &vType) {
            m_type = vType;
            return *this;
        }

        Argument &delimiter(char vDelimiter) {
            m_delimiter = vDelimiter;
            return *this;
        }

        Argument &required(bool vValue) {
            m_required = vValue;
            return *this;
        }

    private:
        typedef std::pair<std::string, std::string> HelpCnt;

        HelpCnt m_getHelp(bool vPositional, size_t &vInOutFirstColSize) const {
            HelpCnt res;
            std::stringstream ss;
            if (vPositional) {
                std::string token = m_help_var_name;
                if (token.empty()) {
                    token = *(m_base_args.begin());
                }
                ss << "  " << token;
            } else {
                size_t idx = 0;
                ss << "  ";
                for (const auto &arg : m_base_args) {
                    if (idx++ > 0) {
                        ss << ", ";
                    }
                    ss << arg;
                }
                if (!m_help_var_name.empty()) {
                    ss << " " << m_help_var_name;
                }
            }
            auto ret = ss.str();
            if (vInOutFirstColSize < ret.size()) {
                vInOutFirstColSize = ret.size();
            }
            return std::make_pair(ret, m_help_text);
        }
    };

private:
    std::string m_AppName;
    std::string m_HelpHeader;
    std::string m_HelpFooter;
    std::string m_HelpDescription;
    Argument m_HelpArgument;
    std::vector<Argument> m_Positionals;
    std::vector<Argument> m_Optionals;

public:
    Args() = default;
    virtual ~Args() = default;
    Args(const std::string &vName) : m_AppName(vName) {
        if (vName.empty()) {
            throw std::runtime_error("Name cant be empty");
        }
        m_addOptional(m_HelpArgument, "-h/--help").help("Show the usage");
    }

    Args &addHeader(const std::string &vHeader) {
        m_HelpHeader = vHeader;
        return *this;
    }

    Args &addFooter(const std::string &vFooter) {
        m_HelpFooter = vFooter;
        return *this;
    }

    Args &addDescription(const std::string &vDescription) {
        m_HelpDescription = vDescription;
        return *this;
    }

    Argument &addArgument(const std::string &vKey) {
        if (vKey.empty()) {
            throw std::runtime_error("argument cant be empty");
        }
        Argument res;
        res.m_required = true;
        res.m_base_args = ez::str::splitStringToVector(vKey, '/');
        for (const auto &a : res.m_base_args) {
            res.m_full_args.emplace(a);
        }
        m_Positionals.push_back(res);
        return m_Positionals.back();
    }

    Argument &addOptional(const std::string &vKey) {
        if (vKey.empty()) {
            throw std::runtime_error("optinnal cant be empty");
        }
        Argument res;
        m_addOptional(res, vKey);
        m_Optionals.push_back(res);
        return m_Optionals.back();
    }

    // is token present
    bool isPresent(const std::string &vKey) {
        auto *ptr = m_getArgumentPtr(vKey, true);
        if (ptr != nullptr) {
            return ptr->m_is_present;
        }
        return false;
    }

    // is token have value
    bool hasValue(const std::string &vKey) {
        auto *ptr = m_getArgumentPtr(vKey, true);
        if (ptr != nullptr) {
            return ptr->m_has_value;
        }
        return false;
    }

    template <typename T>
    T getValue(const std::string &vKey, bool vNoExcept = false) const {
        auto *ptr = m_getArgumentPtr(vKey, vNoExcept);
        if (ptr != nullptr && !ptr->m_value.empty()) {
            return m_convertString<T>(ptr->m_value);
        }
        return {};
    }

    std::string getHelp(  //
        const std::string &vPositionalHeader = "Positionnal arguments",
        const std::string &vOptionalHeader = "Optional arguments") const {
        std::string token;
        std::stringstream ss;
        if (!m_HelpHeader.empty()) {
            ss << m_HelpHeader << std::endl << std::endl;
        }
        ss << m_getCmdLineHelp();
        ss << std::endl;
        if (!m_HelpDescription.empty()) {
            ss << std::endl << " " << m_HelpDescription << std::endl;
        }
        ss << m_getHelpDetails(vPositionalHeader, vOptionalHeader);
        if (!m_HelpFooter.empty()) {
            ss << std::endl << m_HelpFooter << std::endl;
        }
        return ss.str();
    }

    void printHelp() const { std::cout << getHelp() << std::endl; }

    bool parse(const int32_t vArgc, char **vArgv, const int32_t vStartIdx = 1U) {
        size_t positional_idx = 0;
        for (int32_t idx = vStartIdx; idx < vArgc; ++idx) {
            std::string arg = m_trim(vArgv[idx]);

            // print help
            if (m_HelpArgument.m_full_args.find(arg) != m_HelpArgument.m_full_args.end()) {
                printHelp();
                return true;
            }

            // get args values
            std::string token = arg;
            std::string value;
            bool is_optional = false;
            bool check_for_value = false;
            for (auto &arg_ref : m_Optionals) {
                check_for_value = false;
                if (arg_ref.m_delimiter != 0) {
                    if (arg_ref.m_delimiter != ' ') {
                        if (token.find(arg_ref.m_delimiter) != std::string::npos) {
                            auto arr = ez::str::splitStringToVector(token, arg_ref.m_delimiter);
                            if (arr.size() == 2) {
                                token = arr.at(0);
                                value = arr.at(1);
                            } else {
                                if (arr.size() < 2) {
                                    throw std::runtime_error("bad parsing of key \"" + token + "\". no value");
                                } else if (arr.size() > 2) {
                                    throw std::runtime_error("bad parsing of key \"" + token + "\". more than one value");
                                }
                            }
                        }
                    }
                }
                if (arg_ref.one_char_arg != 0) {
                    auto p = token.find(arg_ref.one_char_arg);
                    if (p != std::string::npos) {
                        arg_ref.m_is_present = true;
                        is_optional = true;
                        if (p == (token.size() - 1)) {
                            check_for_value = true;
                        }
                    }
                } else if (arg_ref.m_full_args.find(token) != arg_ref.m_full_args.end()) {
                    arg_ref.m_is_present = true;
                    is_optional = true;
                    check_for_value = true;
                }
                if (check_for_value) {
                    if (arg_ref.m_delimiter == ' ') {
                        if ((idx + 1) < vArgc) {
                            auto *existingArg = m_getArgumentPtr(vArgv[idx + 1], true);
                            if (!existingArg) {
                                arg_ref.m_value = vArgv[++idx];
                                arg_ref.m_has_value = true;
                            }
                        }
                    } else if (arg_ref.m_delimiter != 0) {
                        auto *existingArg = m_getArgumentPtr(value, true);
                        if (!existingArg) {
                            arg_ref.m_value = value;
                            arg_ref.m_has_value = true;
                        }
                    }
                }
            }

            // positionals
            if (!is_optional) {
                if (positional_idx < m_Positionals.size()) {
                    auto &positional = m_Positionals.at(positional_idx);
                    positional.m_value = arg;
                    positional.m_has_value = true;
                    ++positional_idx;
                }
            }
        }

        if (vArgc == 1 &&  // only the app path default argument
            !m_Positionals.empty()) {  // some positionnal are wanted
            printHelp();
        }

        return true;
    }

private:
    const Argument *m_getArgumentPtr(const std::string &vKey, bool vNoExcept = false) const {
        const Argument *ret = nullptr;
        for (const auto &arg : m_Positionals) {
            if (arg.m_full_args.find(vKey) != arg.m_full_args.end()) {
                ret = &arg;
            }
        }
        if (ret == nullptr) {
            for (const auto &arg : m_Optionals) {
                if (arg.m_full_args.find(vKey) != arg.m_full_args.end()) {
                    ret = &arg;
                }
            }
        }
        if (ret == nullptr && vNoExcept == false) {
            throw std::runtime_error("Argument not found");
        }
        return ret;
    }

    Argument &m_addOptional(Argument &vInOutArgument, const std::string &vKey) {
        if (vKey.empty()) {
            throw std::runtime_error("optinnal cant be empty");
        }
        vInOutArgument.m_base_args = ez::str::splitStringToVector(vKey, '/');
        for (const auto &a : vInOutArgument.m_base_args) {
            vInOutArgument.m_full_args.emplace(a);
        }
        for (const auto &arg : vInOutArgument.m_base_args) {
            // tofix : may fail if arg is --toto-titi.
            // we will get titi but we want toto-titi
            vInOutArgument.m_full_args.emplace(m_trim(arg));
        }
        vInOutArgument.one_char_arg = (vKey.size() == 1U) ? vKey[0] : 0;
        return vInOutArgument;
    }

    std::string m_getCmdLineHelp() const {
        std::stringstream ss;
        ss << " Usage : " << m_AppName;
        for (const auto &arg : m_Optionals) {
            ss << " [";
            size_t idx = 0;
            for (const auto &o : arg.m_base_args) {
                if (idx++ > 0) {
                    ss << ':';
                }
                ss << o;
            }
            if (!arg.m_help_var_name.empty()) {
                ss << " " << arg.m_help_var_name;
            }
            ss << "]";
        }
        for (const auto &arg : m_Positionals) {
            std::string token = arg.m_help_var_name;
            if (token.empty()) {
                token = *(arg.m_base_args.begin());
            }
            ss << " " << token;
        }
        return ss.str();
    }

    std::string m_getHelpDetails(  //
        const std::string &vPositionalHeader,
        const std::string &vOptionalHeader) const {
        // collect infos with padding
        size_t first_col_size = 0U;
        std::vector<Argument::HelpCnt> cnt_pos;
        for (const auto &arg : m_Positionals) {
            cnt_pos.push_back(arg.m_getHelp(true, first_col_size));
        }
        std::vector<Argument::HelpCnt> cnt_opt;
        for (const auto &opt : m_Optionals) {
            cnt_opt.push_back(opt.m_getHelp(false, first_col_size));
        }
        // display
        first_col_size += 4U;
        std::stringstream ss;
        if (!cnt_pos.empty()) {
            ss << std::endl << " " << vPositionalHeader << " : " << std::endl;
            for (const auto &it : cnt_pos) {
                ss << it.first << std::string(first_col_size - it.first.size(), ' ') << it.second << std::endl;
            }
        }
        if (!cnt_opt.empty()) {
            ss << std::endl << " " << vOptionalHeader << " : " << std::endl;
            for (const auto &it : cnt_opt) {
                ss << it.first << std::string(first_col_size - it.first.size(), ' ') << it.second << std::endl;
            }
        }
        return ss.str();
    }

    // remove the first '-' of a token
    std::string m_trim(const std::string &vToken) {
        auto short_last_minus = vToken.find_first_not_of("-");
        if (short_last_minus != std::string::npos) {
            return vToken.substr(short_last_minus);
        }
        return {};
    }

    template <typename T>
    T m_convertString(const std::string &str) const {
        std::istringstream iss(str);
        T value;
        if (!(iss >> value)) {
            throw std::runtime_error("Conversion failed");
        }
        return value;
    }
};

template <>
inline bool Args::m_convertString<bool>(const std::string &str) const {
    if (str == "true" || str == "1") {
        return true;
    } else if (str == "false" || str == "0") {
        return false;
    }
    throw std::runtime_error("Invalid boolean string");
}
}  // namespace ez
