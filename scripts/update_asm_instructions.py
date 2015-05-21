#!/usr/bin/env python
# -*- coding: utf-8  -*-

# Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
# Released under the terms of the MIT License. See LICENSE for details.

"""
This script generates 'src/assembler/instructions.inc.c' from
'src/assembler/instructions.yml'. It should be run automatically by make
when the latter is modified, but can also be run manually.
"""

from __future__ import print_function

from itertools import product
import re
import time

SOURCE = "src/assembler/instructions.yml"
DEST = "src/assembler/instructions.inc.c"

ENCODING = "utf8"
TAB = " " * 4

try:
    import yaml
except ImportError:
    print("Error: PyYAML is required (https://pypi.python.org/pypi/PyYAML)\n"
          "If you don't want to rebuild {0}, do:\n`make -t {0}`".format(DEST))
    exit(1)

re_date = re.compile(r"^(\s*@AUTOGEN_DATE\s*)(.*?)$", re.M)
re_inst = re.compile(
    r"(/\* @AUTOGEN_INST_BLOCK_START \*/\n*)(.*?)"
    r"(\n*/\* @AUTOGEN_INST_BLOCK_END \*/)", re.S)
re_lookup = re.compile(
    r"(/\* @AUTOGEN_LOOKUP_BLOCK_START \*/\n*)(.*?)"
    r"(\n*/\* @AUTOGEN_LOOKUP_BLOCK_END \*/)", re.S)

