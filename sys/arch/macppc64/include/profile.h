/*	$OpenBSD: profile.h,v 1.1 2020/06/25 01:55:14 drahn Exp $ */

/*
 * Copyright (c) 2020 Dale Rahn drahn@openbsd.org
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#ifdef __powerpc32__
#define	MCOUNT \
	__asm__(" \
		.section \".text\" \n\
		.align 2 \n\
		.globl _mcount \n\
		.type	_mcount,@function \n\
	_mcount: \n\
		lwz	11, 4(1) \n\
		mflr	0 \n\
		stw	0, 4(1) \n\
		stwu	1, -48(1) \n\
		stw	3, 8(1) \n\
		stw	4, 12(1) \n\
		stw	5, 16(1) \n\
		stw	6, 20(1) \n\
		stw	7, 24(1) \n\
		stw	8, 28(1) \n\
		stw	9, 32(1) \n\
		stw	10,36(1) \n\
		stw	11,40(1) \n\
		mr	4, 0 \n\
		mr 	3, 11 \n\
		bl __mcount \n\
		lwz	3, 8(1) \n\
		lwz	4, 12(1) \n\
		lwz	5, 16(1) \n\
		lwz	6, 20(1) \n\
		lwz	7, 24(1) \n\
		lwz	8, 28(1) \n\
		lwz	9, 32(1) \n\
		lwz	10,36(1) \n\
		lwz	11,40(1) \n\
		addi	1, 1, 48 \n\
		lwz	0, 4(1) \n\
		mtlr	11 \n\
		stw	11, 4(1) \n\
		mtctr	0 \n\
		bctr \n\
	.Lfe2: \n\
		.size _mcount, .Lfe2-_mcount \n\
	");
#define _MCOUNT_DECL static void __mcount
#ifdef _KERNEL
#define MCOUNT_ENTER						\
	__asm volatile("mfmsr %0" : "=r"(s));			\
	s &= ~PSL_POW;						\
	__asm volatile("mtmsr %0" :: "r"(s & ~PSL_EE))

#define	MCOUNT_EXIT						\
	__asm volatile("mtmsr %0" :: "r"(s))
#endif /* _KERNEL */

#else  /* ! __powerpc32__ */

/* 
 * mcount frame size skips over the red zone (288B) (calling function may use)
 * and 128 bytes of local storage (32 bytes of reserved and 96 of our storage
 * this function assumes it will only every call the local __mcount function
 */
#define MCOUNT \
__asm__(" 								\n"\
	"	.section \".text\"					\n"\
	"	.p2align 2						\n"\
	"	.globl _mcount						\n"\
	"	.local __mcount						\n"\
	"	.type	_mcount,@function				\n"\
	"_mcount:							\n"\
	".L_mcount_gep0:						\n"\
	"	addis %r2, %r12, .TOC.-.L_mcount_gep0@ha;		\n"\
	"	addi %r2, %r2, .TOC.-.L_mcount_gep0@l;			\n"\
	".L_mcount_lep0:						\n"\
	".localentry     _mcount, .L_mcount_lep0-.L_mcount_gep0;	\n"\
	"	ld	%r11,16(%r1)					\n"\
	"	mflr	%r0						\n"\
	"	std	%r0, 16(%r1)					\n"\
	"	stdu	%r1,-(288+128)(%r1)				\n"\
	"	std	%r3, 32(%r1)					\n"\
	"	std	%r4, 40(%r1)					\n"\
	"	std	%r5, 48(%r1)					\n"\
	"	std	%r6, 56(%r1)					\n"\
	"	std	%r7, 64(%r1)					\n"\
	"	std	%r8, 72(%r1)					\n"\
	"	std	%r9, 80(%r1)					\n"\
	"	std	%r10,88(%r1)					\n"\
	"	std	%r11,96(%r1)					\n"\
	"	mr	%r4, %r0					\n"\
	"	mr 	%r3, %r11					\n"\
	"	bl __mcount 						\n"\
	"	nop 							\n"\
	"	ld	%r3, 32(%r1)					\n"\
	"	ld	%r4, 40(%r1)					\n"\
	"	ld	%r5, 48(%r1)					\n"\
	"	ld	%r6, 56(%r1)					\n"\
	"	ld	%r7, 64(%r1)					\n"\
	"	ld	%r8, 72(%r1)					\n"\
	"	ld	%r9, 80(%r1)					\n"\
	"	ld	%r10,88(%r1)					\n"\
	"	ld	%r11,96(%r1)					\n"\
	"	addi	%r1, %r1, (288+128)				\n"\
	"	ld	%r0, 16(%r1)					\n"\
	"	std	%r11,16(%r1)					\n"\
	"	mtlr	%r0						\n"\
	"	blr							\n"\
	"	.size _mcount, .-_mcount				\n"\
	);
#endif /* __powerpc32__ */

#define _MCOUNT_DECL static void __mcount
