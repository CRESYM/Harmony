#!/usr/bin/env python3
"""Remove UTF-8 BOM (U+FEFF) from source files."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DIRS = ["src", "input_file", "tests"]
BOM = b"\xef\xbb\xbf"


def main() -> None:
    fixed = 0
    skipped = 0
    for name in DIRS:
        base = ROOT / name
        if not base.is_dir():
            continue
        for path in base.rglob("*"):
            if path.suffix not in {".cpp", ".h", ".hpp"}:
                continue
            try:
                raw = path.read_bytes()
            except PermissionError:
                print(f"SKIP (locked): {path}")
                skipped += 1
                continue
            if BOM not in raw:
                continue
            try:
                path.write_bytes(raw.replace(BOM, b""))
                fixed += 1
            except PermissionError:
                print(f"SKIP (locked): {path}")
                skipped += 1
    print(f"Fixed {fixed} files, skipped {skipped}")


if __name__ == "__main__":
    main()
