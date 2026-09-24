# Week 12: Cortex-A Reverse Engineering - Finding the Reset Handler and `main()` in a Stripped Binary

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

## What You'll Learn This Week

By the end of this tutorial, you will be able to:

- Explain why a **Cortex-A** boots differently from the Cortex-M33 you used in Weeks 1-11
- Read a **Cortex-A exception vector table** and identify the **reset handler** by its bytes
- Find the reset handler in a **stripped** bare-metal image with no symbols at all
- Trace the reset handler into the **C runtime** and the first C function
- Find **`main()`** in a **stripped Linux** application by following `_start` into the C library
- Use `readelf`, `objdump`, `strings`, Ghidra, and radare2 to prove every step
- Map the whole chain onto a **Raspberry Pi 5** (Cortex-A76) and the TELESCREEN device

## Review from Weeks 1-11

Everything you learned on the Pico 2 (Cortex-M33) still matters:

- **Registers and the stack** - the A-series has the same `r0`-`r15`, SP, LR, PC model
- **The vector table** - but on Cortex-A it looks completely different (this week)
- **Ghidra and GDB** - same tools, different entry conventions
- **Thumb mode** - the LSB-of-address trick still applies
- **Reading raw bytes** - the skill that finds the reset handler is pure byte forensics

> **The one big difference:** a Cortex-M loads a **stack pointer and a function
> address** from the vector table; a Cortex-A executes a **branch instruction** at the
> same offsets. That single fact is the key to this entire week.

---

## Part 1: Why Cortex-A Is Different

### Cortex-M (Weeks 1-11, RP2350)

The Cortex-M33 vector table is a **table of addresses**. The hardware reads two 32-bit
words and loads them into registers:

```
offset 0x00 : initial MSP (stack pointer value)
offset 0x04 : reset handler address  -> loaded into PC
offset 0x08 : NMI handler address
...
```

The hardware **does not execute** anything in the table - it **loads** from it. The
first instruction that executes is at the address stored at `0x04`.

### Cortex-A (this week, RP5 and the TELESCREEN device)

The Cortex-A vector table is a **table of instructions**. The hardware reads the
opcode at each offset and **executes it as a branch**:

```
offset 0x00 : B <reset>      ; reset
offset 0x04 : B <undef>      ; undefined instruction
offset 0x08 : B <svc>        ; supervisor call
offset 0x0C : B <prefetch>   ; prefetch abort
offset 0x10 : B <data>       ; data abort
offset 0x14 : B <unused>     ; reserved (older cores: hypervisor)
offset 0x18 : B <irq>        ; IRQ
offset 0x1C : B <fiq>        ; FIQ
```

So on Cortex-A you do not read a pointer at `0x00` - you **decode a branch opcode at
`0x00`**. That opcode *is* the reset handler's first instruction (it branches to it).

> **Memory hook:** on Cortex-M the table says *"the reset handler is over there."* On
> Cortex-A the table says *"go there now"* - the table is executable code.

---

## Part 2: The ARM Branch Opcode (the bytes that reveal everything)

An ARM (A32) branch is a single 32-bit instruction. The top byte is `0xEA`, and the low
24 bits are a signed word offset:

```
 31    28 27                    24 23                              0
+--------+------------------------+----------------------------------+
|  cond  |      1 0 1 0           |          signed offset           |
+--------+------------------------+----------------------------------+
   1110        1010                     (24-bit, in words)
```

- `cond = 1110` (`AL`, always)
- pattern `1010`
- `offset` is in **words** (4 bytes), **PC-relative**, and the PC is `address + 8`
  (the classic ARM pipeline).

So a little-endian reset vector looks like this in a hex dump:

```
offset 0x00:  15 05 00 ea
```

Read it back as a little-endian word: `0xEA000515`. Decode it:

```
cond = 0xE (AL)
opcode = 0xA (branch)
offset = 0x000515 = 1301 words
target = (0x00 + 8) + (1301 * 4) = 0x08 + 0x1454 = 0x145C
```

**That is the reset handler: `0x145C`.** You found it with no symbols, no debug info,
and no vendor documentation - just the bytes.

### The "branch to yourself" filler

The other seven entries are almost always:

```
fe ff ff ea   ->  0xEAFFFFFE  ->  B .    (branch to itself, an infinite loop)
```

That is the safe default for an unhandled exception: hang forever. Seeing seven
`EAFFFFFE` words after a real branch is the signature of a Cortex-A vector table.

---

## Part 3: Finding the Reset Handler - The Method

