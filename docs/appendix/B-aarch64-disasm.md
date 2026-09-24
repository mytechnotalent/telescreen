# Appendix B: Full AArch64 Disassembly

The complete AArch64 assembly of every module, generated with
`clang -O2 -S` on an arm64 host (Apple Silicon emits the same AArch64 ISA as the RP5).

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

## `src/crc.c`

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__literal16,16byte_literals
	.p2align	4, 0x0                          ; -- Begin function crc32_le
lCPI0_0:
	.long	8                               ; 0x8
	.long	4                               ; 0x4
	.long	16                              ; 0x10
	.long	32                              ; 0x20
lCPI0_1:
	.long	249268274                       ; 0xedb8832
	.long	124634137                       ; 0x76dc419
	.long	498536548                       ; 0x1db71064
	.long	997073096                       ; 0x3b6e20c8
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_crc32_le
	.p2align	2
_crc32_le:                              ; @crc32_le
	.cfi_startproc
; %bb.0:
	cbz	x2, LBB0_3
; %bb.1:
	mov	w8, #33568                      ; =0x8320
	movk	w8, #60856, lsl #16
Lloh0:
	adrp	x9, lCPI0_0@PAGE
Lloh1:
	ldr	q0, [x9, lCPI0_0@PAGEOFF]
Lloh2:
	adrp	x9, lCPI0_1@PAGE
Lloh3:
	ldr	q1, [x9, lCPI0_1@PAGEOFF]
	mov	w9, #16784                      ; =0x4190
	movk	w9, #30428, lsl #16
LBB0_2:                                 ; =>This Inner Loop Header: Depth=1
	ldrb	w10, [x1], #1
	eor	w10, w0, w10
	sbfx	w11, w10, #0, #1
	and	w11, w11, w8
	eor	w11, w11, w10, lsr #1
	lsl	w12, w10, #30
	and	w12, w8, w12, asr #31
	eor	w12, w12, w11, lsr #1
	dup.4s	v2, w10
	and.16b	v2, v2, v0
	cmeq.4s	v2, v2, #0
	bic.16b	v2, v1, v2
	lsl	w10, w11, #26
	and	w10, w9, w10, asr #31
	lsl	w11, w12, #26
	and	w11, w8, w11, asr #31
	ext.16b	v3, v2, v2, #8
	eor.8b	v2, v2, v3
	fmov	x13, d2
	lsr	x14, x13, #32
	eor	w10, w13, w10
	eor	w10, w10, w14
	eor	w11, w11, w12, lsr #6
	eor	w0, w10, w11
	subs	x2, x2, #1
	b.ne	LBB0_2
LBB0_3:
	ret
	.loh AdrpLdr	Lloh2, Lloh3
	.loh AdrpAdrp	Lloh0, Lloh2
	.loh AdrpLdr	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__literal16,16byte_literals
	.p2align	4, 0x0                          ; -- Begin function crc32_uboot
lCPI1_0:
	.long	8                               ; 0x8
	.long	4                               ; 0x4
	.long	16                              ; 0x10
	.long	32                              ; 0x20
lCPI1_1:
	.long	249268274                       ; 0xedb8832
	.long	124634137                       ; 0x76dc419
	.long	498536548                       ; 0x1db71064
	.long	997073096                       ; 0x3b6e20c8
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_crc32_uboot
	.p2align	2
_crc32_uboot:                           ; @crc32_uboot
	.cfi_startproc
; %bb.0:
	cbz	x1, LBB1_4
; %bb.1:
	mov	w10, #-1                        ; =0xffffffff
	mov	w8, #33568                      ; =0x8320
	movk	w8, #60856, lsl #16
Lloh4:
	adrp	x9, lCPI1_0@PAGE
Lloh5:
	ldr	q0, [x9, lCPI1_0@PAGEOFF]
Lloh6:
	adrp	x9, lCPI1_1@PAGE
Lloh7:
	ldr	q1, [x9, lCPI1_1@PAGEOFF]
	mov	w9, #16784                      ; =0x4190
	movk	w9, #30428, lsl #16
LBB1_2:                                 ; =>This Inner Loop Header: Depth=1
	ldrb	w11, [x0], #1
	eor	w10, w10, w11
	sbfx	w11, w10, #0, #1
	and	w11, w11, w8
	eor	w11, w11, w10, lsr #1
	lsl	w12, w10, #30
	and	w12, w8, w12, asr #31
	eor	w12, w12, w11, lsr #1
	dup.4s	v2, w10
	and.16b	v2, v2, v0
	cmeq.4s	v2, v2, #0
	bic.16b	v2, v1, v2
	lsl	w10, w11, #26
	and	w10, w9, w10, asr #31
	lsl	w11, w12, #26
	and	w11, w8, w11, asr #31
	ext.16b	v3, v2, v2, #8
	eor.8b	v2, v2, v3
	fmov	x13, d2
	lsr	x14, x13, #32
	eor	w10, w13, w10
	eor	w10, w10, w14
	eor	w11, w11, w12, lsr #6
	eor	w10, w10, w11
	subs	x1, x1, #1
	b.ne	LBB1_2
; %bb.3:
	mvn	w0, w10
	ret
LBB1_4:
	mov	w0, #0                          ; =0x0
	ret
	.loh AdrpLdr	Lloh6, Lloh7
	.loh AdrpAdrp	Lloh4, Lloh6
	.loh AdrpLdr	Lloh4, Lloh5
	.cfi_endproc
                                        ; -- End function
	.globl	_crc16_ccitt                    ; -- Begin function crc16_ccitt
	.p2align	2
_crc16_ccitt:                           ; @crc16_ccitt
	.cfi_startproc
; %bb.0:
	mov	w9, #65535                      ; =0xffff
	cbz	x1, LBB2_3
; %bb.1:
	mov	w8, #4129                       ; =0x1021
LBB2_2:                                 ; =>This Inner Loop Header: Depth=1
	ldrb	w10, [x0], #1
	eor	w11, w9, w10, lsl #8
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w11, w9, w10, lsl #9
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w11, w9, w10, lsl #10
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w11, w9, w10, lsl #11
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w11, w9, w10, lsl #12
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w11, w9, w10, lsl #13
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w11, w9, w10, lsl #14
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w10, w9, w10, lsl #15
	sxth	w10, w10
	lsl	w9, w9, #1
	eor	w11, w9, w8
	cmp	w10, #0
	csel	w9, w11, w9, lt
	subs	x1, x1, #1
	b.ne	LBB2_2
