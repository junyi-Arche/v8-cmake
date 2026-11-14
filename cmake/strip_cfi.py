#!/usr/bin/env python3
# Copyright 2026 the pyv8web authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Strip GNU assembler CFI directives from a mksnapshot generated embedded.S.

mksnapshot emits `.cfi_startproc` / `.cfi_def_cfa` / `.cfi_offset` for the
hand-written (JSEntry) builtins. Those directives are understood by gas/clang
but not by MASM (ml64), which is what assembles this file on Windows. Windows
unwind information comes from the UNWIND_INFO/XDATA records the Win writer
emits separately, so dropping the CFI lines is safe there.

Usage: strip_cfi.py <path-to-embedded.S>
"""

import sys

CFI_PREFIXES = (".cfi_",)


def main():
    if len(sys.argv) != 2:
        sys.stderr.write("usage: strip_cfi.py <embedded.S>\n")
        return 2
    path = sys.argv[1]
    with open(path, "rb") as f:
        data = f.read().decode("utf-8")
    out = []
    removed = 0
    for line in data.splitlines(True):
        stripped = line.lstrip()
        if stripped.startswith(CFI_PREFIXES):
            removed += 1
            continue
        out.append(line)
    if removed:
        with open(path, "wb") as f:
            f.write("".join(out).encode("utf-8"))
    sys.stdout.write("strip_cfi: removed %d CFI directive(s) from %s\n" %
                     (removed, path))
    return 0


if __name__ == "__main__":
    sys.exit(main())
