.text
.arch_extension cpa
.private_extern _add
.globl _add
.p2align 2
_add:
    addpt x0, x0, x1
    ret
.private_extern _load
.globl _load
.p2align 2
_load:
    ldrb w0, [x0, x1]
    ret
.private_extern _store
.globl _store
.p2align 2
_store:
    strb w2, [x0, x1]
    ret
