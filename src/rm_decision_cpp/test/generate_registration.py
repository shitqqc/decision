import argparse
import re
from pathlib import Path


def extract_registration(source):
    registration = re.search(
        r"void\s+BtEngine::registerNodes_\([^)]*\)\s*\{(.*?)^\}",
        source,
        re.DOTALL | re.MULTILINE,
    )
    if registration is None:
        raise RuntimeError("BtEngine node registration was not found")

    body = re.sub(
        r"/\*.*?\*/|//[^\n]*",
        "",
        registration.group(1),
        flags=re.DOTALL,
    )
    statements = re.findall(
        r"\bfactory\.registerNodeType\s*<.*?;",
        body,
        flags=re.DOTALL,
    )
    if not statements:
        raise RuntimeError("No C++ behavior tree nodes were found")

    includes = re.findall(
        r'^#include "rm_decision/bt/[^"\n]+"',
        source,
        flags=re.MULTILINE,
    )
    return includes, statements


def generate_registration(source):
    includes, statements = extract_registration(source)
    return "\n".join(
        [
            *includes,
            "#include <behaviortree_cpp/bt_factory.h>",
            "#include <rclcpp/rclcpp.hpp>",
            "",
            "namespace rm_decision",
            "{",
            "void registerTestNodes(",
            "  BT::BehaviorTreeFactory & factory, rclcpp::Node::SharedPtr node)",
            "{",
            *("  " + re.sub(r"\bzones\b", "nullptr", statement) for statement in statements),
            "}",
            "}",
            "",
        ]
    )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    source = args.source.read_text(encoding="utf-8")
    args.output.write_text(generate_registration(source), encoding="utf-8")


if __name__ == "__main__":
    main()
