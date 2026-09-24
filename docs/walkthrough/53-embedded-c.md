# Walkthrough 53: Embedded C for Security

**the C you must know to read and break firmware**

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

## Why C, Specifically

Firmware is almost always C (with some C++ and Rust). To read a binary you must know the C
that produced it. This volume is the subset that matters for security: memory, the stack,
the ABI, and the constructs that become bugs.

## 1. Memory: the Three Regions

```
+-------------------+
|  .text  (code)    |  read + execute
+-------------------+
|  .data  (init)    |  read + write  (copied from flash at boot)
|  .bss   (zeroed)  |  read + write  (zeroed at boot)
+-------------------+
|  heap   (grows up)|
|         ...       |
|  stack  (grows dn)|
+-------------------+
```

On bare metal there is no heap; on Linux there is. The lab uses **no heap** - all state is
static or on the stack. That is a security simplification: no allocator bugs.

## 2. Pointers

A pointer is just a number that indexes memory. In firmware you meet:

| pointer | use |
| ------- | --- |
| `uint32_t *` | a register or a word buffer |
| `volatile uint32_t *` | an **MMIO** register (must not be optimised away) |
| a function pointer | a callback or a jump table |
| a `char *` from input | the start of most bugs |

`volatile` matters on hardware: without it the optimiser deletes "write then readback"
sequences you need.

## 3. The Stack, and the Bug

Each function call creates a **frame**: saved registers, locals, and the **return address**.

```
+-------------------+
|  caller locals    |
+-------------------+
|  saved LR         |  <- the return address
|  saved regs       |
|  locals  [buf]    |  <- a fixed buffer often lives here
+-------------------+
```

If a copy into `buf` overflows, it climbs into the **saved LR**; on return, the CPU jumps
wherever LR now points.

```
strcpy(buf, input);   // input longer than buf -> LR overwritten
```

That is the whole memory-corruption class (`docs/walkthrough/29`).

## 4. The ABI (ARM)

| rule | consequence |
| ---- | ----------- |
| args in `r0..r3`, then the stack | reading a call means reading registers |
| return value in `r0` | |
| `LR` holds the return address | the overflow target |
| callee-saved `r4..r11` | a function must restore them |
| the stack is 8-byte aligned | |

On AArch64: `x0..x7` for args, `x30` for LR.

`_start` follows the ABI when it hands `main` to `__libc_start_main` - which is how you
find `main` in a stripped binary (`docs/WEEK12_CORTEX_A_RE.md`).

## 5. The Dangerous Functions

| function | bug |
| -------- | --- |
| `strcpy` | no bound |
| `strcat` | no bound |
| `sprintf` | no bound |
| `gets` | no bound |
| `memcpy` | safe **if** the length is right |
| `strncpy` | does **not** NUL-terminate |
| `snprintf` | safe **if** you check the return |

The **safe** idiom in this lab:

```c
n = snprintf(out, out_len, fmt, ...);
if ((n < 0) || ((size_t)n >= out_len)) return false;
```

## 6. Integer Bugs

| bug | example |
| --- | ------- |
| overflow | `size = a + b` wraps; a short check passes |
| underflow | `len - 1` when `len == 0` becomes huge |
| signedness | `int len` negative cast to `size_t` |
| truncation | `(uint8_t)n` when `n > 255` |

Guard by checking **before** the arithmetic: `if (a > LIMIT - b)`.

## 7. `system()` vs `execve`

```c
system("ping " + input);            /* a shell parses input */
execve("/bin/ping", argv, envp);    /* no shell; no metacharacters */
```

The first is the B2 bug (`docs/15`); the second is the fix.

## 8. Static vs Dynamic

| storage | lifetime | where |
| ------- | -------- | ----- |
| `static` file-scope | process | `.data`/`.bss` |
| `static` in a function | process | `.data`/`.bss` |
| automatic (local) | the frame | stack |
| `malloc` | until freed | heap |

The lab's `g_*` state is **file-scope static** - visible to the whole module, invisible
outside. That is deliberate encapsulation.

## 9. Reading C From Assembly

| C | AArch64 |
| - | ------- |
| `a + b` | `add w0, w0, w1` |
| `a & 0xFF` | `and w0, w0, #0xff` |
| `a >> 8` | `lsr w0, w0, #8` |
| `for (i=0;i<8;i++)` | a compare + a branch + an `add #1` |
| a call | `bl target` |
| a `return x` | `mov w0, x` then `ret` |

Once you can map these, the assembly of `crc.c` (`docs/walkthrough/01`) reads like C.

## 10. The Checklist

```
[ ] every copy is bounded
[ ] every length is checked before use
[ ] no signed/unsigned mix on sizes
[ ] no system() from input
[ ] no uninitialised secrets
[ ] no missing NUL terminator
[ ] MMIO is volatile
```

## Exercises

1. Draw the stack frame of `part_identify` and mark where a local would live.
2. Rewrite one `strncpy` use to guarantee a NUL.
3. Find one integer operation that could overflow and guard it.
4. Read `crc32_byte`'s assembly and map each line to the C.

## Reference

- `docs/walkthrough/01`, `docs/walkthrough/11`, `docs/walkthrough/30`
- `docs/WEEK12_CORTEX_A_RE.md`
