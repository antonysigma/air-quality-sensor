from dataclasses import dataclass
from pathlib import Path
from typing import Dict

from parsimonious.grammar import Grammar
from parsimonious.nodes import NodeVisitor

grammar = Grammar(r"""
document = top_level_line* project ignored_lines*

top_level_line = using_statement / pre_project

project = 'struct Project {' indent config_statement indent '};' indent*

config_statement = 'static constexpr auto config = cib::components<' indent
item (',' indent item)* indent '>;'

item = template_instantiation / qualified_name

using_statement = using_alias / using_namespace
using_alias = 'using ' qualified_name ' = ' definition ';\n'
using_namespace = 'using namespace ' qualified_name ';\n'
definition = template_instantiation / qualified_name
template_instantiation = qualified_name '<' dependency (', ' dependency)* '>'
qualified_name = (snake_case / CarmelBack) ('::' (snake_case / CarmelBack))*
dependency = template_instantiation / value_struct / qualified_name / number
value_struct = qualified_name "{" ~r"[^\}]+" "}"

CarmelBack = ~"[A-Z][a-zA-Z0-9]+"i
snake_case = ~"[a-z][_a-z0-9]+"i
number = ~"[0-9]+"i
indent = ~r"\n[ \t]*"i

pre_project = !"struct Project {" ~r"[^\n]*\n"
ignored_lines = ~r"[^\n]*\n"i
""")


@dataclass
class TemplateInstantiation:
    name: str
    dependencies: list[str]


@dataclass
class DependencyTree:
    alias: Dict[str, str]
    fanout: list[TemplateInstantiation]


@dataclass
class Graph:
    nodes: set[str]
    edges: Dict[str, list[str]]


class DependencyExtractor(NodeVisitor):
    def __init__(self):
        super().__init__()
        self.tree = DependencyTree(alias={}, fanout=[])

    def visit_config_statement(self, _, visited_children):
        for item in self._walk_template_instantiations(visited_children):
            self.tree.fanout.append(item)

    def visit_item(self, _, visited_children):
        return visited_children[0]

    def visit_definition(self, _, visited_children):
        return visited_children[0]

    def visit_using_alias(self, _, visited_children):
        _, alias, _, definition, _ = visited_children
        target = (
            definition.name
            if isinstance(definition, TemplateInstantiation)
            else definition
        )
        self.tree.alias.update({alias: target})
        if isinstance(definition, TemplateInstantiation):
            self.tree.fanout.append(definition)

    def visit_using_namespace(self, _, visited_children):
        return None

    def visit_template_instantiation(
        self, _, visited_children
    ) -> TemplateInstantiation:
        struct_name, _, first_dependency, other_dependencies, _ = visited_children

        dependencies: list[str] = [first_dependency]
        for item in other_dependencies:
            dependencies.append(item[1])

        return TemplateInstantiation(struct_name, dependencies)

    def visit_dependency(self, _, visited_children) -> str:
        return visited_children[0]

    def visit_value_struct(self, _, visited_children) -> str:
        qualified_name, _, _, _ = visited_children
        return qualified_name

    def visit_number(self, node, _) -> str:
        return node.text

    def visit_qualified_name(self, node, _) -> str:
        return node.text

    def visit_snake_case(self, node, _) -> str:
        return node.text

    def visit_CarmelBack(self, node, _) -> str:
        return node.text

    def _walk_template_instantiations(self, value):
        if isinstance(value, TemplateInstantiation):
            yield value
        elif isinstance(value, (list, tuple)):
            for item in value:
                yield from self._walk_template_instantiations(item)

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


def resolveName(tree: DependencyTree, name: str) -> str:
    if name in tree.alias:
        return tree.alias[name]

    if "::" not in name:
        return name

    suffix = name.rsplit("::", 1)[1]
    if suffix in tree.alias:
        return tree.alias[suffix]

    if suffix[:1].islower() or suffix[:1].isdigit():
        return resolveName(tree, name.rsplit("::", 1)[0])

    return name


def convertToGraph(tree: DependencyTree) -> Graph:
    graph = Graph(nodes=set(), edges={})

    # Extract nodes
    for _, node in tree.alias.items():
        graph.nodes.add(node)

    for item in tree.fanout:
        # Resolve the alias
        target = resolveName(tree, item.name)

        # Deduplicate nodes
        graph.nodes.add(target)

        # Extract edges
        sources: list[str] = []
        for d in item.dependencies:
            # Resolve alias
            resolved_dependency: str = resolveName(tree, d)
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
