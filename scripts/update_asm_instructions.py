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

def build_inst(name, data):
    """
    Convert data for an individual instruction into a C parse function.
    """
    # TODO
    return "INST_FUNC({0})\n{{\n}}".format(name)

def build_inst_block(data):
    """
    Return the instruction parser block, given instruction data.
    """
    return "\n\n".join(build_inst(k, v) for k, v in sorted(data.items()))

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