Here is the complete, repeatable method. It works on any Cortex-A bare-metal image.

### Step 1: Get the bytes

```bash
# carve the boot partition (see the TELESCREEN four-partition layout)
dd if=TELESCREEN-full.img of=boot.img bs=1 count=$((0x20000))
```

### Step 2: Look at offset zero

```bash
xxd -l 32 boot.img
```

Real output from the TELESCREEN device:

```
00000000: 15 05 00 ea  fe ff ff ea  fe ff ff ea  fe ff ff ea
00000010: fe ff ff ea  fe ff ff ea  fe ff ff ea  fe ff ff ea
```

Read it as a table:

| offset | word | meaning |
|--------|------|---------|
| `0x00` | `0xEA000515` | `B 0x145C` - **reset** |
| `0x04` | `0xEAFFFFFE` | `B .` - undef |
| `0x08` | `0xEAFFFFFE` | `B .` - svc |
| `0x0C` | `0xEAFFFFFE` | `B .` - prefetch abort |
| `0x10` | `0xEAFFFFFE` | `B .` - data abort |
| `0x14` | `0xEAFFFFFE` | `B .` - reserved |
| `0x18` | `0xEAFFFFFE` | `B .` - IRQ |
| `0x1C` | `0xEAFFFFFE` | `B .` - FIQ |

### Step 3: Disassemble the reset handler

Tell the disassembler it is ARM, 32-bit, little-endian, and go to the target:

```bash
arm-none-eabi-objdump -D -b binary -m arm --start-address=0x145c --stop-address=0x1500 boot.img
# or with radare2
r2 -a arm -b 32 -q -c 's 0x145c; pd 24' boot.img
```

### Step 4: Read what the reset handler does

Real output from the TELESCREEN device:

```asm
0x145c  ldr r0, =0x120280F8     ; an SoC control register
0x1460  mov r1, #1
0x1464  str r1, [r0]            ; poke it
0x1468  b 0x16C0                ; continue
...
0x146c  mrs r0, CPSR            ; read processor state
0x1470  and r1, r0, #0x1f
0x1474  teq r1, #0x1a           ; are we in HYP mode?
0x1478  orrne r0, r0, #0x13     ; if so, move to SVC
0x1480  orr r0, r0, #0xc0       ; disable IRQ and FIQ
0x1484  msr CPSR_fc, r0         ; commit
0x1488  mrc p15, 0, r0, c1, c0, 0   ; read SCTLR
0x148c  bic r0, r0, #0x2000     ; clear V (high vectors off)
0x1490  mcr p15, 0, r0, c1, c0, 0   ; write SCTLR
0x1494  mcr p15, 0, r0, c12, c0, 0  ; VBAR = r0 (vector base)
```

You have now identified the reset handler, the processor-mode setup, and the vector
base - **the entire boot contract** - from a stripped blob.

> **Why this matters:** this is exactly the work you did on the Pico, but the *table*
> is code, not pointers. Once you internalize the `0xEA` branch decode, every Cortex-A
> image in the world opens up the same way.

---

## Part 4: From Reset to the C Runtime

The reset handler is assembly, not C. To reach the first C function you follow it
forward:

1. **Processor state** - set SVC mode, disable interrupts (the `CPSR` writes above).
2. **MMU and caches** - `mrc/mcr p15, ...` to SCTLR, and the `c12` vector base.
3. **Early RAM** - a call like `bl 0x17e4` that trains DRAM (there is no stack yet).
4. **SoC init** - a call like `bl 0x1ae8` that brings up clocks and pads.
5. **Jump to the next stage** - often `mov pc, r1` (U-Boot proper on the TELESCREEN).

```
vector[0] = B 0x145C
    |
    +-- 0x145C  reset: poke SoC reg, branch to CPU setup
    +-- 0x146C  SVC mode, IRQ/FIQ off, VBAR=0
    +-- 0x14BC  cold path: set SP, bl 0x17E4 (DRAM), bl 0x1AE8 (SoC)
    +-- 0x14F4  mov pc, r1   -> next stage (U-Boot / kernel)
```

**Finding the first C function** on bare metal: look for the standard C prologue
`push {r4, lr}` (or `stmdb sp!, {...}`) right after a `bl` to a stack-setup routine,
and for a call to a function that zeroes memory (`.bss`) and copies `.data`. That is
`crt0`. The function it calls last is usually `main`.

---

## Part 5: Finding `main()` in a Stripped Linux Application

