from dataclasses import dataclass
from pathlib import Path
from typing import Dict

from parsimonious.grammar import Grammar
from parsimonious.nodes import NodeVisitor

grammar = Grammar(r"""
document = header* using_statement+ indent* project ignored_lines*

project = 'struct Project {' indent config_statement indent '};' indent*

config_statement = 'static constexpr auto config = cib::components<' inline_comment?
item (',' inline_comment? item)* inline_comment? '>;'

item = template_instantiation / struct / alias

using_statement = 'using ' alias ' = ' template_instantiation ';\n'
template_instantiation = struct '<' dependency (', ' dependency)* '>'
alias = CarmelBack
struct = (snake_case '::')* CarmelBack
dependency = alias / number

CarmelBack = ~"[A-Z][a-zA-Z0-9]+"i
snake_case = ~"[a-z][_a-z0-9]+"i
number = ~"[0-9]+"i
indent = ~r"\n[ \t]*"i

inline_comment = ~r" *//[^\n]*\n *"i
header = !using_statement ~r"[^\n]*\n"
ignored_lines = ~r"[^\n]*\n"i
""")


@dataclass
class TemplateInstantiation:
    name: str
    dependencies: list[str]


class DependencyTree:
    alias: Dict[str, str] = {}
    fanout: list[TemplateInstantiation] = []


class Graph:
    nodes: set[str] = set()
    edges: Dict[str, list[str]] = {}


class DependencyExtractor(NodeVisitor):
    tree = DependencyTree()

    def visit_config_statement(self, _, visited_children):
        _, _, first_item, other_items, _, _ = visited_children

        if isinstance(first_item[0], TemplateInstantiation):
            self.tree.fanout.append(first_item[0])

        for _, _, item in other_items:
            if isinstance(item[0], TemplateInstantiation):
                self.tree.fanout.append(item[0])

    def visit_using_statement(self, _, visited_children):
        _, alias, _, definition, _ = visited_children
        # print(f'{alias} <- {definition.name} {definition.dependencies}')
        self.tree.alias.update({alias: definition.name})
        self.tree.fanout.append(definition)

    def visit_template_instantiation(
        self, _, visited_children
    ) -> TemplateInstantiation:
        struct_name, _, first_dependency, other_dependencies, _ = visited_children

        dependencies: list[str] = first_dependency
        for item in other_dependencies:
            dependencies.append(item[1][0])

        return TemplateInstantiation(struct_name, dependencies)

    def visit_number(self, node, _) -> str:
        return node.text

    def visit_struct(self, node, _) -> str:
        return node.text

    def visit_snake_case(self, node, _) -> str:
        return node.text

    def visit_CarmelBack(self, node, _) -> str:
        return node.text

    def generic_visit(self, node, visited_children):
        return visited_children or node


def extractDependencyTree(source_code: str) -> DependencyTree:
    # Parse the C++ file into abstract syntax tree
    ast = grammar.parse(source_code + "\n")

    # Compile into Python structs
    extractor = DependencyExtractor()
    extractor.visit(ast)

    return extractor.tree


def sanitizedName(tag: str) -> str:
    return tag.replace(":", "_")


def convertToGraph(tree: DependencyTree) -> Graph:
    graph = Graph()

    # Extract nodes
    for _, node in tree.alias.items():
        graph.nodes.add(node)

    for item in tree.fanout:
        # Resolve the alias
        target = tree.alias[item.name] if item.name in tree.alias else item.name

        # Deduplicate nodes
        graph.nodes.add(target)

        # Extract edges
        sources: list[str] = []
        for d in item.dependencies:
            # Resolve alias
            resolved_dependency: str = tree.alias[d] if d in tree.alias else d
            sources.append(resolved_dependency)

        graph.edges.update({target: sources})

    return graph


def generateDigraph(graph: Graph) -> None:
    print('digraph "DependencyTree" {')

    # Extract nodes
    for node in graph.nodes:
        print(f'    {sanitizedName(node)} [label="{node}"]')

    # Extact edges
    for target, sources in graph.edges.items():
        for s in sources:
            print(f"    {sanitizedName(target)} -> {sanitizedName(s)}")

    print("}")


if __name__ == "__main__":
    with open(Path("../main.cpp"), "r") as f:
        tree = extractDependencyTree(f.read())

    graph = convertToGraph(tree)
    generateDigraph(graph)
