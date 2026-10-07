// The same load as _load in checks.S, built into a library that has only an arm64e slice.
.text
.globl _library_load
.p2align 2
_library_load:
    ldrb w0, [x0, x1]
    ret