class Instruction(object):
    """
    Represent a single ASM instruction mnemonic.
    """
    ARG_TYPES = {
        "register": "AT_REGISTER",
        "immediate": "AT_IMMEDIATE",
        "indirect": "AT_INDIRECT",
        "indexed": "AT_INDEXED",
        "condition": "AT_CONDITION",
        "port": "AT_PORT"
    }
    PSEUDO_TYPES = {
        "indirect_hl_or_indexed": ["AT_INDIRECT", "AT_INDEXED"]
    }

    def __init__(self, name, data):
        self._name = name
        self._data = data
        self._has_optional_args = False

    def _get_arg_parse_mask(self, num):
        """
        Return the appropriate mask to parse_args() for the num-th argument.
        """
        types = set()
        optional = False
        for case in self._data["cases"]:
            if num < len(case["type"]):
                atype = case["type"][num]
                if atype in self.ARG_TYPES:
                    types.add(self.ARG_TYPES[atype])
                else:
                    types.update(self.PSEUDO_TYPES[atype])
            else:
                optional = True

        if not types:
            return "AT_NONE"
        if optional:
            types.add("AT_OPTIONAL")
            self._has_optional_args = True
        return "|".join(sorted(types))

    def _handle_return(self, ret, indent=1):
        """
        Return code to handle an instruction return statement.
        """
        data = ", ".join("0x%02X" % byte if isinstance(byte, int) else byte
                         for byte in ret)
        return TAB * indent + "INST_RETURN({0}, {1})".format(len(ret), data)

    def _build_case_type_check(self, args):
        """
        Return the test part of an if statement for an instruction case.
        """
        conds = ["INST_TYPE({0}) == {1}".format(i, self.ARG_TYPES[cond])
                 for i, cond in enumerate(args)]
        check = " && ".join(conds)

        if self._has_optional_args:
            return "INST_NARGS == {0} && ".format(len(args)) + check
        return check

    def _build_register_check(self, num, cond):
        """
        Return an expression to check for a particular register value.
        """
        return "INST_REG({0}) == REG_{1}".format(num, cond.upper())

    def _build_immediate_check(self, num, cond):
        """
        Return an expression to check for a particular immediate value.
        """
        if "." in cond:
            itype, value = cond.split(".", 1)
            try:
                value = int(value)
            except ValueError:
                value = int(value, 16)
            vtype = "sval" if itype.upper() in ["S8", "REL"] else "uval"

            test1 = "INST_IMM({0}).mask & IMM_{1}".format(num, itype.upper())
            if (itype.upper() == "U16"):
                test1 += " && !INST_IMM({0}).is_label".format(num)
            test2 = "INST_IMM({0}).{1} == {2}".format(num, vtype, value)
            return "({0} && {1})".format(test1, test2)

        return "INST_IMM({0}).mask & IMM_{1}".format(num, cond.upper())

    def _build_indirect_check(self, num, cond):
        """
        Return an expression to check for a particular indirect value.
        """
        if cond.startswith("reg."):
            test1 = "INST_INDIRECT({0}).type == AT_REGISTER".format(num)
            test2 = "INST_INDIRECT({0}).addr.reg == REG_{1}".format(
                num, cond[len("reg."):].upper())
            return "({0} && {1})".format(test1, test2)

        if cond == "imm" or cond == "immediate":
            return "INST_INDIRECT({0}).type == AT_IMMEDIATE".format(num)

        err = "Unknown condition for indirect argument: {0}"
        return RuntimeError(err.format(cond))

    def _build_indexed_check(self, num, cond):
        """
        Return an expression to check for a particular indexed value.
        """
        raise RuntimeError("The indexed arg type does not support conditions")

    def _build_condition_check(self, num, cond):
        """
        Return an expression to check for a particular condition value.
        """
        return "INST_COND({0}) == COND_{1}".format(num, cond.upper())

    def _build_port_check(self, num, cond):
        """
        Return an expression to check for a particular port value.
        """
        if cond == "reg" or cond == "reg.c":
            return "INST_PORT({0}).type == AT_REGISTER".format(num)
        if cond == "imm" or cond == "immediate":
            return "INST_PORT({0}).type == AT_IMMEDIATE".format(num)

        err = "Unknown condition for port argument: {0}"
        return RuntimeError(err.format(cond))

    _SUBCASE_LOOKUP_TABLE = {
        "register": _build_register_check,
        "immediate": _build_immediate_check,
        "indirect": _build_indirect_check,
        "indexed": _build_indexed_check,
        "condition": _build_condition_check,
        "port": _build_port_check
    }

    def _build_subcase_check(self, types, conds):
        """
        Return the test part of an if statement for an instruction subcase.
        """
        conds = [self._SUBCASE_LOOKUP_TABLE[types[i]](self, i, cond)
                 for i, cond in enumerate(conds) if cond != "_"]
        return " && ".join(conds)

    def _iter_permutations(self, types, conds):
        """
        Iterate over all permutations of the given subcase conditions.
        """
        def split(typ, cond):
            if "|" in cond:
                splits = [split(typ, c) for c in cond.split("|")]
                merged = [choice for s in splits for choice in s]
                if len(merged) != len(set(merged)):
                    msg = "Repeated conditions for {0}: {1}"
                    raise RuntimeError(msg.format(typ, cond))
                return merged
            if typ == "register":
                if cond == "i":
                    return ["ix", "iy"]
                if cond == "ih":
                    return ["ixh", "iyh"]
                if cond == "il":
                    return ["ixl", "iyl"]
            return [cond]

        splits = [split(typ, cond) for typ, cond in zip(types, conds)]
        num = max(len(cond) for cond in splits)

        if any(1 < len(cond) < num for cond in splits):
            msg = "Invalid condition permutations: {0}"
            raise RuntimeError(msg.format(conds))

        choices = [cond * num if len(cond) == 1 else cond for cond in splits]
        return zip(*choices)

    def _adapt_return(self, types, conds, ret):
        """
        Return a modified byte list to accomodate for prefixes and immediates.
        """
        ret = ret[:]
        for i, byte in enumerate(ret):
            if not isinstance(byte, int):
                if byte == "u8":
                    try:
                        index = types.index("immediate")
                        imm = "INST_IMM({0})".format(index)
                    except ValueError:
                        index = types.index("port")
                        imm = "INST_PORT({0}).port.imm".format(index)
                    ret[i] = imm + ".uval"

                elif byte == "u16":
                    if i < len(ret) - 1:
                        raise RuntimeError("U16 return byte must be last")
                    try:
                        index = types.index("immediate")
                        imm = "INST_IMM({0})".format(index)
                    except ValueError:
                        indir = types.index("indirect")
                        if not conds[indir].startswith("imm"):
                            msg = "Passing non-immediate indirect as immediate"
                            raise RuntimeError(msg)
                        imm = "INST_INDIRECT({0}).addr.imm".format(indir)
                    ret[i] = "INST_IMM_U16_B1({0})".format(imm)
                    ret.append("INST_IMM_U16_B2({0})".format(imm))
                    break

                elif byte == "rel":
                    index = types.index("immediate")
                    ret[i] = "INST_IMM({0}).sval - 2".format(index)

                elif byte.startswith("bit(") and byte.endswith(")"):
                    index = types.index("immediate")
                    off = byte[4:-1]
                    ret[i] = "{0} + 8 * INST_IMM({1}).uval".format(off, index)

                else:
                    msg = "Unsupported return byte: {0}"
                    raise RuntimeError(msg.format(byte))

        for i, cond in enumerate(conds):
            if types[i] == "register" and cond[0] == "i":
                prefix = "INST_I{0}_PREFIX".format(cond[1].upper())
                if ret[0] != prefix:
                    ret.insert(0, prefix)
            elif types[i] == "indexed":
                ret.insert(0, "INST_INDEX_PREFIX({0})".format(i))
                ret.insert(2, "INST_INDEX({0}).offset".format(i))

        return ret

    def _handle_null_case(self, case):
        """
        Return code to handle an instruction case that takes no arguments.
        """
        return [
            TAB + "if (INST_NARGS == 0) {",
            self._handle_return(case["return"], 2),
            TAB + "}"
        ]

    def _handle_pseudo_case(self, pseudo, case):
        """
        Return code to handle an instruction pseudo-case.

        Pseudo-cases are cases that have pseudo-types as arguments. This means
        they are expanded to cover multiple "real" argument types.
        """
        index = case["type"].index(pseudo)

        if pseudo == "indirect_hl_or_indexed":
            case["type"][index] = "indexed"
            indexed = self._handle_case(case)

            case["type"][index] = "indirect"
            indirect = self._handle_case(case)
            base_cond = self._build_case_type_check(case["type"])
            hl_reg = TAB * 3 + self._build_indirect_check(index, "reg.hl")
            indirect[0] = TAB + "if ({0} &&\n{1}) {{".format(base_cond, hl_reg)

            return indirect + indexed

        raise RuntimeError("Unknown pseudo-type: {0}".format(pseudo))

    def _handle_case(self, case):
        """
        Return code to handle an instruction case.
        """
        ctype = case["type"]
        if not ctype:
            return self._handle_null_case(case)

        for pseudo in self.PSEUDO_TYPES:
            if pseudo in ctype:
                return self._handle_pseudo_case(pseudo, case)

        lines = []
        cond = self._build_case_type_check(ctype)
        lines.append(TAB + "if ({0}) {{".format(cond))

        subcases = [(perm, sub["return"]) for sub in case["cases"]
                    for perm in self._iter_permutations(ctype, sub["cond"])]
        for cond, ret in subcases:
            check = self._build_subcase_check(ctype, cond)
            ret = self._adapt_return(ctype, cond, ret)
            if check:
                lines.append(TAB * 2 + "if ({0})".format(check))
                lines.append(self._handle_return(ret, 3))
            else:
                lines.append(self._handle_return(ret, 2))
                break  # Unconditional subcase
        else:
            lines.append(TAB * 2 + "INST_ERROR(ARG_VALUE)")

        lines.append(TAB + "}")
        return lines

    def render(self):
        """
        Convert data for an individual instruction into a C parse function.
        """
        lines = []

        if self._data["args"]:
            lines.append("{tab}INST_TAKES_ARGS(\n{tab2}{0},\n{tab2}{1},"
                         "\n{tab2}{2}\n{tab})".format(
                self._get_arg_parse_mask(0), self._get_arg_parse_mask(1),
                self._get_arg_parse_mask(2), tab=TAB, tab2=TAB * 2))
        else:
            lines.append(TAB + "INST_TAKES_NO_ARGS")

        if "return" in self._data:
            lines.append(self._handle_return(self._data["return"]))
        elif "cases" in self._data:
            for case in self._data["cases"]:
                lines.extend(self._handle_case(case))
            lines.append(TAB + "INST_ERROR(ARG_TYPE)")
        else:
            msg = "Missing return or case block for {0} instruction"
            raise RuntimeError(msg.format(self._name))

        contents = "\n".join(lines)
        return "INST_FUNC({0})\n{{\n{1}\n}}".format(self._name, contents)


def build_inst_block(data):
    """
    Return the instruction parser block, given instruction data.
    """
    return "\n\n".join(
        Instruction(k, v).render() for k, v in sorted(data.items()))

def build_lookup_block(data):
    """
    Return the instruction lookup block, given instruction data.
    """
    macro = TAB + "HANDLE({0})"
    return "\n".join(macro.format(inst) for inst in sorted(data.keys()))

def process(template, data):
    """
    Return C code generated from a source template and instruction data.
    """
    inst_block = build_inst_block(data)
    lookup_block = build_lookup_block(data)
    date = time.asctime(time.gmtime())

    result = re_date.sub(r"\1{0} UTC".format(date), template)
    result = re_inst.sub(r"\1{0}\3".format(inst_block), result)
    result = re_lookup.sub(r"\1{0}\3".format(lookup_block), result)
    return result

def main():
    """
    Main script entry point.
    """
    with open(SOURCE, "r") as fp:
        text = fp.read().decode(ENCODING)
    with open(DEST, "r") as fp:
        template = fp.read().decode(ENCODING)

    data = yaml.load(text)
    result = process(template, data)

    with open(DEST, "w") as fp:
        fp.write(result.encode(ENCODING))

if __name__ == "__main__":
    main()
