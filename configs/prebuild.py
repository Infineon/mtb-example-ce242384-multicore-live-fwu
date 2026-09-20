#!/usr/bin/env python3
import argparse
import json
import os
import sys
from typing import Any, Optional

DUAL_BANK_CTR_MASK = 0x5A3C0000

def get_var(name: str, cli_value: Optional[str]) -> str:
    """
    Resolve a variable from CLI first, then environment.
    Raises if not provided by either source.
    """
    if cli_value is not None and cli_value.strip() != "":
        return cli_value.strip()
    env_val = os.environ.get(name)
    if env_val is not None and env_val.strip() != "":
        return env_val.strip()
    raise KeyError(f"{name} not provided (use --{name.lower()} or set environment variable {name})")

def replace_placeholder_in_json(obj: Any, placeholder: str, replacement: str) -> Any:
    """
    Recursively replace occurrences of placeholder in all string values within a JSON-like object.
    """
    if isinstance(obj, dict):
        return {k: replace_placeholder_in_json(v, placeholder, replacement) for k, v in obj.items()}
    if isinstance(obj, list):
        return [replace_placeholder_in_json(v, placeholder, replacement) for v in obj]
    if isinstance(obj, str):
        return obj.replace(placeholder, replacement)
    return obj

def main():
    parser = argparse.ArgumentParser(
        description="Generate JSON from a template by replacing 'MAJ.MIN.REV+BN' using CLI/env variables."
    )
    parser.add_argument("--maj", help="Major version (or set env MAJ)")
    parser.add_argument("--min", help="Minor version (or set env MIN)")
    parser.add_argument("--rev", help="Revision (or set env REV)")
    parser.add_argument("--bn",  help="Build number (or set env BN)")
    parser.add_argument("--template", "-t", required=True, help="Path to the template JSON file.")
    parser.add_argument("--output", "-o", required=True, help="Path to write the generated JSON file.")
    parser.add_argument("--placeholder", "-p", default="MAJ.MIN.REV+BN",
                        help="Placeholder string to replace (default: MAJ.MIN.REV+BN).")
    parser.add_argument("--indent", type=int, default=2, help="Indent level for output JSON (default: 2).")
    args = parser.parse_args()

    try:
        maj = get_var("MAJ", args.maj)
        min_ = get_var("MIN", args.min)
        rev = get_var("REV", args.rev)
        bn = str(int(get_var("BN", args.bn)) | DUAL_BANK_CTR_MASK)
    except KeyError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

    version_str = f"{maj}.{min_}.{rev}+{bn}"

    try:
        with open(args.template, "r", encoding="utf-8") as f:
            template_obj = json.load(f)
    except Exception as e:
        print(f"Error reading template JSON: {e}", file=sys.stderr)
        sys.exit(1)

    result_obj = replace_placeholder_in_json(template_obj, args.placeholder, version_str)

    try:
        out_dir = os.path.dirname(args.output)
        if out_dir:
            os.makedirs(out_dir, exist_ok=True)
        with open(args.output, "w", encoding="utf-8") as f:
            json.dump(result_obj, f, indent=args.indent, ensure_ascii=False)
            f.write("\n")
    except Exception as e:
        print(f"Error writing output JSON: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
