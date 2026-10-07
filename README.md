# apple-cpa

The code behind [Checked pointers on Apple silicon](https://almamatertech.com/notes/checked-pointers-on-apple-silicon/),
a post on checked pointer arithmetic (CPA) in Apple's `arm64e.x1` slice. CPA stops pointer arithmetic
from changing a pointer's top byte, where memory tagging (MTE) keeps its tag.

## What's here

| Path | What it is | Post section |
| --- | --- | --- |
| `examples/` | The functions the post compiles for both slices. They're never run. | [What Xcode emits](https://almamatertech.com/notes/checked-pointers-on-apple-silicon/#what-xcode-emits) |
| `tests/` | The test program, its assembly, an `arm64e`-only library and the entitlements for three variants | [Testing on the M6](https://almamatertech.com/notes/checked-pointers-on-apple-silicon/#testing-on-the-m6) |

## Requirements

Building needs Xcode 27. Running the tests also needs a Mac whose CPU can switch the check on, an M6
or later with macOS 27, where `sysctl hw.optional.arm.FEAT_CPA2` prints 1.

## Disassembly

```
make disassembly
```

This compiles each example for `arm64e` and `arm64e.x1` at `-O0`, `-Os` and `-O2` and writes the
disassembly to `build/disassembly`, for example `arithmetic.arm64e.x1.O2.txt`.
`add_index.arm64e.O2.checked.txt` is the software check from the post's appendix.

## Tests

```
make
```

This builds `build/tests-full`, `build/tests-mte-only` and `build/tests-none`, the same `arm64e.x1`
code signed with `tests/full.plist`, `tests/mte-only.plist` and `tests/none.plist`. Only `full.plist`
has the key that turns CPA on. All three let LLDB attach.

Every test but `valid` adds `0x0100000000000000` to the address of a page the program maps, which
changes the top byte from `00` to `01`:

```
$ build/tests-full add
slice arm64e.x1, process flags 0x84404010
page   0x0000000104fc4000
offset 0x0100000000000000
ADDPT  0x0040000104fc4000
not used
exited normally
```

The full variant poisoned the result, keeping the top byte `00` and setting bit 54. The other two
print the sum, `0x01…`.

| Test | What it does | full | mte-only | none |
| --- | --- | --- | --- | --- |
| `valid` | `read`, `load` and `library` with offset 0 | reads 42 | reads 42 | reads 42 |
| `add` | ADDPT the offset, don't use it | poisoned | sum | sum |
| `read` | read through that result | CPA fault | tag fault | reads |
| `load` | load with the offset inside the instruction | CPA fault | tag fault | reads |
| `write` | write through that result | CPA fault | tag fault | writes |
| `store` | store with the offset inside the instruction | CPA fault | tag fault | writes |
| `library` | `load`, in the `arm64e`-only library | CPA fault | tag fault | reads |

A CPA fault is `EXC_BAD_ACCESS` with subtype `EXC_ARM_CPA_FAIL` and code `0x108`. A tag fault has
code `0x107`. Run a test under LLDB to see which one stopped it:

```
xcrun lldb build/tests-full -o "run read"
```

Licensed under [MIT](LICENSE).
