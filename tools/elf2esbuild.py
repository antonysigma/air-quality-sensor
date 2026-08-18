#!/usr/bin/env python3

import argparse
import json
import subprocess
from collections import defaultdict
from pathlib import Path

from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection
from parsimonious.grammar import Grammar
from parsimonious.nodes import NodeVisitor

SHF_ALLOC = 0x2

CPP_NAME = Grammar(r"""
name        = prefix? component (scope component)*
scope       = "::"

prefix      = ("vtable" / "guard variable") " for "

component   = top_chunk+
top_chunk   = angle / paren / bracket / brace / top_plain

angle       = "<" nested_chunk* ">"
paren       = "(" nested_chunk* ")"
bracket     = "[" nested_chunk* "]"
brace       = "{" nested_chunk* "}"

nested_chunk = angle / paren / bracket / brace / nested_plain

# At top level, :: is forbidden because it separates hierarchy components.
top_plain    = ~r"(?:(?!::)[^<>()\[\]{}])+"

# Inside a nested construct, :: is perfectly legal.
nested_plain = ~r"[^<>()\[\]{}]+"
""")


class ScopeVisitor(NodeVisitor):
    def visit_name(self, node, children):
        prefix, first, rest = children

        parts = [prefix, first] if isinstance(prefix, str) else [first]

        if len(rest) > 0 and rest[0] is None:
            parts.append(rest[1])
        else:
            parts.extend(component for _, component in rest)

        return parts

    def visit_component(self, node, children):
        return node.text.strip()

    def visit_scope(self, node, children):
        return None

    def visit_prefix(self, node, children):
        return node.text.strip()

    def generic_visit(self, node, children):
        if len(children) == 1:
            return children[0]
        return children


def split_cpp_name(name: str) -> list[str]:
    return ScopeVisitor().visit(CPP_NAME.parse(name))


def clean(name: str) -> str:
    return name.strip().replace("/", "∕").replace("\\", "∖") or "[anonymous]"


def demangle(names: set[str]) -> dict[str, str]:
    mangled = sorted(n for n in names if n.startswith("_Z") or n.startswith("_GLOBAL"))

    if not mangled:
        return {}

    p = subprocess.run(
        ["c++filt"],
        input="\n".join(n.replace("_GLOBAL__sub_I_", "") for n in mangled) + "\n",
        text=True,
        stdout=subprocess.PIPE,
        check=True,
    )

    return dict(zip(mangled, p.stdout.splitlines()))


def load_symbols(elf):
    symtab = elf.get_section_by_name(".symtab")

    if not isinstance(symtab, SymbolTableSection):
        raise RuntimeError("ELF has no usable .symtab")

    result = []

    for symbol in symtab.iter_symbols():
        shndx = symbol["st_shndx"]

        if not isinstance(shndx, int):
            continue

        section = elf.get_section(shndx)

        if not (section["sh_flags"] & SHF_ALLOC):
            continue

        if symbol["st_info"]["type"] in ("STT_FILE", "STT_SECTION"):
            continue

        size = int(symbol["st_size"])

        if size:
            result.append(
                (
                    shndx,
                    symbol.name,
                    int(symbol["st_value"]),
                    size,
                )
            )

    return result


def symbol_path(section, symbol, hierarchy):
    if hierarchy == "flat":
        return f"{clean(section)}/{clean(symbol)}"

    return "/".join([clean(section), *(clean(x) for x in split_cpp_name(symbol))])


def build_metadata(elf, hierarchy, output_name):
    symbols = load_symbols(elf)
    demangled = demangle({name for _, name, _, _ in symbols})

    by_section = defaultdict(list)

    for shndx, name, address, size in symbols:
        by_section[shndx].append((demangled.get(name, name), address, size))

    sizes = defaultdict(int)

    for shndx, section in enumerate(elf.iter_sections()):
        if not (section["sh_flags"] & SHF_ALLOC):
            continue

        section_size = int(section["sh_size"])

        if not section_size:
            continue

        attributed = 0

        for name, _, size in by_section[shndx]:
            sizes[symbol_path(section.name, name, hierarchy)] += size
            attributed += size

        if section_size > attributed:
            sizes[f"{clean(section.name)}/[unattributed]"] += section_size - attributed

        elif attributed > section_size:
            print(
                f"warning: {section.name}: symbols={attributed}, section={section_size}"
            )

    inputs = {
        name: {"bytes": size, "imports": []} for name, size in sorted(sizes.items())
    }

    return {
        "inputs": inputs,
        "outputs": {
            output_name: {
                "imports": [],
                "exports": [],
                "inputs": {
                    name: {"bytesInOutput": size}
                    for name, size in sorted(sizes.items())
                },
                "bytes": sum(sizes.values()),
            }
        },
    }


def main():
    p = argparse.ArgumentParser()
    p.add_argument("elf", type=Path)
    p.add_argument("-o", "--output", type=Path, default=Path("metadata.json"))
    p.add_argument(
        "--hierarchy",
        choices=("flat", "cpp"),
        default="flat",
    )
    args = p.parse_args()

    with args.elf.open("rb") as f:
        elf = ELFFile(f)
        metadata = build_metadata(
            elf,
            args.hierarchy,
            args.elf.name,
        )

    args.output.write_text(json.dumps(metadata, indent=2) + "\n")


if __name__ == "__main__":
    main()