Now the harder and more common case: a **stripped Linux ELF** like the TELESCREEN
application. There are no symbols, but the ABI leaves fingerprints.

### Step 1: The ELF entry point

The ELF header stores the entry point - the address the kernel jumps to:

```bash
readelf -h teled | grep Entry
# Entry point address: 0x10e64
```

That entry is **`_start`**, supplied by the C library's start files, not by your
program. `_start` is always at the entry point.

### Step 2: Recognize the `_start` prologue

Disassemble at the entry point:

```bash
arm-none-eabi-objdump -d teled --start-address=0x10e64 --stop-address=0x10ea0
```

The canonical ARM `_start` looks like this (real bytes from the TELESCREEN app):

```asm
0x10e64  mov fp, #0
0x10e68  mov lr, #0
0x10e6c  pop {r1}            ; argc
0x10e70  mov r2, sp          ; argv
0x10e74  push {r2}
0x10e78  push {r0}
0x10e7c  ldr ip, [pc, #16]   ; -> __libc_start_main
0x10e80  push {ip}
0x10e84  ldr r0, [pc, #12]   ; -> main
0x10e88  ldr r3, [pc, #12]   ; -> __libc_csu_init (or 0)
0x10e8c  b __libc_start_main@plt
```

This is a **fingerprint**. Every ARM Linux binary produced by GCC/Clang has a variant
of it. The three `ldr ..., [pc, #...]` loads are the giveaway:

- one points at the C library's `__libc_start_main`
- **one points at `main`** - that is the address you want
- one points at the init/fini helper

### Step 3: Resolve the literal pool to get `main`

Each `ldr rX, [pc, #imm]` reads a 32-bit word from a nearby literal pool. The pool
word is the **address of the function**. Do the arithmetic:

```
instruction at 0x10e84:  ldr r0, [pc, #12]
pool address = 0x10e84 + 8 + 12 = 0x10e98
read 4 bytes at 0x10e98 -> 0x00010780
=> main = 0x10780
```

On the TELESCREEN application this is exactly what happens:

```bash
# the entry's first-argument load
arm-none-eabi-objdump -s -j .text teled | grep -A2 10e90
#  10e90  04feffeb 68160100 80070100 00060100
#                        ^^^^^^^^  this word is main
```

`0x00010780` is `main`. You found it with no symbols.

### Step 4: Confirm it with the decompiler

Open the binary in Ghidra (ARM:LE:32:v7), go to the entry point, and follow the first
argument into `__libc_start_main`. Ghidra names the target function `FUN_00010780`
until you rename it `main`. radare2 users can do the same:

```bash
r2 -a arm -b 32 -q -c 'aaa; s entry0; pdf' teled
```

### Step 5: The library-call fallback

If the binary is statically linked or the `_start` is unusual, use the **call graph**:

- Find `__libc_start_main` (or `__uClibc_main` on uClibc - the TELESCREEN device uses
  uClibc). Its **first argument is `main`**.
- In a stripped binary the import is still named (`.dynsym` survives), so
  `readelf -s teled | grep -E '_start|uClibc_main'` finds the call, and the
  register feeding it is `main`.

Real confirmation on the TELESCREEN device:

```bash
readelf -s teled | grep -E '_start|__uClibc_main'
#   _start
#   __uClibc_main
```

Following `_start` -> `__uClibc_main(r0)` -> the word at the pool -> **`main = 0x19cb8`**.

### The complete `main`-finding decision tree

```
Is it bare metal?
  YES -> decode vector[0] (0xEA branch) -> reset handler -> crt0 -> main
  NO  -> ELF entry = _start
          -> _start loads a function pointer into r0
          -> that pointer is main
          OR
          -> _start branches to __libc_start_main / __uClibc_main
          -> its FIRST ARGUMENT is main
          OR
          -> no start files? search for the libc init call and follow its arg
```

---

## Part 6: Mapping to the Raspberry Pi 5 (Cortex-A76)

The RP5 is a Cortex-A76 (ARMv8-A). The same rules apply, with one extra stage:

```
RP5 power on
  -> VideoCore firmware (closed, like the TELESCREEN BootROM)
  -> loads kernel8.img  (this is YOUR entry point; make it U-Boot)
  -> U-Boot reads the four partitions
  -> loads and inflates the kernel container
  -> jumps to the Linux Image
  -> Linux _start -> start_kernel -> init -> your rootfs -> your app's main
```

To find the reset handler and `main` on RP5:

