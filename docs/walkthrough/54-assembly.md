# Walkthrough 54: Reading Assembly

**the ARM/AArch64 patterns you will see in every firmware**

***
**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. 

You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:

1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws regarding cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**

***

## Why Patterns

You do not read assembly instruction by instruction; you read **patterns**. A `bl` is a
call; a `cmp` + `b.eq` is an `if`; a loop has an add, a compare, and a backward branch.
Learn the patterns and any function is readable in minutes.

## 1. AArch64 Patterns

### Call and return

```asm
bl   target        ; call
ret                ; return (to x30/LR)
```

### If

```asm
cmp  w0, w1
b.eq label         ; if (a == b) goto label
```

The condition suffix: `eq ne gt ge lt le hi lo`.

### Loop

```asm
    mov  w19, #0        ; i = 0
    mov  w20, #8        ; limit
loop:
    ...
    add  w19, w19, #1   ; i++
    cmp  w19, w20
    b.lt loop           ; if (i < limit) goto loop
```

### A function prologue

```asm
stp  x29, x30, [sp, #-16]!   ; save frame pointer + return address
mov  x29, sp                 ; new frame
str  x19, [sp, #8]           ; save a callee register
...
ldp  x29, x30, [sp], #16     ; restore
ret
```

If a function writes to a stack buffer without a bound, the **saved x30** is the overflow
target (`docs/walkthrough/53`).

### Word/byte access

```asm
ldr  w8, [x0]        ; load 4 bytes
ldr  w8, [x0, #8]    ; load 4 bytes at offset 8
ldrb w8, [x0]        ; load 1 byte
str  w8, [x0]        ; store 4 bytes
strb w8, [x0]        ; store 1 byte
```

### The immediate that matters

```asm
mov  w8, #33568
movk w8, #60856, lsl #16     ; w8 = 0xEDB88320  <- the CRC polynomial
```

The constants tell you **which algorithm**. `0xEDB88320` = CRC-32; `0x1021` = CRC-16/CCITT;
`0x61707865` = "expa" = ChaCha (`docs/walkthrough/11`).

## 2. ARM32 (A32) Patterns

The camera class is 32-bit:

```asm
push {r4, lr}        ; save
pop  {r4, pc}        ; return (pc <- saved lr)
cmp  r0, #3          ; compare
bhi  label           ; if (unsigned) r0 > 3
bl   target          ; call
ldr  r0, [pc, #4]    ; load from the literal pool
add  r0, pc, r0      ; PIC string address
```

The `push {..., lr}` / `pop {..., pc}` pair is the ARM32 call convention; a missing
stack-protector means the frame is directly overflowable.

## 3. The Literal Pool

32-bit constants live in a **literal pool** near the code; `ldr rX, [pc, #imm]` loads them.
This is how you resolve string addresses and function pointers:

```
instruction at 0x107a4: ldr r0, [pc, #1448]
pool = 0x107a4 + 8 + 1448 = 0x10d... ; read the 4 bytes there
```

That is how you find pooled strings and, in a stripped binary, **the address of `main`**
(`docs/WEEK12_CORTEX_A_RE.md`).

## 4. Recognising Algorithms

| pattern | likely algorithm |
| ------- | ---------------- |
| `0xEDB88320` (as mov/movk or a literal) | reflected CRC-32 |
| `0x1021` | CRC-16/CCITT |
| `0x67452301 0xEFCDAB89 ...` | MD5 |
| `0x6A09E667 ...` | SHA-256 |
| `AESE`/`AESD`/`PMULL` | AES / GCM |
| the 4 ChaCha constants | ChaCha20 |
| a ladder of `mul`/`add`/`xor` | a hash or a KDF |

The constants are the fingerprints (`docs/walkthrough/19`, `docs/walkthrough/18`).

## 5. Recognising Bugs

| pattern | bug |
| ------- | --- |
| `bl strcpy` with a fixed dest | overflow |
| `bl sprintf` with a fixed dest | overflow |
| `bl system` | shell from a string |
| a length read from input used as a copy size | unbounded copy |
| a `cmp` on a length that does not bound the copy | missing check |
| `bl memcmp` on a tag without a following branch | a check that never fails |

That last one is why `aead_open` calls `EVP_DecryptFinal_ex` and **branches on the result**
(`docs/walkthrough/11`).

## 6. Reading a Function, Step by Step

```
1. Prologue: what is saved? (canary? frame?)
2. Arguments: which registers are used as inputs?
3. Calls: what does it call? (bl targets)
4. Constants: what algorithm?
5. Loops: what is the loop bound?
6. Copies: any strcpy/sprintf, and is the dest fixed?
7. Return: what is returned in r0/w0?
```

Seven questions, any function, any architecture.

## 7. The Transliteration

```asm
bl   crc32_le            ; crc32_le(...)
ldr  w8, [x19, #8]       ; node[8]
cmp  w0, w8              ; computed == stored?
cset w0, eq             ; return the comparison
```

```
return crc32_le(0, node, 8) == node[8];   // jffs2_hdr_crc_valid
```

When the assembly maps this cleanly to the C, you have understood the function.

## Exercises

1. Take a function from `docs/appendix/B` and answer the seven questions.
2. Find the CRC polynomial in `crc.c`'s assembly.
3. Find a `bl` to `strcpy`/`sprintf`/`system` in a real binary.
4. Transliterate one AArch64 function back to C.

## Reference

- `docs/appendix/B-aarch64-disasm.md`, `docs/appendix/H-annotated-disasm.md`
- `docs/walkthrough/11`, `docs/walkthrough/54`
