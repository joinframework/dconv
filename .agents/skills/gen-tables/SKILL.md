---
name: gen-tables
description: Regenerate a power-of-ten table with tools/*powgen and keep lookups in sync. Use when changing a table, its range or a generator.
---

# Generate power tables

Never hand-edit table entries; every change goes through the generator.

| Header (`dconv/include/dconv/`) | Generator | Default range | Lookup |
|---|---|---|---|
| `grisupow.hpp` (`DiyFp[]`) | `grisupowgen` | `-343..343` | `mk + 343` in `grisu.hpp` |
| `eisellemirepow.hpp` (`Power[]`) | `eisellemirepowgen` | `-342..308` | `exponent + 342` in `eisellemire.hpp` |
| `schubfachpow.hpp` (`alignas (64) Power[]`) | `schubfachpowgen` | `-292..324` | `292 - k` in `schubfach.hpp` |

1. **Build** the tools with any preset (skill `compile`); binaries are in `build/<compiler>/<kind>/tools/<gen>/<gen>`.
2. **Check reproducibility first**: run the generator with the default range and compare its entries with the current header; they must match before any change.

   ```bash
   entries () { grep -oE '\{0x[0-9a-f]+, *-?(0x)?[0-9a-f]+\}' "$1" | tr -d ' '; }
   build/<compiler>/<kind>/tools/<gen>/<gen> > <scratchpad>/<gen>.hpp
   diff <(entries <scratchpad>/<gen>.hpp) <(entries dconv/include/dconv/<table>.hpp)
   ```

3. **Generate** with the new range (`-l <lower> -u <upper>`) or the modified generator.
4. **Paste** only the entries into the existing table; keep the header's guard, includes, namespace, declaration (name, element type, `alignas`) and Doxygen. Let `clang-format` lay out the lines.
5. **Sync** the lookup offset, any range assertion (`assert` in `grisu.hpp`) and the exponent bounds checks in the algorithm with the new range; update the generator README if its defaults changed.
6. **Verify**: re-run the entries diff against the new output, then the closing checks of `AGENTS.md` "Definition of done" (`conformance` and `performance` included).

Report the range, entry count before/after, and the test and conformance results.