- **The "reset handler" you control is U-Boot's entry** (`_start` in
  `arch/arm/lib/vectors.S`). Disassemble `u-boot.bin` at its entry and you will see the
  same `0xEA` branch table.
- **The kernel entry** is the first bytes of the inflated `Image` - the ARM
  `head.S` prologue (`add r9, pc, #4; orr r9, r9, #1; bx r9`).
- **`main` in your rootfs application** is found exactly as in Part 5 - `_start` into
  `__libc_start_main`.

> **The RP5 gives you the same four-partition firmware shape as the TELESCREEN
> device**, so every technique in this week applies unchanged.

---

## Part 7: The Tool Cookbook

```bash
# 1. What is this file?
file boot.img ; readelf -h app

# 2. Hex at offset zero (find the vector table)
xxd -l 32 boot.img

# 3. Decode an ARM branch word
python3 - <<'PY'
w = 0xEA000515
off = w & 0xFFFFFF
if off & 0x800000: off -= 0x1000000
print(hex(0x08 + off * 4))   # -> 0x145c
PY

# 4. Disassemble bare metal
arm-none-eabi-objdump -D -b binary -m arm boot.img | head

# 5. Find the ELF entry
readelf -h app | grep Entry

# 6. Disassemble from the entry
arm-none-eabi-objdump -d app --start-address=0x10e64

# 7. Read a literal pool
arm-none-eabi-objdump -s -j .text app | grep -A2 10e90

# 8. Find surviving symbols in a stripped binary
readelf -s app | grep -E '_start|__libc_start_main|__uClibc_main'

# 9. Ghidra
#    Import -> ARM:LE:32:v7 -> auto-analyze -> go to entry -> follow r0

# 10. radare2
r2 -a arm -b 32 -q -c 'aaa; s entry0; pd 20' app
```

---

## Part 8: Labs

### Lab 1 - Decode three branch words (warm-up)

Given `0xEA000515`, `0xEAFFFFFE`, and `0xEA000004`, compute each target address by hand.
Which one is an infinite loop?

### Lab 2 - Find the reset handler

Carve `boot.img` from the TELESCREEN image. Read the first 32 bytes. Decode `vector[0]`.
Disassemble the reset handler and list the first three things it does.

### Lab 3 - Find `main` without symbols

Using `teled` (stripped), find the entry point, disassemble `_start`, resolve the
literal pool, and report the address of `main`. Confirm it in Ghidra by following the
first argument of `__uClibc_main`.

### Lab 4 - Prove the chain

Write a one-page trace: `power on -> vector[0] -> reset handler -> crt0 -> main ->
_start -> __libc_start_main -> your app`. Cite the exact bytes and addresses at every
step.

### Lab 5 - Do it on RP5

Build U-Boot for the RP5, find its vector table, decode its reset branch, and then find
`main` in your own rootfs application. Show both with bytes.

---

## Part 9: Reference

### Cortex-A exception offsets

| offset | exception | notes |
|--------|-----------|-------|
| `0x00` | Reset | **this is the reset handler branch** |
| `0x04` | Undefined instruction | |
| `0x08` | Supervisor call (SVC) | syscalls |
| `0x0C` | Prefetch abort | |
| `0x10` | Data abort | |
| `0x14` | Reserved / Hypervisor | AArch32 HVC on newer cores |
| `0x18` | IRQ | |
| `0x1C` | FIQ | |

### ARM branch decode

```
0xEA000000 | (offset & 0xFFFFFF)   ; offset in words, PC = address + 8
```

### Key registers you will meet

| register | meaning |
|----------|---------|
| `PC` (`r15`) | program counter; reads as `address + 8` in A32 |
| `LR` (`r14`) | link register (return address) |
| `SP` (`r13`) | stack pointer |
| `CPSR` | current program status register (mode + flags) |
| `SCTLR` | system control register (MMU, caches, V bit) |
| `VBAR` | vector base address register (`c12, c0, 0`) |

### Further reading

- ARM Architecture Reference Manual (ARMv7-A / ARMv8-A)
- U-Boot `arch/arm/lib/vectors.S`
- Linux `arch/arm/kernel/head.S` and `arch/arm/lib/` start files
- The TELESCREEN four-partition layout (`README.md`) for the raw images used here

> **The takeaway:** a stripped binary is not a black box. The reset handler announces
> itself with an `0xEA` branch at offset zero, and `main` announces itself as the
> pointer `_start` hands to the C library. Learn those two signatures and you can open
> any Cortex-A firmware ever shipped.