LBB2_3:
	and	w0, w9, #0xffff
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```

## `src/partition.c`

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_part_desc_for                  ; -- Begin function part_desc_for
	.p2align	2
_part_desc_for:                         ; @part_desc_for
	.cfi_startproc
; %bb.0:
	sub	w8, w0, #1
	cmp	w8, #3
	b.hi	LBB0_2
; %bb.1:
Lloh0:
	adrp	x9, l_switch.table.part_desc_for@PAGE
Lloh1:
	add	x9, x9, l_switch.table.part_desc_for@PAGEOFF
	ldr	x0, [x9, w8, uxtw #3]
	ret
LBB0_2:
	mov	x0, #0                          ; =0x0
	ret
	.loh AdrpAdd	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	_part_identify                  ; -- Begin function part_identify
	.p2align	2
_part_identify:                         ; @part_identify
	.cfi_startproc
; %bb.0:
	mov	x8, x0
	mov	w0, #0                          ; =0x0
	cbz	x8, LBB1_3
; %bb.1:
	cmp	x1, #4
	b.lo	LBB1_3
; %bb.2:
	ldr	w8, [x8]
	mov	w9, #1                          ; =0x1
	mov	w10, #1301                      ; =0x515
	movk	w10, #59904, lsl #16
	mov	w11, #4                         ; =0x4
	mov	w12, #6533                      ; =0x1985
	sub	w13, w8, #440, lsl #12          ; =1802240
	cmp	w13, #1057
	mov	w13, #2                         ; =0x2
	cinc	w13, w13, eq
	cmp	w12, w8, uxth
	csel	w11, w11, w13, eq
	cmp	w8, w10
	csel	w0, w9, w11, eq
LBB1_3:
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_part_verify_layout             ; -- Begin function part_verify_layout
	.p2align	2
_part_verify_layout:                    ; @part_verify_layout
	.cfi_startproc
; %bb.0:
	lsr	x8, x0, #24
	cmp	x8, #0
	mov	w8, #2                          ; =0x2
	csel	w0, w8, wzr, eq
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_part_carve                     ; -- Begin function part_carve
	.p2align	2
_part_carve:                            ; @part_carve
	.cfi_startproc
; %bb.0:
	cmp	w2, #2
	b.gt	LBB3_4
; %bb.1:
	cmp	w2, #1
	b.eq	LBB3_8
; %bb.2:
	cmp	w2, #2
	b.ne	LBB3_7
; %bb.3:
	mov	w10, #0                         ; =0x0
Lloh2:
	adrp	x9, _g_parts@PAGE+24
Lloh3:
	add	x9, x9, _g_parts@PAGEOFF+24
	mov	w8, #1                          ; =0x1
	cbnz	x0, LBB3_11
	b	LBB3_9
LBB3_4:
	cmp	w2, #3
	b.eq	LBB3_10
; %bb.5:
	cmp	w2, #4
	b.ne	LBB3_7
; %bb.6:
	mov	w10, #0                         ; =0x0
Lloh4:
	adrp	x9, _g_parts@PAGE+72
Lloh5:
	add	x9, x9, _g_parts@PAGEOFF+72
	mov	w8, #1                          ; =0x1
	cbnz	x0, LBB3_11
	b	LBB3_9
LBB3_7:
	mov	x9, #0                          ; =0x0
	mov	w10, #1                         ; =0x1
	mov	w8, #1                          ; =0x1
	cbnz	x0, LBB3_11
	b	LBB3_9
LBB3_8:
	mov	w10, #0                         ; =0x0
Lloh6:
	adrp	x9, _g_parts@PAGE
Lloh7:
	add	x9, x9, _g_parts@PAGEOFF
	mov	w8, #1                          ; =0x1
	cbnz	x0, LBB3_11
LBB3_9:
	mov	x0, x8
	ret
LBB3_10:
	mov	w10, #0                         ; =0x0
Lloh8:
	adrp	x9, _g_parts@PAGE+48
Lloh9:
	add	x9, x9, _g_parts@PAGEOFF+48
	mov	w8, #1                          ; =0x1
	cbz	x0, LBB3_9
LBB3_11:
	cbz	x3, LBB3_9
; %bb.12:
	lsr	x8, x1, #24
	cmp	x8, #0
	csinc	w10, w10, wzr, ne
	mov	w8, #1                          ; =0x1
	cinc	w8, w8, eq
	tbnz	w10, #0, LBB3_9
; %bb.13:
	ldr	w2, [x9, #8]
	cmp	x4, x2
	b.hs	LBB3_15
; %bb.14:
	mov	w0, #2                          ; =0x2
	ret
LBB3_15:
	stp	x29, x30, [sp, #-16]!           ; 16-byte Folded Spill
	mov	x29, sp
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	ldr	w8, [x9, #4]
	add	x1, x0, x8
	mov	x0, x3
	bl	_memcpy
	mov	w8, #0                          ; =0x0
	ldp	x29, x30, [sp], #16             ; 16-byte Folded Reload
	mov	x0, x8
	ret
	.loh AdrpAdd	Lloh2, Lloh3
	.loh AdrpAdd	Lloh4, Lloh5
	.loh AdrpAdd	Lloh6, Lloh7
	.loh AdrpAdd	Lloh8, Lloh9
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"boot"

l_.str.1:                               ; @.str.1
	.asciz	"bootargs"

l_.str.2:                               ; @.str.2
	.asciz	"kernel"

l_.str.3:                               ; @.str.3
	.asciz	"rootfs"

	.section	__DATA,__const
	.p2align	3, 0x0                          ; @g_parts
_g_parts:
	.long	1                               ; 0x1
	.long	0                               ; 0x0
	.long	131072                          ; 0x20000
	.space	4
	.quad	l_.str
	.long	2                               ; 0x2
	.long	131072                          ; 0x20000
	.long	65536                           ; 0x10000
	.space	4
	.quad	l_.str.1
	.long	3                               ; 0x3
	.long	196608                          ; 0x30000
	.long	1835008                         ; 0x1c0000
	.space	4
	.quad	l_.str.2
	.long	4                               ; 0x4
	.long	2031616                         ; 0x1f0000
	.long	14745600                        ; 0xe10000
	.space	4
	.quad	l_.str.3

	.p2align	3, 0x0                          ; @switch.table.part_desc_for
l_switch.table.part_desc_for:
	.quad	_g_parts
	.quad	_g_parts+24
	.quad	_g_parts+48
	.quad	_g_parts+72

.subsections_via_symbols
```

## `src/env.c`

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_env_compute_crc                ; -- Begin function env_compute_crc
	.p2align	2
_env_compute_crc:                       ; @env_compute_crc
	.cfi_startproc
; %bb.0:
	cbz	x0, LBB0_3
; %bb.1:
	subs	x1, x1, #4
	b.lo	LBB0_3
; %bb.2:
	add	x0, x0, #4
	b	_crc32_uboot
LBB0_3:
	mov	w0, #0                          ; =0x0
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_env_crc_valid                  ; -- Begin function env_crc_valid
	.p2align	2
_env_crc_valid:                         ; @env_crc_valid
	.cfi_startproc
; %bb.0:
	mov	x8, x0
	mov	w0, #0                          ; =0x0
	cbz	x8, LBB1_3
; %bb.1:
	subs	x1, x1, #4
	b.lo	LBB1_3
; %bb.2:
	stp	x20, x19, [sp, #-32]!           ; 16-byte Folded Spill
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	ldr	w19, [x8], #4
	mov	x0, x8
	bl	_crc32_uboot
	cmp	w19, w0
	cset	w0, eq
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp], #32             ; 16-byte Folded Reload
LBB1_3:
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_env_find                       ; -- Begin function env_find
	.p2align	2
_env_find:                              ; @env_find
	.cfi_startproc
