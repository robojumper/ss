from dataclasses import dataclass
import re
from typing import List

@dataclass
class SplitSection:
    name: str
    start: int
    end: int
    attrs: str


@dataclass
class SplitFile:
    path: str
    sections: List[SplitSection]


@dataclass
class SplitsTxt:
    module_block: str
    files: List[SplitFile]


_ADDR_LINE_RE = re.compile(r"\t?([\.a-z0-9]+)\s+start:0x([0-9A-Fa-f]+) end:0x([0-9A-Fa-f]+)( .*)?")

def parse_splits_section(line: str) -> SplitSection:
    match = _ADDR_LINE_RE.match(line)
    if not match:
        print(line)
    name_ = match.group(1)
    start_ = match.group(2)
    end_ = match.group(3)
    attrs_ = match.group(4)

    return SplitSection(
        name_,
        int(start_, 16),
        int(end_, 16),
        attrs_[1:] if attrs_ else ""
    )

def _parse_split_file(file: str) -> SplitFile:
    file_line, *rest = file.split("\n")
    assert file_line.endswith(':')

    sections = []
    for line in rest:
        if line == "":
            continue
        sections.append(parse_splits_section(line))

    return SplitFile(file_line[:-1], sections)


def parse_splits_file(file: str) -> SplitsTxt:
    sections_block, rest = file.strip().split("\n\n", maxsplit=1)
    files = list(map(_parse_split_file, rest.split("\n\n")))

    return SplitsTxt(sections_block, files)


def serialize_splits_file(splits_file: SplitsTxt) -> str:
    result = splits_file.module_block
    result += "\n\n"

    for file in splits_file.files:
        result += file.path + ":\n"
        for section in file.sections:
            result += "\t" + section.name + " start:" + hex(section.start) + " end:" + hex(section.end)
            if section.attrs:
                result += " " + section.attrs

            result += "\n"

        result += "\n"

    return result
