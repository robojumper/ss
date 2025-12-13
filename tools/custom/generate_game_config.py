"""
Skyward Sword has 8 versions, all without official symbols.
We work on NTSC 1.0 given that most previous reversing has
been done on it, and we will keep re-splitting and re-naming
things. Since other versions are *very* similar, we derive their
config on-the-fly from the NTSC 1.0 config.

Note that this is a very simple script with little error checking.
derive.txt files must have correctly sorted commands.

This script is not useful for creating derive files. You should have a good
understanding of the kind of changes that were made between versions before
attempting to write a derive.txt. I used scripts found at
https://github.com/robojumper/wii-code-tools/tree/ss-tools to create and
verify an address map, then ported the address map to derive files.

This script is unapologetically written for Skyward Sword specifically since
the Skyward Sword version differences are miniscule. There are no guarantees
that it works for any other use case.

Goals:

* Continuously generate other versions' config based on SOUE01 config with no effort
  even as symbols and files are renamed, and little effort when files are re-split.
* Easily extend SOUE01 matches to the other versions.

Non-goals:

* Generate a 100% accurate derived symbol map. It is acceptable for those symbol maps
  to be fixed up by dtk after generation (e.g. letting dtk insert additional anonymous float symbols or string labels)

Design decisions:

* derive.txt files should be largely independent of the way splits.txt and symbols.txt are organized.
  They mostly mention memory addresses instead of file names and symbols. Still, some versioning changes
  have different effects on splits.txt and symbols.txt
* derive.txt files are self-contained and do not rely on additional info like DOL or REL metadata/header info.
* Especially in .text sections, offset changes should be preceded by a resize, delete or insert command,
  since those are the cause of offset changes.
* Non-flat address space: Memory is organized in a Module > Section > Addr/Offset hierarchy, similar to
  dtk addresses (main DOL addresses *are* flat but section names are required for consistency).

Call with:
    python ./tools/custom/generate_game_config.py [SOUE01|SOUE01_R1|SOUE01_R2|SOUP01|SOUP01_R1|SOUP01_R2|SOUJ01|SOUK01]
"""

import bisect
from dataclasses import dataclass
import os
import pathlib
import re
import shutil
import sys
from typing import Dict, List, TextIO, Tuple

from splits_txt import parse_splits_file, parse_splits_section, serialize_splits_file

@dataclass
class SymbolCommand:
    # 'insert' | 'offset' | 'resize' | 'split_offset'  | 'sym_offset' | 'delete'
    cmd: str
    section: str
    addr: int
    payload: str

    def get_insert_line(self) -> str:
        return self.payload

    def get_offset(self) -> int:
        return int(self.payload, 16)

    def get_resize(self) -> int:
        return int(self.payload, 16)

@dataclass
class DeriveModule:
    # MAIN or rel name
    module: str
    commands_by_section: Dict[str, List[SymbolCommand]]
    deletions_by_section: Dict[str, set]
    split_additions: Dict[str, List[str]]

@dataclass
class DeriveFile:
    base: str
    modules: Dict[str, DeriveModule]


def parse_derive_file(text: str) -> DeriveFile:
    base = None
    modules = {}
    curr_module = None
    seen_modules = set()

    for line in text.split('\n'):
        line = line.strip()
        if line == "" or line.startswith("#"):
            continue

        if line.startswith("base:"):
            base = line.split(' ')[1].strip()
            continue

        if line.startswith("module:"):
            curr_module_name = line.split(' ')[1].strip()
            if curr_module_name in seen_modules:
                raise ValueError(f"module {curr_module_name} already seen")
            seen_modules.add(curr_module_name)
            curr_module = DeriveModule(curr_module_name, {}, {}, {})
            modules[curr_module_name] = curr_module
            continue

        if (
            line.startswith("offset ")
            or line.startswith("resize ")
            or line.startswith("insert ")
            or line.startswith("split_offset ")
            or line.startswith("sym_offset ")
        ):
            if not curr_module:
                raise ValueError("no module")
            cmd, sec_addr, payload = line.split(" ", maxsplit=2)
            section, addr = sec_addr.split(":")
            if section not in curr_module.commands_by_section:
                curr_module.commands_by_section[section] = []

            curr_module.commands_by_section[section].append(SymbolCommand(cmd, section, int(addr, 16), payload))
            continue

        if line.startswith("split_addition "):
            cmd, file, addition = line.split(" ", maxsplit=2)
            if file not in curr_module.split_additions:
                curr_module.split_additions[file] = []

            curr_module.split_additions[file].append(addition)
            continue

        if line.startswith("delete "):
            if not curr_module:
                raise ValueError("no module")
            cmd, sec_addr = line.split(" ", maxsplit=1)
            section, addr = sec_addr.split(":")
            if section not in curr_module.deletions_by_section:
                curr_module.deletions_by_section[section] = set()

            curr_module.deletions_by_section[section].add(int(addr, 16))
            continue

        raise ValueError(f"unparsed line {line}")

    return DeriveFile(base, modules)

_DTK_SYM_RE = re.compile("([_\\.A-Za-z0-9]+):0x([0-9A-Fa-f]+); // (.*)")