; %bb.0:
	stp	x22, x21, [sp, #-48]!           ; 16-byte Folded Spill
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	mov	x20, x0
	mov	x0, #0                          ; =0x0
	cmp	x1, #4
	b.lo	LBB2_10
; %bb.1:
	cbz	x20, LBB2_10
; %bb.2:
	mov	x19, x2
	cbz	x2, LBB2_10
; %bb.3:
	mov	x22, x1
	mov	x0, x19
	bl	_strlen
	add	x8, x0, #5
	cmp	x8, x22
	b.hs	LBB2_9
; %bb.4:
	mov	x21, x0
	sub	x22, x22, #5
	b	LBB2_6
LBB2_5:                                 ;   in Loop: Header=BB2_6 Depth=1
	add	x20, x20, #1
	sub	x22, x22, #1
	cmp	x21, x22
	b.eq	LBB2_9
LBB2_6:                                 ; =>This Inner Loop Header: Depth=1
	add	x8, x20, x21
	ldrb	w8, [x8, #4]
	cmp	w8, #61
	b.ne	LBB2_5
; %bb.7:                                ;   in Loop: Header=BB2_6 Depth=1
	add	x0, x20, #4
	mov	x1, x19
	mov	x2, x21
	bl	_memcmp
	cbnz	w0, LBB2_5
; %bb.8:
	add	x8, x20, x21
	add	x0, x8, #5
	b	LBB2_10
LBB2_9:
	mov	x0, #0                          ; =0x0
LBB2_10:
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp], #48             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```

## `src/container.c`

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_container_magic                ; -- Begin function container_magic
	.p2align	2
_container_magic:                       ; @container_magic
	.cfi_startproc
; %bb.0:
	ldr	w0, [x0]
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_container_length               ; -- Begin function container_length
	.p2align	2
_container_length:                      ; @container_length
	.cfi_startproc
; %bb.0:
	ldr	w0, [x0, #4]
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_container_tag_ok               ; -- Begin function container_tag_ok
	.p2align	2
_container_tag_ok:                      ; @container_tag_ok
	.cfi_startproc
; %bb.0:
	ldr	x8, [x0, #8]
	mov	x9, #31335                      ; =0x7a67
	movk	x9, #28777, lsl #16
	movk	x9, #25960, lsl #32
	movk	x9, #25697, lsl #48
	cmp	x8, x9
	cset	w0, eq
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_container_valid                ; -- Begin function container_valid
	.p2align	2
_container_valid:                       ; @container_valid
	.cfi_startproc
; %bb.0:
	mov	x8, x0
	mov	w0, #0                          ; =0x0
	cbz	x8, LBB3_4
; %bb.1:
	cmp	x1, #16
	b.lo	LBB3_4
; %bb.2:
	ldr	w9, [x8]
	sub	w9, w9, #440, lsl #12           ; =1802240
	cmp	w9, #1057
	b.ne	LBB3_5
; %bb.3:
	ldr	x8, [x8, #8]
	mov	x9, #31335                      ; =0x7a67
	movk	x9, #28777, lsl #16
	movk	x9, #25960, lsl #32
	movk	x9, #25697, lsl #48
	cmp	x8, x9
	cset	w0, eq
LBB3_4:
	ret
LBB3_5:
	mov	w0, #0                          ; =0x0
	ret
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"gziphead"

.subsections_via_symbols
```

## `src/jffs2.c`

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_jffs2_magic                    ; -- Begin function jffs2_magic
	.p2align	2
_jffs2_magic:                           ; @jffs2_magic
	.cfi_startproc
; %bb.0:
	ldrh	w0, [x0]
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_jffs2_type                     ; -- Begin function jffs2_type
	.p2align	2
_jffs2_type:                            ; @jffs2_type
	.cfi_startproc
; %bb.0:
	ldrh	w0, [x0, #2]
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_jffs2_totlen                   ; -- Begin function jffs2_totlen
	.p2align	2
_jffs2_totlen:                          ; @jffs2_totlen
	.cfi_startproc
; %bb.0:
	ldr	w0, [x0, #4]
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_jffs2_hdr_crc_valid            ; -- Begin function jffs2_hdr_crc_valid
	.p2align	2
_jffs2_hdr_crc_valid:                   ; @jffs2_hdr_crc_valid
	.cfi_startproc
; %bb.0:
	stp	x20, x19, [sp, #-32]!           ; 16-byte Folded Spill
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x0
	mov	w0, #0                          ; =0x0
	mov	x1, x19
	mov	w2, #8                          ; =0x8
	bl	_crc32_le
	ldr	w8, [x19, #8]
	cmp	w0, w8
	cset	w0, eq
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp], #32             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```

## `src/aead.c`

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_aead_init                      ; -- Begin function aead_init
	.p2align	2
_aead_init:                             ; @aead_init
	.cfi_startproc
; %bb.0:
	cmp	w0, #1
	b.hi	LBB0_2
; %bb.1:
Lloh0:
	adrp	x8, _g_aead_algo@PAGE
	str	w0, [x8, _g_aead_algo@PAGEOFF]
Lloh1:
	adrp	x8, _g_last_nonce@PAGE
Lloh2:
	add	x8, x8, _g_last_nonce@PAGEOFF
	stp	xzr, xzr, [x8, #8]
	str	xzr, [x8]
	mov	w8, #1                          ; =0x1
	adrp	x9, _g_aead_ready@PAGE
	strb	w8, [x9, _g_aead_ready@PAGEOFF]
LBB0_2:
	cmp	w0, #2
	cset	w0, lo
	ret
	.loh AdrpAdd	Lloh1, Lloh2
	.loh AdrpAdrp	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	_aead_nonce_size                ; -- Begin function aead_nonce_size
	.p2align	2
_aead_nonce_size:                       ; @aead_nonce_size
	.cfi_startproc
; %bb.0:
	adrp	x8, _g_aead_ready@PAGE
	ldrb	w8, [x8, _g_aead_ready@PAGEOFF]
Lloh3:
	adrp	x9, _g_aead_algo@PAGE
Lloh4:
	ldr	w9, [x9, _g_aead_algo@PAGEOFF]
	cmp	w9, #0
	mov	w9, #24                         ; =0x18
	mov	w10, #12                        ; =0xc
	csel	x9, x10, x9, eq
	cmp	w8, #0
	csel	x0, x9, xzr, ne
	ret
	.loh AdrpLdr	Lloh3, Lloh4
	.cfi_endproc
                                        ; -- End function
	.globl	_aead_seal                      ; -- Begin function aead_seal
	.p2align	2
_aead_seal:                             ; @aead_seal
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #128
	stp	x24, x23, [sp, #64]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #80]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #96]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #112]            ; 16-byte Folded Spill
	add	x29, sp, #112
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x23, x0
Lloh5:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh6:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh7:
	ldr	x8, [x8]
	str	x8, [sp, #56]
	mov	w0, #1                          ; =0x1
	cbz	x3, LBB2_24
; %bb.1:
	cbz	x2, LBB2_24
; %bb.2:
	mov	x22, x1
	cbz	x1, LBB2_24
; %bb.3:
	cbz	x23, LBB2_24
; %bb.4:
	adrp	x8, _g_aead_ready@PAGE
	ldrb	w8, [x8, _g_aead_ready@PAGEOFF]
	cbz	w8, LBB2_24
; %bb.5:
	mov	x20, x2
	mov	x19, x3
Lloh8:
	adrp	x9, _g_aead_algo@PAGE
Lloh9:
	ldr	w21, [x9, _g_aead_algo@PAGEOFF]
	cmp	w21, #0
	mov	w9, #24                         ; =0x18
	mov	w10, #12                        ; =0xc
	csel	x9, x10, x9, eq
	cmp	w8, #0
	csel	x2, x9, xzr, ne
Lloh10:
	adrp	x0, _g_last_nonce@PAGE
Lloh11:
	add	x0, x0, _g_last_nonce@PAGEOFF
	mov	x1, x22
	mov	w3, #24                         ; =0x18
	bl	___memcpy_chk
	cbz	w21, LBB2_12
; %bb.6:
	add	x2, sp, #24
	mov	x0, x23
	mov	x1, x22
	bl	_hchacha20
	str	wzr, [sp, #12]
	ldr	x8, [x22, #16]
	stur	x8, [sp, #16]
	bl	_EVP_CIPHER_CTX_new
	cbz	x0, LBB2_23
; %bb.7:
	mov	x21, x0
	bl	_EVP_chacha20_poly1305
	mov	x1, x0
	mov	x0, x21
	mov	x2, #0                          ; =0x0
	mov	x3, #0                          ; =0x0
	mov	x4, #0                          ; =0x0
	bl	_EVP_EncryptInit_ex
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.8:
	mov	x0, x21
	mov	w1, #9                          ; =0x9
	mov	w2, #12                         ; =0xc
	mov	x3, #0                          ; =0x0
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.9:
	add	x3, sp, #24
	add	x4, sp, #12
	mov	x0, x21
	mov	x1, #0                          ; =0x0
	mov	x2, #0                          ; =0x0
	bl	_EVP_EncryptInit_ex
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.10:
	str	wzr, [sp, #8]
	add	x2, sp, #8
	mov	x0, x21
	mov	x1, x19
	mov	x3, x20
	mov	w4, #48                         ; =0x30
	bl	_EVP_EncryptUpdate
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.11:
	str	wzr, [sp, #8]
	add	x1, x19, #48
	add	x2, sp, #8
	b	LBB2_18
LBB2_12:
	bl	_EVP_CIPHER_CTX_new
	cbz	x0, LBB2_23
; %bb.13:
	mov	x21, x0
	bl	_EVP_aes_256_gcm
	mov	x1, x0
	mov	x0, x21
	mov	x2, #0                          ; =0x0
	mov	x3, #0                          ; =0x0
	mov	x4, #0                          ; =0x0
	bl	_EVP_EncryptInit_ex
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.14:
	mov	x0, x21
	mov	w1, #9                          ; =0x9
	mov	w2, #12                         ; =0xc
	mov	x3, #0                          ; =0x0
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.15:
	mov	x0, x21
	mov	x1, #0                          ; =0x0
	mov	x2, #0                          ; =0x0
	mov	x3, x23
	mov	x4, x22
	bl	_EVP_EncryptInit_ex
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.16:
	str	wzr, [sp, #24]
	add	x2, sp, #24
	mov	x0, x21
	mov	x1, x19
	mov	x3, x20
	mov	w4, #48                         ; =0x30
	bl	_EVP_EncryptUpdate
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.17:
	str	wzr, [sp, #24]
	add	x1, x19, #48
	add	x2, sp, #24
LBB2_18:
	mov	x0, x21
	bl	_EVP_EncryptFinal_ex
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.19:
	add	x3, x19, #48
	mov	x0, x21
	mov	w1, #16                         ; =0x10
	mov	w2, #16                         ; =0x10
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.20:
	mov	w19, #0                         ; =0x0
	b	LBB2_22
LBB2_21:
	mov	w19, #3                         ; =0x3
LBB2_22:
	mov	x0, x21
	bl	_EVP_CIPHER_CTX_free
	mov	x0, x19
	b	LBB2_24
LBB2_23:
	mov	w0, #3                          ; =0x3
LBB2_24:
	ldr	x8, [sp, #56]
Lloh12:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh13:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh14:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB2_26
; %bb.25:
	ldp	x29, x30, [sp, #112]            ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #96]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #80]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #64]             ; 16-byte Folded Reload
	add	sp, sp, #128
	ret
LBB2_26:
	bl	___stack_chk_fail
	.loh AdrpLdrGotLdr	Lloh5, Lloh6, Lloh7
	.loh AdrpAdd	Lloh10, Lloh11
	.loh AdrpLdr	Lloh8, Lloh9
	.loh AdrpLdrGotLdr	Lloh12, Lloh13, Lloh14
	.cfi_endproc
                                        ; -- End function
	.globl	_aead_open                      ; -- Begin function aead_open
	.p2align	2
_aead_open:                             ; @aead_open
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #128
	stp	x24, x23, [sp, #64]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #80]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #96]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #112]            ; 16-byte Folded Spill
	add	x29, sp, #112
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x23, x0
Lloh15:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh16:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh17:
	ldr	x8, [x8]
	str	x8, [sp, #56]
	mov	w0, #1                          ; =0x1
	cbz	x3, LBB3_25
; %bb.1:
	cbz	x2, LBB3_25
; %bb.2:
	cbz	x1, LBB3_25
; %bb.3:
	cbz	x23, LBB3_25
; %bb.4:
	adrp	x8, _g_aead_ready@PAGE
	ldrb	w8, [x8, _g_aead_ready@PAGEOFF]
	cbz	w8, LBB3_25
; %bb.5:
	mov	x19, x3
	mov	x20, x2
Lloh18:
	adrp	x8, _g_aead_algo@PAGE
Lloh19:
	ldr	w8, [x8, _g_aead_algo@PAGEOFF]
	cbz	w8, LBB3_13
; %bb.6:
	add	x2, sp, #24
	mov	x0, x23
	mov	x21, x1
	bl	_hchacha20
	str	wzr, [sp, #12]
	ldr	x8, [x21, #16]
	stur	x8, [sp, #16]
	bl	_EVP_CIPHER_CTX_new
	cbz	x0, LBB3_24
; %bb.7:
	mov	x21, x0
	bl	_EVP_chacha20_poly1305
	mov	x1, x0
	mov	x0, x21
	mov	x2, #0                          ; =0x0
	mov	x3, #0                          ; =0x0
	mov	x4, #0                          ; =0x0
	bl	_EVP_DecryptInit_ex
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.8:
	mov	x0, x21
	mov	w1, #9                          ; =0x9
	mov	w2, #12                         ; =0xc
	mov	x3, #0                          ; =0x0
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.9:
	add	x3, sp, #24
	add	x4, sp, #12
	mov	x0, x21
	mov	x1, #0                          ; =0x0
	mov	x2, #0                          ; =0x0
	bl	_EVP_DecryptInit_ex
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.10:
	str	wzr, [sp, #8]
	add	x2, sp, #8
	mov	x0, x21
	mov	x1, x19
	mov	x3, x20
	mov	w4, #48                         ; =0x30
	bl	_EVP_DecryptUpdate
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.11:
	ldrsw	x22, [sp, #8]
	add	x3, x20, #48
	mov	x0, x21
	mov	w1, #17                         ; =0x11
	mov	w2, #16                         ; =0x10
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.12:
	add	x1, x19, x22
	add	x2, sp, #8
	b	LBB3_20
LBB3_13:
	mov	x22, x1
	bl	_EVP_CIPHER_CTX_new
	cbz	x0, LBB3_24
; %bb.14:
	mov	x21, x0
	bl	_EVP_aes_256_gcm
	mov	x1, x0
	mov	x0, x21
	mov	x2, #0                          ; =0x0
	mov	x3, #0                          ; =0x0
	mov	x4, #0                          ; =0x0
	bl	_EVP_DecryptInit_ex
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.15:
	mov	x0, x21
	mov	w1, #9                          ; =0x9
	mov	w2, #12                         ; =0xc
	mov	x3, #0                          ; =0x0
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.16:
	mov	x0, x21
	mov	x1, #0                          ; =0x0
	mov	x2, #0                          ; =0x0
	mov	x3, x23
	mov	x4, x22
	bl	_EVP_DecryptInit_ex
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.17:
	str	wzr, [sp, #24]
	add	x2, sp, #24
	mov	x0, x21
	mov	x1, x19
	mov	x3, x20
	mov	w4, #48                         ; =0x30
	bl	_EVP_DecryptUpdate
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.18:
	ldrsw	x22, [sp, #24]
	add	x3, x20, #48
	mov	x0, x21
	mov	w1, #17                         ; =0x11
	mov	w2, #16                         ; =0x10
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.19:
	add	x1, x19, x22
	add	x2, sp, #24
LBB3_20:
	mov	x0, x21
	bl	_EVP_DecryptFinal_ex
	mov	x20, x0
	mov	x0, x21
	bl	_EVP_CIPHER_CTX_free
	cmp	w20, #1
	b.ne	LBB3_23
; %bb.21:
	mov	w0, #0                          ; =0x0
	b	LBB3_25
LBB3_22:
	mov	x0, x21
	bl	_EVP_CIPHER_CTX_free
LBB3_23:
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [x19, #16]
	str	q0, [x19]
	mov	w0, #2                          ; =0x2
	b	LBB3_25
LBB3_24:
	mov	w0, #3                          ; =0x3
LBB3_25:
	ldr	x8, [sp, #56]
Lloh20:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh21:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh22:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB3_27
; %bb.26:
	ldp	x29, x30, [sp, #112]            ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #96]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #80]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #64]             ; 16-byte Folded Reload
	add	sp, sp, #128
	ret
LBB3_27:
	bl	___stack_chk_fail
	.loh AdrpLdrGotLdr	Lloh15, Lloh16, Lloh17
	.loh AdrpLdr	Lloh18, Lloh19
	.loh AdrpLdrGotLdr	Lloh20, Lloh21, Lloh22
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function hchacha20
_hchacha20:                             ; @hchacha20
	.cfi_startproc
; %bb.0:
	stp	x20, x19, [sp, #-16]!           ; 16-byte Folded Spill
	.cfi_def_cfa_offset 16
	.cfi_offset w19, -8
	.cfi_offset w20, -16
	mov	w9, #0                          ; =0x0
	ldp	w19, w11, [x0]
	ldp	w15, w16, [x0, #8]
	ldp	w6, w7, [x0, #16]
	ldp	w10, w12, [x0, #24]
	ldp	w14, w13, [x1]
	mov	w0, #25972                      ; =0x6574
	movk	w0, #27424, lsl #16
	mov	w3, #11570                      ; =0x2d32
	movk	w3, #31074, lsl #16
	mov	w4, #25710                      ; =0x646e
	movk	w4, #13088, lsl #16
	mov	w5, #30821                      ; =0x7865
	movk	w5, #24944, lsl #16
	ldp	w17, w8, [x1, #8]
LBB4_1:                                 ; =>This Inner Loop Header: Depth=1
	add	w1, w5, w19
	eor	w14, w14, w1
	ror	w14, w14, #16
	add	w5, w14, w6
	eor	w6, w5, w19
	ror	w19, w6, #20
	add	w20, w19, w1
	eor	w14, w20, w14
	ror	w14, w14, #24
	add	w6, w14, w5
	eor	w1, w6, w19
	ror	w1, w1, #25
	add	w4, w4, w11
	eor	w13, w13, w4
	ror	w13, w13, #16
	add	w5, w13, w7
	eor	w11, w5, w11
	ror	w11, w11, #20
	add	w4, w11, w4
	eor	w13, w4, w13
	ror	w13, w13, #24
	add	w7, w13, w5
	eor	w11, w7, w11
	ror	w11, w11, #25
	add	w3, w3, w15
	eor	w17, w17, w3
	ror	w17, w17, #16
	add	w10, w17, w10
	eor	w15, w10, w15
	ror	w15, w15, #20
	add	w3, w15, w3
	eor	w17, w3, w17
	ror	w17, w17, #24
	add	w10, w17, w10
	eor	w15, w10, w15
	ror	w15, w15, #25
	add	w0, w0, w16
	eor	w8, w8, w0
	ror	w8, w8, #16
	add	w12, w8, w12
	eor	w16, w12, w16
	ror	w16, w16, #20
	add	w0, w16, w0
	eor	w8, w0, w8
	ror	w8, w8, #24
	add	w12, w8, w12
	eor	w16, w12, w16
	ror	w16, w16, #25
	add	w5, w11, w20
	eor	w8, w8, w5
	ror	w8, w8, #16
	add	w10, w8, w10
	eor	w11, w10, w11
	ror	w11, w11, #20
	add	w5, w11, w5
	eor	w8, w5, w8
	ror	w8, w8, #24
	add	w10, w8, w10
	eor	w11, w10, w11
	ror	w11, w11, #25
	add	w4, w15, w4
	eor	w14, w4, w14
	ror	w14, w14, #16
	add	w12, w14, w12
	eor	w15, w12, w15
	ror	w15, w15, #20
	add	w4, w15, w4
	eor	w14, w4, w14
	ror	w14, w14, #24
	add	w12, w14, w12
	eor	w15, w12, w15
	ror	w15, w15, #25
	add	w3, w16, w3
	eor	w13, w3, w13
	ror	w13, w13, #16
	add	w6, w13, w6
	eor	w16, w6, w16
	ror	w16, w16, #20
	add	w3, w16, w3
	eor	w13, w3, w13
	ror	w13, w13, #24
	add	w6, w13, w6
	eor	w16, w6, w16
	ror	w16, w16, #25
	add	w0, w0, w1
	eor	w17, w0, w17
	ror	w17, w17, #16
	add	w7, w17, w7
	eor	w1, w7, w1
	ror	w1, w1, #20
	add	w0, w1, w0
	eor	w17, w0, w17
	ror	w17, w17, #24
	add	w7, w17, w7
	eor	w1, w7, w1
	ror	w19, w1, #25
	cmp	w9, #9
	add	w9, w9, #1
	b.lo	LBB4_1
; %bb.2:
	strb	w5, [x2]
	lsr	w9, w5, #8
	strb	w9, [x2, #1]
	lsr	w9, w5, #16
	strb	w9, [x2, #2]
	lsr	w9, w5, #24
	strb	w9, [x2, #3]
	strb	w4, [x2, #4]
	lsr	w9, w4, #8
	strb	w9, [x2, #5]
	lsr	w9, w4, #16
	strb	w9, [x2, #6]
	lsr	w9, w4, #24
	strb	w9, [x2, #7]
	strb	w3, [x2, #8]
	lsr	w9, w3, #8
	strb	w9, [x2, #9]
	lsr	w9, w3, #16
	strb	w9, [x2, #10]
	lsr	w9, w3, #24
	strb	w9, [x2, #11]
	strb	w0, [x2, #12]
	lsr	w9, w0, #8
	strb	w9, [x2, #13]
	lsr	w9, w0, #16
	strb	w9, [x2, #14]
	lsr	w9, w0, #24
	strb	w9, [x2, #15]
	strb	w14, [x2, #16]
	lsr	w9, w14, #8
	strb	w9, [x2, #17]
	lsr	w9, w14, #16
	strb	w9, [x2, #18]
	lsr	w9, w14, #24
	strb	w9, [x2, #19]
	strb	w13, [x2, #20]
	lsr	w9, w13, #8
	strb	w9, [x2, #21]
	lsr	w9, w13, #16
	strb	w9, [x2, #22]
	lsr	w9, w13, #24
	strb	w9, [x2, #23]
	strb	w17, [x2, #24]
	lsr	w9, w17, #8
	strb	w9, [x2, #25]
	lsr	w9, w17, #16
	strb	w9, [x2, #26]
	lsr	w9, w17, #24
	strb	w9, [x2, #27]
	strb	w8, [x2, #28]
	lsr	w9, w8, #8
	strb	w9, [x2, #29]
	lsr	w9, w8, #16
	strb	w9, [x2, #30]
	lsr	w8, w8, #24
	strb	w8, [x2, #31]
	ldp	x20, x19, [sp], #16             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
.zerofill __DATA,__bss,_g_aead_algo,4,2 ; @g_aead_algo
.zerofill __DATA,__bss,_g_last_nonce,24,0 ; @g_last_nonce
.zerofill __DATA,__bss,_g_aead_ready,1,0 ; @g_aead_ready
.subsections_via_symbols
```

## `src/beacon.c`

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_beacon_weak_key                ; -- Begin function beacon_weak_key
	.p2align	2
_beacon_weak_key:                       ; @beacon_weak_key
	.cfi_startproc
; %bb.0:
	stp	x22, x21, [sp, #-48]!           ; 16-byte Folded Spill
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	mov	x19, x1
	mov	x20, x0
	bl	_strlen
	mov	x21, x0
	mov	w0, #0                          ; =0x0
	mov	x1, x20
	mov	x2, x21
	bl	_crc32_le
	mov	x22, #0                         ; =0x0
LBB0_1:                                 ; =>This Inner Loop Header: Depth=1
	mov	x1, x20
	mov	x2, x21
	bl	_crc32_le
	strb	w0, [x19, x22]
	add	x22, x22, #1
	cmp	x22, #32
	b.ne	LBB0_1
; %bb.2:
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp], #48             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_beacon_nonce                   ; -- Begin function beacon_nonce
	.p2align	2
_beacon_nonce:                          ; @beacon_nonce
	.cfi_startproc
; %bb.0:
	strb	w0, [x1]
	lsr	w8, w0, #8
	strb	w8, [x1, #1]
	lsr	w8, w0, #16
	strb	w8, [x1, #2]
	lsr	w8, w0, #24
	strb	w8, [x1, #3]
	stur	xzr, [x1, #12]
	stur	xzr, [x1, #4]
	str	wzr, [x1, #20]
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_beacon_seal                    ; -- Begin function beacon_seal
	.p2align	2
_beacon_seal:                           ; @beacon_seal
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
Lloh0:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh1:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh2:
	ldr	x8, [x8]
	stur	x8, [x29, #-8]
	strb	w1, [sp]
	lsr	w8, w1, #8
	strb	w8, [sp, #1]
	lsr	w8, w1, #16
	strb	w8, [sp, #2]
	lsr	w8, w1, #24
	strb	w8, [sp, #3]
	stur	xzr, [sp, #12]
	stur	xzr, [sp, #4]
	str	wzr, [sp, #20]
	mov	x1, sp
	bl	_aead_seal
	cmp	w0, #0
	cset	w0, eq
	ldur	x8, [x29, #-8]
Lloh3:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh4:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh5:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB2_2
; %bb.1:
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
LBB2_2:
	bl	___stack_chk_fail
	.loh AdrpLdrGotLdr	Lloh3, Lloh4, Lloh5
	.loh AdrpLdrGotLdr	Lloh0, Lloh1, Lloh2
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```

## `src/kex.c`

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_kex_hkdf                       ; -- Begin function kex_hkdf
	.p2align	2
_kex_hkdf:                              ; @kex_hkdf
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #304
	stp	x28, x27, [sp, #208]            ; 16-byte Folded Spill
	stp	x26, x25, [sp, #224]            ; 16-byte Folded Spill
	stp	x24, x23, [sp, #240]            ; 16-byte Folded Spill
	stp	x22, x21, [sp, #256]            ; 16-byte Folded Spill
	stp	x20, x19, [sp, #272]            ; 16-byte Folded Spill
	stp	x29, x30, [sp, #288]            ; 16-byte Folded Spill
	add	x29, sp, #288
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	.cfi_offset w25, -72
	.cfi_offset w26, -80
	.cfi_offset w27, -88
	.cfi_offset w28, -96
	mov	x19, x7
	mov	x20, x6
	mov	x21, x5
	mov	x22, x4
	mov	x23, x3
	mov	x24, x2
	mov	x25, x1
	mov	x26, x0
Lloh0:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh1:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh2:
	ldr	x8, [x8]
	stur	x8, [x29, #-96]
	str	wzr, [sp, #80]
	bl	_EVP_sha256
	add	x5, sp, #16
	add	x6, sp, #80
	mov	x1, x24
	mov	x2, x23
	mov	x3, x26
	mov	x4, x25
	bl	_HMAC
	cbz	x0, LBB0_7
; %bb.1:
	mov	w25, #1                         ; =0x1
	cbz	x19, LBB0_8
; %bb.2:
	add	x23, sp, #80
	add	x0, sp, #80
	mov	x1, x22
	mov	x2, x21
	mov	w3, #97                         ; =0x61
	bl	___memcpy_chk
	strb	w25, [x23, x21]
	str	wzr, [sp, #12]
	bl	_EVP_sha256
	add	x1, sp, #16
	add	x3, sp, #80
	add	x4, x21, #1
	add	x5, sp, #48
	add	x6, sp, #12
	mov	w2, #32                         ; =0x20
	bl	_HMAC
	cbz	x0, LBB0_7
; %bb.3:
	mov	w8, #32                         ; =0x20
	cmp	x19, #32
	csel	x23, x19, x8, lo
	add	x1, sp, #48
	mov	x0, x20
	mov	x2, x23
	bl	_memcpy
	cmp	x19, #33
	b.lo	LBB0_8
; %bb.4:
	add	x8, sp, #80
	add	x24, x8, #32
	mov	w27, #2                         ; =0x2
	mov	w28, #32                        ; =0x20
LBB0_5:                                 ; =>This Inner Loop Header: Depth=1
	ldp	q0, q1, [sp, #48]
	stp	q0, q1, [sp, #80]
	mov	x0, x24
	mov	x1, x22
	mov	x2, x21
	mov	w3, #65                         ; =0x41
	bl	___memcpy_chk
	strb	w27, [x24, x21]
	str	wzr, [sp, #12]
	bl	_EVP_sha256
	add	x1, sp, #16
	add	x3, sp, #80
	add	x4, x21, #33
	add	x5, sp, #48
	add	x6, sp, #12
	mov	w2, #32                         ; =0x20
	bl	_HMAC
	cmp	x0, #0
	cset	w25, ne
	cbz	x0, LBB0_8
; %bb.6:                                ;   in Loop: Header=BB0_5 Depth=1
	sub	x8, x19, x23
	cmp	x8, #32
	csel	x26, x8, x28, lo
	add	x0, x20, x23
	add	x1, sp, #48
	mov	x2, x26
	bl	_memcpy
	add	w27, w27, #1
	add	x23, x26, x23
	cmp	x23, x19
	b.lo	LBB0_5
	b	LBB0_8
LBB0_7:
	mov	w25, #0                         ; =0x0
LBB0_8:
	ldur	x8, [x29, #-96]
Lloh3:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh4:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh5:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB0_10
; %bb.9:
	mov	x0, x25
	ldp	x29, x30, [sp, #288]            ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #272]            ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #256]            ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #240]            ; 16-byte Folded Reload
	ldp	x26, x25, [sp, #224]            ; 16-byte Folded Reload
	ldp	x28, x27, [sp, #208]            ; 16-byte Folded Reload
	add	sp, sp, #304
	ret
LBB0_10:
	bl	___stack_chk_fail
	.loh AdrpLdrGotLdr	Lloh0, Lloh1, Lloh2
	.loh AdrpLdrGotLdr	Lloh3, Lloh4, Lloh5
	.cfi_endproc
                                        ; -- End function
	.globl	_kex_x25519                     ; -- Begin function kex_x25519
	.p2align	2
_kex_x25519:                            ; @kex_x25519
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #64
	stp	x22, x21, [sp, #16]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #32]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	mov	x21, x2
	mov	x20, x1
	mov	x2, x0
	mov	w0, #1034                       ; =0x40a
	mov	x1, #0                          ; =0x0
	mov	w3, #32                         ; =0x20
	bl	_EVP_PKEY_new_raw_private_key
	mov	x19, x0
	mov	w0, #1034                       ; =0x40a
	mov	x1, #0                          ; =0x0
	mov	x2, x20
	mov	w3, #32                         ; =0x20
	bl	_EVP_PKEY_new_raw_public_key
	mov	x20, x0
	cbz	x19, LBB1_6
; %bb.1:
	cbz	x20, LBB1_6
; %bb.2:
	mov	x0, x19
	mov	x1, #0                          ; =0x0
	bl	_EVP_PKEY_CTX_new
	mov	w8, #32                         ; =0x20
	str	x8, [sp, #8]
	cbz	x0, LBB1_8
; %bb.3:
	mov	x22, x0
	bl	_EVP_PKEY_derive_init
	cmp	w0, #1
	b.ne	LBB1_7
; %bb.4:
	mov	x0, x22
	mov	x1, x20
	bl	_EVP_PKEY_derive_set_peer
	cmp	w0, #1
	b.ne	LBB1_7
; %bb.5:
	add	x2, sp, #8
	mov	x0, x22
	mov	x1, x21
	bl	_EVP_PKEY_derive
	mov	x8, x0
	mov	x0, x22
	cmp	w8, #1
	cset	w21, eq
	b	LBB1_9
LBB1_6:
	mov	x0, #0                          ; =0x0
	mov	w21, #0                         ; =0x0
	mov	w8, #32                         ; =0x20
	str	x8, [sp, #8]
	b	LBB1_9
LBB1_7:
	mov	w21, #0                         ; =0x0
	mov	x0, x22
	b	LBB1_9
LBB1_8:
	mov	w21, #0                         ; =0x0
LBB1_9:
	bl	_EVP_PKEY_CTX_free
	mov	x0, x19
	bl	_EVP_PKEY_free
	mov	x0, x20
	bl	_EVP_PKEY_free
	ldr	x8, [sp, #8]
	cmp	x8, #32
	csel	w0, wzr, w21, ne
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #64
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```

## `src/identity.c`

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_identity_pubkey                ; -- Begin function identity_pubkey
	.p2align	2
_identity_pubkey:                       ; @identity_pubkey
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x1
	mov	x2, x0
	mov	w20, #32                        ; =0x20
	mov	w0, #1087                       ; =0x43f
	mov	x1, #0                          ; =0x0
	mov	w3, #32                         ; =0x20
	bl	_EVP_PKEY_new_raw_private_key
	str	x20, [sp, #8]
	cbz	x0, LBB0_2
; %bb.1:
	add	x2, sp, #8
	mov	x20, x0
	mov	x1, x19
	bl	_EVP_PKEY_get_raw_public_key
	mov	x8, x0
	mov	x0, x20
	cmp	w8, #1
	cset	w19, eq
	b	LBB0_3
LBB0_2:
	mov	w19, #0                         ; =0x0
LBB0_3:
	bl	_EVP_PKEY_free
	ldr	x8, [sp, #8]
	cmp	x8, #32
	csel	w0, wzr, w19, ne
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_identity_sign                  ; -- Begin function identity_sign
	.p2align	2
_identity_sign:                         ; @identity_sign
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #80
	stp	x24, x23, [sp, #16]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #32]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #48]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #64]             ; 16-byte Folded Spill
	add	x29, sp, #64
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x21, x3
	mov	x20, x2
	mov	x22, x1
	mov	x2, x0
	mov	w0, #1087                       ; =0x43f
	mov	x1, #0                          ; =0x0
	mov	w3, #32                         ; =0x20
	bl	_EVP_PKEY_new_raw_private_key
	mov	x19, x0
	bl	_EVP_MD_CTX_new
	mov	w23, #0                         ; =0x0
	mov	w8, #64                         ; =0x40
	str	x8, [sp, #8]
	cbz	x19, LBB1_5
; %bb.1:
	cbz	x0, LBB1_5
; %bb.2:
	mov	x24, x0
	mov	x1, #0                          ; =0x0
	mov	x2, #0                          ; =0x0
	mov	x3, #0                          ; =0x0
	mov	x4, x19
	bl	_EVP_DigestSignInit
	cmp	w0, #1
	b.ne	LBB1_4
; %bb.3:
	add	x2, sp, #8
	mov	x0, x24
	mov	x1, x21
	mov	x3, x22
	mov	x4, x20
	bl	_EVP_DigestSign
	mov	x8, x0
	mov	x0, x24
	cmp	w8, #1
	cset	w23, eq
	b	LBB1_5
LBB1_4:
	mov	w23, #0                         ; =0x0
	mov	x0, x24
LBB1_5:
	bl	_EVP_MD_CTX_free
	mov	x0, x19
	bl	_EVP_PKEY_free
	ldr	x8, [sp, #8]
	cmp	x8, #64
	csel	w0, wzr, w23, ne
	ldp	x29, x30, [sp, #64]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #48]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #32]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #80
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_identity_verify                ; -- Begin function identity_verify
	.p2align	2
_identity_verify:                       ; @identity_verify
	.cfi_startproc
; %bb.0:
	stp	x24, x23, [sp, #-64]!           ; 16-byte Folded Spill
	stp	x22, x21, [sp, #16]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #32]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x22, x3
	mov	x20, x2
	mov	x21, x1
	mov	x2, x0
	mov	w0, #1087                       ; =0x43f
	mov	x1, #0                          ; =0x0
	mov	w3, #32                         ; =0x20
	bl	_EVP_PKEY_new_raw_public_key
	mov	x19, x0
	bl	_EVP_MD_CTX_new
	mov	w23, #0                         ; =0x0
	cbz	x19, LBB2_5
; %bb.1:
	cbz	x0, LBB2_5
; %bb.2:
	mov	x24, x0
	mov	x1, #0                          ; =0x0
	mov	x2, #0                          ; =0x0
	mov	x3, #0                          ; =0x0
	mov	x4, x19
	bl	_EVP_DigestVerifyInit
	cmp	w0, #1
	b.ne	LBB2_4
; %bb.3:
	mov	x0, x24
	mov	x1, x22
	mov	w2, #64                         ; =0x40
	mov	x3, x21
	mov	x4, x20
	bl	_EVP_DigestVerify
	mov	x8, x0
	mov	x0, x24
	cmp	w8, #1
	cset	w23, eq
	b	LBB2_5
LBB2_4:
	mov	w23, #0                         ; =0x0
	mov	x0, x24
LBB2_5:
	bl	_EVP_MD_CTX_free
	mov	x0, x19
	bl	_EVP_PKEY_free
	mov	x0, x23
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp], #64             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```

## `src/collector.c`

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_collector_init                 ; -- Begin function collector_init
	.p2align	2
_collector_init:                        ; @collector_init
	.cfi_startproc
; %bb.0:
Lloh0:
	adrp	x8, _g_stats@PAGE
Lloh1:
	add	x8, x8, _g_stats@PAGEOFF
	str	xzr, [x8]
	str	wzr, [x8, #8]
	ret
	.loh AdrpAdd	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	_collector_accept               ; -- Begin function collector_accept
	.p2align	2
_collector_accept:                      ; @collector_accept
	.cfi_startproc
; %bb.0:
	cmp	x0, #0
	mov	w8, #64                         ; =0x40
	ccmp	x1, x8, #0, ne
	cset	w0, eq
	b.ne	LBB1_2
; %bb.1:
Lloh2:
	adrp	x8, _g_stats@PAGE
Lloh3:
	add	x8, x8, _g_stats@PAGEOFF
	ldr	w9, [x8]
	add	w9, w9, #1
	str	w9, [x8]
	str	w2, [x8, #8]
	ret
LBB1_2:
	adrp	x8, _g_stats@PAGE+4
	ldr	w9, [x8, _g_stats@PAGEOFF+4]
	add	w9, w9, #1
	str	w9, [x8, _g_stats@PAGEOFF+4]
	ret
	.loh AdrpAdd	Lloh2, Lloh3
	.cfi_endproc
                                        ; -- End function
	.globl	_collector_stats                ; -- Begin function collector_stats
	.p2align	2
_collector_stats:                       ; @collector_stats
	.cfi_startproc
; %bb.0:
Lloh4:
	adrp	x0, _g_stats@PAGE
Lloh5:
	add	x0, x0, _g_stats@PAGEOFF
	ret
	.loh AdrpAdd	Lloh4, Lloh5
	.cfi_endproc
                                        ; -- End function
.zerofill __DATA,__bss,_g_stats,12,3    ; @g_stats
.subsections_via_symbols
```

## `src/teled.c`

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_teled_init                     ; -- Begin function teled_init
	.p2align	2
_teled_init:                            ; @teled_init
	.cfi_startproc
; %bb.0:
	stp	x20, x19, [sp, #-32]!           ; 16-byte Folded Spill
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x1
	mov	x1, x0
Lloh0:
	adrp	x0, _g_teled@PAGE
Lloh1:
	add	x0, x0, _g_teled@PAGEOFF
	str	xzr, [x0, #64]
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [x0, #32]
	stp	q0, q0, [x0]
	mov	w2, #31                         ; =0x1f
	bl	_strncpy
	ldp	q0, q1, [x19]
	stp	q0, q1, [x0, #32]
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp], #32             ; 16-byte Folded Reload
	b	_collector_init
	.loh AdrpAdd	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	_teled_beacon                   ; -- Begin function teled_beacon
	.p2align	2
_teled_beacon:                          ; @teled_beacon
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #96
	stp	x29, x30, [sp, #80]             ; 16-byte Folded Spill
	add	x29, sp, #80
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	mov	x2, x0
Lloh2:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh3:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh4:
	ldr	x8, [x8]
	stur	x8, [x29, #-8]
Lloh5:
	adrp	x0, _g_teled@PAGE+32
Lloh6:
	add	x0, x0, _g_teled@PAGEOFF+32
	ldr	w8, [x0, #32]
	add	w1, w8, #1
	str	w1, [x0, #32]
	add	x3, sp, #8
	bl	_beacon_seal
	cbz	w0, LBB1_2
; %bb.1:
Lloh7:
	adrp	x8, _g_teled@PAGE+64
Lloh8:
	add	x8, x8, _g_teled@PAGEOFF+64
	ldp	w2, w9, [x8]
	add	w9, w9, #1
	str	w9, [x8, #4]
	add	x0, sp, #8
	mov	w1, #64                         ; =0x40
	bl	_collector_accept
LBB1_2:
	ldur	x8, [x29, #-8]
Lloh9:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh10:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh11:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB1_4
; %bb.3:
	ldp	x29, x30, [sp, #80]             ; 16-byte Folded Reload
	add	sp, sp, #96
	ret
LBB1_4:
	bl	___stack_chk_fail
	.loh AdrpAdd	Lloh5, Lloh6
	.loh AdrpLdrGotLdr	Lloh2, Lloh3, Lloh4
	.loh AdrpAdd	Lloh7, Lloh8
	.loh AdrpLdrGotLdr	Lloh9, Lloh10, Lloh11
	.cfi_endproc
                                        ; -- End function
	.globl	_teled_state                    ; -- Begin function teled_state
	.p2align	2
_teled_state:                           ; @teled_state
	.cfi_startproc
; %bb.0:
Lloh12:
	adrp	x0, _g_teled@PAGE
Lloh13:
	add	x0, x0, _g_teled@PAGEOFF
	ret
	.loh AdrpAdd	Lloh12, Lloh13
	.cfi_endproc
                                        ; -- End function
.zerofill __DATA,__bss,_g_teled,72,2    ; @g_teled
.subsections_via_symbols
```

## `src/camera.c`

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_camera_rtsp_url                ; -- Begin function camera_rtsp_url
	.p2align	2
_camera_rtsp_url:                       ; @camera_rtsp_url
	.cfi_startproc
; %bb.0:
                                        ; kill: def $w1 killed $w1 def $x1
	mov	w8, #0                          ; =0x0
	cbz	x3, LBB0_4
; %bb.1:
	cbz	x0, LBB0_4
; %bb.2:
	cbz	x2, LBB0_4
; %bb.3:
	sub	sp, sp, #64
	stp	x20, x19, [sp, #32]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
Lloh0:
	adrp	x8, l_.str.1@PAGE
Lloh1:
	add	x8, x8, l_.str.1@PAGEOFF
	stp	x1, x8, [sp, #16]
Lloh2:
	adrp	x8, l_.str@PAGE
Lloh3:
	add	x8, x8, l_.str@PAGEOFF
	stp	x8, x0, [sp]
Lloh4:
	adrp	x8, l_.str.4@PAGE
Lloh5:
	add	x8, x8, l_.str.4@PAGEOFF
	mov	x0, x2
	mov	x1, x3
	mov	x2, x8
	mov	x19, x3
	bl	_snprintf
                                        ; kill: def $w0 killed $w0 def $x0
	cmp	w0, #0
	sxtw	x8, w0
	ccmp	x19, x8, #0, gt
	cset	w8, hi
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #64
LBB0_4:
	mov	x0, x8
	ret
	.loh AdrpAdd	Lloh4, Lloh5
	.loh AdrpAdd	Lloh2, Lloh3
	.loh AdrpAdd	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	_camera_mjpeg_url               ; -- Begin function camera_mjpeg_url
	.p2align	2
_camera_mjpeg_url:                      ; @camera_mjpeg_url
	.cfi_startproc
; %bb.0:
                                        ; kill: def $w1 killed $w1 def $x1
	mov	w8, #0                          ; =0x0
	cbz	x3, LBB1_4
; %bb.1:
	cbz	x0, LBB1_4
; %bb.2:
	cbz	x2, LBB1_4
; %bb.3:
	sub	sp, sp, #64
	stp	x20, x19, [sp, #32]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
Lloh6:
	adrp	x8, l_.str.3@PAGE
Lloh7:
	add	x8, x8, l_.str.3@PAGEOFF
	stp	x1, x8, [sp, #16]
Lloh8:
	adrp	x8, l_.str.2@PAGE
Lloh9:
	add	x8, x8, l_.str.2@PAGEOFF
	stp	x8, x0, [sp]
Lloh10:
	adrp	x8, l_.str.4@PAGE
Lloh11:
	add	x8, x8, l_.str.4@PAGEOFF
	mov	x0, x2
	mov	x1, x3
	mov	x2, x8
	mov	x19, x3
	bl	_snprintf
                                        ; kill: def $w0 killed $w0 def $x0
	cmp	w0, #0
	sxtw	x8, w0
	ccmp	x19, x8, #0, gt
	cset	w8, hi
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #64
LBB1_4:
	mov	x0, x8
	ret
	.loh AdrpAdd	Lloh10, Lloh11
	.loh AdrpAdd	Lloh8, Lloh9
	.loh AdrpAdd	Lloh6, Lloh7
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"rtsp"

l_.str.1:                               ; @.str.1
	.asciz	"/stream"

l_.str.2:                               ; @.str.2
	.asciz	"http"

l_.str.3:                               ; @.str.3
	.asciz	"/video.mjpg"

l_.str.4:                               ; @.str.4
	.asciz	"%s://%s:%u%s"

.subsections_via_symbols
```

## `src/main.c`

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #112
	stp	x29, x30, [sp, #96]             ; 16-byte Folded Spill
	add	x29, sp, #96
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
Lloh0:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh1:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh2:
	ldr	x8, [x8]
	stur	x8, [x29, #-8]
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [sp, #48]
	stp	q0, q0, [sp]
	str	q0, [sp, #32]
	mov	w0, #0                          ; =0x0
	bl	_aead_init
Lloh3:
	adrp	x0, l_.str@PAGE
Lloh4:
	add	x0, x0, l_.str@PAGEOFF
	add	x1, sp, #48
	bl	_teled_init
	mov	x0, sp
	bl	_teled_beacon
	ldur	x8, [x29, #-8]
Lloh5:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh6:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh7:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB0_2
; %bb.1:
	eor	w0, w0, #0x1
	ldp	x29, x30, [sp, #96]             ; 16-byte Folded Reload
	add	sp, sp, #112
	ret
LBB0_2:
	bl	___stack_chk_fail
	.loh AdrpLdrGotLdr	Lloh5, Lloh6, Lloh7
	.loh AdrpAdd	Lloh3, Lloh4
	.loh AdrpLdrGotLdr	Lloh0, Lloh1, Lloh2
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"SSAT-468547-FEEBD"

.subsections_via_symbols
```
