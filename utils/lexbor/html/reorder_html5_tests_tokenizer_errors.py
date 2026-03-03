#!/usr/bin/env python3

"""
Reorders errors in html5lib tokenizer test files (.test) so that
surrogate-in-input-stream, noncharacter-in-input-stream, and
control-character-in-input-stream errors come first in each "errors" list.

Preserves the original formatting of the files.

Reads .test files from the 'tokenizer' directory (next to this script)
and saves results into a specified output directory.
"""

import re
import os
import glob

PRIORITY_ERRORS = {
    "surrogate-in-input-stream",
    "noncharacter-in-input-stream",
    "control-character-in-input-stream",
}


def is_priority(error_line):
    for code in PRIORITY_ERRORS:
        if '"' + code + '"' in error_line:
            return True
    return False


def reorder_error_block(match):
    prefix = match.group(1)   # "errors": [ or "errors":[
    content = match.group(2)  # everything between [ and ]
    suffix = match.group(3)   # ]

    lines = content.split("\n")

    # Separate error entry lines from non-entry lines (empty lines, etc.)
    error_entries = []
    leading = []   # lines before first entry
    trailing = []  # lines after last entry
    found_first = False
    found_last_idx = -1

    for i, line in enumerate(lines):
        stripped = line.strip()
        if stripped.startswith("{") and "code" in stripped:
            if not found_first:
                found_first = True
            found_last_idx = i
            error_entries.append(line)

    if len(error_entries) < 2:
        return match.group(0)

    priority = [e for e in error_entries if is_priority(e)]
    rest = [e for e in error_entries if not is_priority(e)]

    if not priority or priority + rest == error_entries:
        return match.group(0)

    reordered = priority + rest

    # Fix commas: all entries except the last one should end with comma
    fixed = []
    for i, entry in enumerate(reordered):
        stripped = entry.strip()
        if i < len(reordered) - 1:
            if not stripped.endswith(","):
                stripped += ","
        else:
            if stripped.endswith(","):
                stripped = stripped[:-1]

        indent = re.match(r'^(\s*)', error_entries[0]).group(1)
        fixed.append(indent + stripped)

    # Rebuild: leading non-entry lines + reordered entries + trailing non-entry lines
    # Find leading and trailing in original lines
    first_entry_idx = None
    last_entry_idx = None
    for i, line in enumerate(lines):
        stripped = line.strip()
        if stripped.startswith("{") and "code" in stripped:
            if first_entry_idx is None:
                first_entry_idx = i
            last_entry_idx = i

    leading = lines[:first_entry_idx]
    trailing = lines[last_entry_idx + 1:]

    new_lines = leading + fixed + trailing
    return prefix + "\n".join(new_lines) + suffix


def process_file(filepath, save_path):
    with open(filepath, "r", encoding="utf-8") as f:
        original = f.read()

    pattern = r'("errors"\s*:\s*\[)(.*?)(\])'
    result = re.sub(pattern, reorder_error_block, original, flags=re.DOTALL)

    changed = result != original

    with open(save_path, "w", encoding="utf-8") as f:
        f.write(result)

    return changed


def main(src_dir, dst_dir):
    os.makedirs(dst_dir, exist_ok=True)

    files = sorted(glob.glob(os.path.join(src_dir, "*.test")))

    if not files:
        print("No .test files found in {}".format(src_dir))
        return

    changed_files = 0

    for filepath in files:
        fname = os.path.basename(filepath)
        save_path = os.path.join(dst_dir, fname)
        if process_file(filepath, save_path):
            print("{}: reordered".format(fname))
            changed_files += 1
        else:
            print("{}: no changes".format(fname))

    print("\nSaved to: {}".format(dst_dir))
    print("Files modified: {}".format(changed_files))


if __name__ == "__main__":
    main("tokenizer", "tokenizer_reordered")