def advance_commands(commands: List[SymbolCommand], new_addr: int, last_idx: int) -> Tuple[int, List[SymbolCommand]]:
    ret = []
    idx = last_idx
    while idx < len(commands) and new_addr >= commands[idx].addr:
        ret.append(commands[idx])
        idx += 1

    return idx, ret


def dtkhex(num: int) -> str:
    ret = hex(num).upper()
    return "0x" + ret[2:]


def port_symbols_txt(input: TextIO, output: TextIO, module: DeriveModule):
    active_section_name = None
    last_offset = 0

    cmd_idx = 0

    for line in input:
        name, rest = line.split(" = ")
        name = name.strip()
        rest = rest.strip()
        match = _DTK_SYM_RE.match(rest)
        section = match.group(1)
        addr_ = match.group(2)
        attrs_ = match.group(3)

        attrs = {}
        for pair in attrs_.split(' '):
            parts = pair.split(':')
            if len(parts) == 1:
                attrs[parts[0]] = None
            else:
                attrs[parts[0]] = parts[1]

        size_ = attrs.get('size', None)
        size = None
        if size_:
            size = int(size_[2:], 16)
        addr = int(addr_, 16)

        if section != active_section_name:
            active_section_name = section
            last_offset = 0
            cmd_idx = 0

        cmd_idx, commands = advance_commands(module.commands_by_section.get(active_section_name, []), addr, cmd_idx)

        for cmd in commands:
            if cmd.cmd == 'offset' or cmd.cmd == 'sym_offset':
                last_offset = cmd.get_offset()
            elif cmd.cmd == 'resize':
                size = cmd.get_resize()
            elif cmd.cmd == 'insert':
                output.write(cmd.get_insert_line() + "\n")
            elif cmd.cmd == 'split_offset':
                continue

        if addr in module.deletions_by_section.get(section, []):
            continue

        if name.startswith("...") and attrs.get("type", None) == "label":
            # skip `...` labels since these are not necessary and point to the
            # start of a file, which is not necessarily the correct address
            # when porting symbols.
            continue

        output.write(f"{name} = {section}:{dtkhex(addr + last_offset)}; //")
        for key, value in attrs.items():
            if key == "size":
                value = f"{dtkhex(size)}"
            if value is not None:
                output.write(f" {key}:{value}")
            else:
                output.write(f" {key}")
        output.write("\n")


def port_splits_txt(input: TextIO, output: TextIO, module: DeriveModule):
    splits_file = parse_splits_file(input.read())

    @dataclass
    class SplitPoint:
        src_addr: int
        offset: int

    split_points_by_section: Dict[str, List[SplitPoint]] = {}

    def map_addr(section: str, src_addr: int):
        if section not in split_points_by_section or len(split_points_by_section[section]) == 0:
            return src_addr
        idx = bisect.bisect_right(split_points_by_section[section], src_addr, key=lambda p: p.src_addr)
        return src_addr + split_points_by_section[section][idx - 1].offset

    for section, commands in module.commands_by_section.items():
        split_points_by_section[section] = [SplitPoint(0, 0)]
        for cmd in commands:
            if cmd.cmd == 'offset' or cmd.cmd == 'split_offset':
                split_points_by_section[section].append(SplitPoint(cmd.addr, cmd.get_offset()))

    for file in splits_file.files:
        for section in file.sections:
            section.start = map_addr(section.name, section.start)
            section.end = map_addr(section.name, section.end - 1) + 1

        for addition in module.split_additions.get(file.path, []):
            file.sections.append(parse_splits_section(addition))

    output.write(serialize_splits_file(splits_file))


def main():
    version = sys.argv[1]
    derive_text = pathlib.Path(f"config/{version}/derive.txt").read_text()
    file = parse_derive_file(derive_text)

    with open(f"config/{file.base}/symbols.txt", 'r', encoding='utf-8') as f_in:
        with open(f"config/{version}/symbols.txt", 'w', encoding='utf-8') as f_out:
            port_symbols_txt(f_in, f_out, file.modules["MAIN"])

    with open(f"config/{file.base}/splits.txt", 'r', encoding='utf-8') as f_in:
        with open(f"config/{version}/splits.txt", 'w', encoding='utf-8') as f_out:
            port_splits_txt(f_in, f_out, file.modules["MAIN"])

    for name in os.listdir('config/SOUE01/rels'):
        module = file.modules.get(name, None)
        if module:
            os.makedirs(f"config/{version}/rels/{name}/", exist_ok=True)
            with open(f"config/{file.base}/rels/{name}/symbols.txt", 'r', encoding='utf-8') as f_in:
                with open(f"config/{version}/rels/{name}/symbols.txt", 'w', encoding='utf-8') as f_out:
                    port_symbols_txt(f_in, f_out, module)

            with open(f"config/{file.base}/rels/{name}/splits.txt", 'r', encoding='utf-8') as f_in:
                with open(f"config/{version}/rels/{name}/splits.txt", 'w', encoding='utf-8') as f_out:
                    port_splits_txt(f_in, f_out, module)
        else:
            os.makedirs(f"config/{version}/rels/{name}/", exist_ok=True)
            shutil.copy(f"config/{file.base}/rels/{name}/symbols.txt", f"config/{version}/rels/{name}/symbols.txt")
            shutil.copy(f"config/{file.base}/rels/{name}/splits.txt", f"config/{version}/rels/{name}/splits.txt")


if __name__ == "__main__":
    main()
