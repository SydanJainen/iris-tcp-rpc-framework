#!/usr/bin/env python3
import argparse
import json
import re
import sys
from datetime import datetime, timezone


TYPE_MAP: dict[str, str] = {
    "int": "int",
    "double": "double",
    "std::string": "string",
}

_FUNC_RE = re.compile(
    r"^\s*API_EXPORT\s+"      
    r"([\w:<>]+)\s+"      
    r"(\w+)\s*"     
    r"\(([^)]*)\)\s*;", 
)


_ARG_RE = re.compile(r"([\w:]+)\s+(\w+)")


def _map_type(cpp_type: str) -> str | None:
    return TYPE_MAP.get(cpp_type)


def parse_api_header(path: str) -> list[dict]:

    functions: list[dict] = []

    with open(path, encoding="utf-8") as f:
        for line_no, line in enumerate(f, start=1):
            match = _FUNC_RE.match(line)
            if not match:
                continue

            ret_type_cpp = match.group(1)
            func_name = match.group(2)
            args_str = match.group(3).strip()

            ret_type = _map_type(ret_type_cpp)
            if ret_type is None:
                print(
                    f"WARNING: skipping '{func_name}' at line {line_no} in {path} due to "
                    f"unsupported return type '{ret_type_cpp}'",
                    file=sys.stderr,
                )
                continue

            args: list[dict] = []
            skip = False
            if args_str:
                for arg_match in _ARG_RE.finditer(args_str):
                    arg_type_cpp = arg_match.group(1)
                    arg_name = arg_match.group(2)
                    arg_type = _map_type(arg_type_cpp)
                    if arg_type is None:
                        print(
                            f"WARNING: skipping '{func_name}' at line {line_no} in {path} due to "
                            f"unsupported argument type '{arg_type_cpp}' for parameter '{arg_name}'",
                            file=sys.stderr,
                        )
                        skip = True
                        break
                    args.append({"name": arg_name, "type": arg_type})

            if skip:
                continue

            functions.append({
                "name": func_name,
                "args": args,
                "returns": ret_type,
            })

    return functions


def generate_spec(functions: list[dict]) -> dict:
    return {
        "version": "1.0",
        "generated_at": datetime.now(timezone.utc).strftime(
            "%Y-%m-%dT%H:%M:%SZ"
        ),
        "functions": functions,
    }


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate api_spec.json from a C++ API header."
    )
    parser.add_argument(
        "header",
        help="Path to the C++ header file (e.g. server/src/api.h)",
    )
    parser.add_argument(
        "-o",
        "--output",
        default="api_spec.json",
        help="Output path for the spec JSON (default: api_spec.json)",
    )

    args = parser.parse_args()

    functions = parse_api_header(args.header)
    spec = generate_spec(functions)

    with open(args.output, "w", encoding="utf-8") as f:
        json.dump(spec, f, indent=4)

    print(
        f"Generated {args.output} with {len(functions)} function(s).",
        file=sys.stderr,
    )

if __name__ == "__main__":
    main()
