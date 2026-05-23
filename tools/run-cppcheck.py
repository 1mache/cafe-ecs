#!/usr/bin/env python3

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


DEFAULT_EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hpp", ".hh", ".hxx"}
PROJECT_ROOT = Path(__file__).resolve().parent


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Run cppcheck on one or more files or directories with C++ defaults.",
    )
    parser.add_argument(
        "paths",
        nargs="+",
        help="Files or directories to analyze.",
    )
    parser.add_argument(
        "--cppcheck",
        default="cppcheck",
        help="Path to cppcheck executable. Default: cppcheck",
    )
    parser.add_argument(
        "--std",
        default="c++20",
        help="C++ standard to use. Default: c++20",
    )
    parser.add_argument(
        "--language",
        default="c++",
        help="Language to use. Default: c++",
    )
    parser.add_argument(
        "--enable",
        default="all",
        help="Cppcheck enable set. Default: all",
    )
    parser.add_argument(
        "--extra",
        nargs=argparse.REMAINDER,
        default=[],
        help="Extra args passed to cppcheck after --.",
    )
    return parser


def collect_paths(raw_paths: list[str]) -> list[str]:
    collected: list[str] = []

    for raw in raw_paths:
        path = Path(raw)
        if not path.exists():
            raise FileNotFoundError(f"Path not found: {raw}")

        if path.is_dir():
            collected.append(str(path))
            continue

        if path.suffix.lower() not in DEFAULT_EXTENSIONS:
            continue

        collected.append(str(path))

    return collected


def build_context_args() -> list[str]:
    context_args: list[str] = []

    for rel_path in ("include", "src", "tests", "dependencies"):
        candidate = PROJECT_ROOT / rel_path
        if candidate.exists():
            context_args.append(f"-I{candidate}")

    context_args.append("--suppress=missingIncludeSystem")
    return context_args


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    try:
        targets = collect_paths(args.paths)
    except FileNotFoundError as exc:
        print(exc, file=sys.stderr)
        return 2

    if not targets:
        print("No valid files or directories to check.", file=sys.stderr)
        return 2

    extra_args = list(args.extra)
    if extra_args[:1] == ["--"]:
        extra_args = extra_args[1:]

    command = [
        args.cppcheck,
        "--language=" + args.language,
        "--std=" + args.std,
        "--enable=" + args.enable,
        *build_context_args(),
        *extra_args,
        *targets,
    ]

    completed = subprocess.run(command)
    return completed.returncode


if __name__ == "__main__":
    raise SystemExit(main())