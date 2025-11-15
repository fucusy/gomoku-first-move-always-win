#!/usr/bin/env python3
"""Extract stepsString values from tornado logs.

This script scans the current directory for the files ``tornado_log`` and
``tornado_log.1`` through ``tornado_log.5``. For each file that exists, it
searches for ``stepsString=`` occurrences and writes the parsed sequences to an
output file (``parsed_steps.txt`` by default).
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path
from typing import Iterable

DEFAULT_LOG_BASENAME = "tornado_log"
DEFAULT_OUTPUT_FILE = "parsed_steps.txt"

STEP_PATTERN = re.compile(r"stepsString=(_[a-z]\d(?:_[a-z]\d)*)")


def iter_log_files(base_path: Path) -> Iterable[Path]:
    """Yield tornado log files from ``base_path`` if they exist."""

    candidates = [DEFAULT_LOG_BASENAME] + [
        f"{DEFAULT_LOG_BASENAME}.{suffix}" for suffix in range(1, 6)
    ]

    for name in candidates:
        path = base_path / name
        if path.exists() and path.is_file():
            yield path


def extract_steps(text: str) -> Iterable[str]:
    """Extract step sequences from the provided log text."""

    for match in STEP_PATTERN.finditer(text):
        # Drop the leading underscore in the recorded sequence.
        yield match.group(1)[1:]


def parse_logs(log_dir: Path, output_path: Path) -> None:
    """Parse all available tornado logs and write sequences to ``output_path``."""

    lines = []
    for log_file in iter_log_files(log_dir):
        content = log_file.read_text(encoding="utf-8", errors="ignore")
        lines.extend(extract_steps(content))

    output_path.write_text("\n".join(lines) + ("\n" if lines else ""), encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(description="Extract stepsString from tornado logs.")
    parser.add_argument(
        "--log-dir",
        type=Path,
        default=Path.cwd(),
        help="Directory containing tornado_log files (default: current directory)",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path(DEFAULT_OUTPUT_FILE),
        help="Destination file for the extracted sequences",
    )

    args = parser.parse_args()

    parse_logs(args.log_dir, args.output)


if __name__ == "__main__":
    main()
