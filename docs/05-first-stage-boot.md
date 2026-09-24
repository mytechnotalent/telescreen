# Volume 05: First-Stage Boot

**the Cortex-A vector table and the reset handler**

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

## What You'll Learn

- Why a Cortex-A vector table differs from a Cortex-M one
- How to decode an ARM branch opcode by hand
- How to find the reset handler in a **stripped** blob
- Where the RP5's first stage sits and what you actually control

## 1. The One Big Difference

A Cortex-M (Pico 2) vector table holds **addresses**: the hardware *loads* the stack
pointer and the reset address from offsets 0 and 4.

A Cortex-A table holds **branch instructions**: the hardware *executes* the word at
each offset as a branch.

```
offset 0x00: 15 05 00 ea   -> 0xEA000515 -> B 0x145C   ; reset
offset 0x04: fe ff ff ea   -> 0xEAFFFFFE -> B .        ; undefined
offset 0x08: fe ff ff ea   -> 0xEAFFFFFE -> B .        ; svc
offset 0x0C: fe ff ff ea   -> 0xEAFFFFFE -> B .        ; prefetch abort
offset 0x10: fe ff ff ea   -> 0xEAFFFFFE -> B .        ; data abort
offset 0x14: fe ff ff ea   -> 0xEAFFFFFE -> B .        ; reserved
offset 0x18: fe ff ff ea   -> 0xEAFFFFFE -> B .        ; IRQ
offset 0x1C: fe ff ff ea   -> 0xEAFFFFFE -> B .        ; FIQ
```

Seven `EAFFFFFE` words after one real branch is the signature of a Cortex-A vector
table. `B .` is a deliberate infinite loop: an unhandled exception hangs the core.

## 2. Decode an ARM Branch

An A32 branch is `cond=1110` (`AL`) with opcode `1010` and a 24-bit word offset; the
CPU's PC reads as `address + 8`:

```
word   = 0xEA000515
offset = 0x000515          (1301 words)
target = (0x00 + 8) + (1301 * 4)
       = 0x08 + 0x1454
       = 0x145C
```

Do it in one command:

```bash
python3 - <<'PY'
w = 0xEA000515
off = w & 0xFFFFFF
if off & 0x800000: off -= 0x1000000
print(hex(0x08 + off * 4))   # 0x145c
PY
```

## 3. Find It in a Stripped Blob

```bash
xxd -l 32 boot.img
# 00000000: 15 05 00 ea  fe ff ff ea  fe ff ff ea  fe ff ff ea
arm-none-eabi-objdump -D -b binary -m arm boot.img --start-address=0x145c | head
```

No symbols. No debug info. Just bytes. That is the skill.

## 4. What the Reset Handler Does

```asm
0x145C  ldr r0, =0x120280F8   ; an SoC control register
0x1460  mov r1, #1
0x1464  str r1, [r0]          ; poke it
...
0x146C  mrs r0, CPSR          ; read processor state
0x1478  orrne r0, r0, #0x13   ; ensure SVC mode
0x1480  orr r0, r0, #0xc0     ; disable IRQ and FIQ
0x1484  msr CPSR_fc, r0       ; commit
0x1488  mrc p15, 0, r0, c1, c0, 0   ; read SCTLR
0x148C  bic r0, r0, #0x2000   ; clear V (high vectors off)
0x1490  mcr p15, 0, r0, c1, c0, 0   ; write SCTLR
0x1494  mcr p15, 0, r0, c12, c0, 0  ; VBAR = 0
```

That is the entire first-stage contract: set SVC, mask interrupts, disable high
vectors, then jump onward.

## 5. Where the RP5 Actually Starts

On a camera the closed **BootROM** is the true first stage. On an RP5 the closed
**VideoCore firmware** plays that role. The first stage **you control** is **U-Boot**.

```
RP5 diode power -> VideoCore firmware (closed) -> U-Boot (_start, ARM64)
    -> Linux Image -> rootfs -> teled
```

So the skill transfers: on the RP5 you find `_start` in a **stripped U-Boot** the same
way, decode the branch table, and follow it into C. `docs/WEEK12_CORTEX_A_RE.md`
walks through a full worked example, including finding `main` in a stripped Linux
application via `_start -> __libc_start_main -> main`.

## 6. AArch64 vs A32

The RP5 is **AArch64**, so real U-Boot code is 64-bit (`ldp`, `mov x0, ...`). But the
**A32 vector decode above still matters**: older Raspberry Pi boot stages and every
32-bit camera use the `0xEA` branch table, and the CTF's `CTF-XX-boot.img` carries one.
Know both.

## Labs

1. Decode `0xEA000515`, `0xEAFFFFFE`, and `0xEA000004` by hand.
2. Find the reset handler in a stripped `boot.img` and disassemble its first 16 lines.
3. Cross-reference `docs/WEEK12_CORTEX_A_RE.md` and repeat the method on RP5 U-Boot.

## Reference

- `docs/WEEK12_CORTEX_A_RE.md` (the full method)
- ARM Architecture Reference Manual (ARMv7-A / ARMv8-A)
