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
        "indexed": "AT_INDEXED|AT_INDIRECT",
        "condition": "AT_CONDITION",
        "port": "AT_PORT"
    }

    def __init__(self, name, data):
        self._name = name
        self._data = data

    def _get_arg_parse_mask(self, num):
        """
        Return the appropriate mask to parse_args() for the num-th argument.
        """
        types = set()
        optional = False
        for case in self._data["cases"]:
            if num < len(case["type"]):
                types.add(self.ARG_TYPES[case["type"][num]])
            else:
                optional = True

        if not types:
            return "AT_NONE"
        if optional:
            types.add("AT_OPTIONAL")
        return "|".join(types)

    def _handle_return(self, arg, indent=1):
        """
        Return code to handle an instruction return statement.
        """
        tabs = TAB * indent
        if arg == "error":
            return tabs + "INST_ERROR(ARG_SYNTAX)"
        else:
            data = ", ".join("0x%02X" % byte for byte in arg)
            return tabs + "INST_RETURN({0}, {1})".format(len(arg), data)

    def _handle_case(self, case):
        """
        TODO
        """
        return [TAB + "// " + str(case)]

    def render(self):
        """
        Convert data for an individual instruction into a C parse function.
        """
        lines = []

        if self._data["args"]:
            lines.append("{tab}INST_TAKES_ARGS(\n{tab2}{0}, \n{tab2}{1}, "
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

    # with open(DEST, "w") as fp:
    #     fp.write(result.encode(ENCODING))
    print(result)  # TODO: remove me!

if __name__ == "__main__":
    main()
