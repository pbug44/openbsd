/*	$OpenBSD: asm.h,v 1.6 2020/10/22 23:35:43 mortimer Exp $	*/

/*
 * Copyright (c) 2020 Dale Rahn <drahn@openbsd.org>
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

#ifndef _POWERPC64_ASM_H_
#define _POWERPC64_ASM_H_

#ifdef __powerpc32__

/* from /sys/arch/powerpc/include/asm.h */

/* XXX */
#define TARGET_ELF

#ifdef __PIC__
#define PIC_PROLOGUE	XXX
#define PIC_EPILOGUE	XXX
#ifdef	__STDC__
#define PIC_PLT(x)	x ## @plt
#define PIC_GOT(x)	XXX
#define PIC_GOTOFF(x)	XXX
#else	/* not __STDC__ */
#define PIC_PLT(x)	x/**/@plt
#define PIC_GOT(x)	XXX
#define PIC_GOTOFF(x)	XXX
#endif	/* __STDC__ */
#else
#define PIC_PROLOGUE
#define PIC_EPILOGUE
#define PIC_PLT(x)	x
#define PIC_GOT(x)	x
#define PIC_GOTOFF(x)	x
#endif

#ifdef TARGET_ELF
# define _C_LABEL(x)	x
#endif
#define	_ASM_LABEL(x)	x

#ifdef __STDC__
# define _TMP_LABEL(x)	.L_ ## x
#else
# define _TMP_LABEL(x)	.L_/**/x
#endif

#define _ENTRY(x) \
	.text; .align 2; .globl x; .type x,@function; x:

#if defined(PROF) || defined(GPROF)
# define _PROF_PROLOGUE(y)	\
	.section ".data"; \
	.align 2; \
_TMP_LABEL(y):; \
	.long 0; \
	.section ".text"; \
	mflr 0; \
	addis 11, 11, _TMP_LABEL(y)@ha; \
	stw 0, 4(1); \
	addi 0, 11,_TMP_LABEL(y)@l; \
	bl _mcount; 
#else
# define _PROF_PROLOGUE(y)
#endif

#define	ENTRY(y)	_ENTRY(_C_LABEL(y)); _PROF_PROLOGUE(y)
#define	ASENTRY(y)	_ENTRY(_ASM_LABEL(y)); _PROF_PROLOGUE(y)
#define	END(y)		.size y, . - y

#define STRONG_ALIAS(alias,sym) \
	.global alias; .set alias,sym
#define WEAK_ALIAS(alias,sym) \
	.weak alias; .set alias,sym

#if defined(_RET_PROTECTOR)
# if defined(__PIC__)
#  define RETGUARD_LOAD_RANDOM(x, reg)					\
	bcl	20, 31, 66f;						\
66:	mflr	reg;							\
	addis	reg, reg, (__retguard_ ## x - 66b)@ha;			\
	lwz	reg, ((__retguard_ ## x - 66b)@l)(reg)
# else
#  define RETGUARD_LOAD_RANDOM(x, reg)					\
	lis	reg, (__retguard_ ## x)@ha;				\
	lwz	reg, ((__retguard_ ## x)@l)(reg)
# endif
# define RETGUARD_SETUP(x, reg, retreg)					\
	mflr	retreg;							\
	RETGUARD_SETUP_LATE(x, reg, retreg)
# define RETGUARD_SETUP_LATE(x, reg, retreg)				\
	RETGUARD_SYMBOL(x);						\
	RETGUARD_LOAD_RANDOM(x, reg);					\
	xor	reg, reg, retreg
# define RETGUARD_CHECK(x, reg, retreg)					\
	xor	reg, reg, retreg;					\
	RETGUARD_LOAD_RANDOM(x, %r10);					\
	mtlr	retreg;							\
	twne	reg, %r10
# define RETGUARD_SAVE(reg, loc)					\
	stw reg, loc
# define RETGUARD_LOAD(reg, loc)					\
	lwz reg, loc
# define RETGUARD_SYMBOL(x)						\
	.ifndef __retguard_ ## x;					\
	.hidden __retguard_ ## x;					\
	.type   __retguard_ ## x,@object;				\
	.pushsection .openbsd.randomdata.retguard,"aw",@progbits; 	\
	.weak   __retguard_ ## x;					\
	.p2align 2;							\
	__retguard_ ## x: ;						\
	.long 0;							\
	.size __retguard_ ## x, 4;					\
	.popsection;							\
	.endif
#else
# define RETGUARD_LOAD_RANDOM(x, reg)
# define RETGUARD_SETUP(x, reg, retreg)
# define RETGUARD_SETUP_LATE(x, reg, retreg)
# define RETGUARD_CHECK(x, reg, retreg)
# define RETGUARD_SAVE(reg, loc)
# define RETGUARD_LOAD(reg, loc)
# define RETGUARD_SYMBOL(x)
#endif


#else

#define _C_LABEL(x)	x
#define _ASM_LABEL(x)	x

#define _TMP_LABEL(x)	.L_ ## x
#define _GEP_LABEL(x)	.L_ ## x ## _gep0
#define _LEP_LABEL(x)	.L_ ## x ## _lep0

#define _ENTRY(x)						\
	.text; .align 2; .globl x; .type x,@function; x:	\
	_GEP_LABEL(x):						\
	addis	%r2, %r12, .TOC.-_GEP_LABEL(x)@ha;		\
	addi	%r2, %r2, .TOC.-_GEP_LABEL(x)@l;		\
	_LEP_LABEL(x):						\
	.localentry     _C_LABEL(x), _LEP_LABEL(x)-_GEP_LABEL(x);

#if defined(PROF) || defined(GPROF)
# define _PROF_PROLOGUE(y)					\
	.section ".data";					\
	.align 2;						\
_TMP_LABEL(y):;							\
	.long	0;						\
	.section ".text";					\
	mflr	%r0;						\
	addis	%r11, %r2, _TMP_LABEL(y)@toc@ha;		\
	std	%r0, 8(%r1);					\
	addi	%r0, %r11, _TMP_LABEL(y)@toc@l;			\
	bl _mcount; 
#else
# define _PROF_PROLOGUE(y)
#endif

#define ENTRY(y)	_ENTRY(_C_LABEL(y)); _PROF_PROLOGUE(y)
#define ASENTRY(y)	_ENTRY(_ASM_LABEL(y)); _PROF_PROLOGUE(y)
#define END(y)		.size y, . - y

#define STRONG_ALIAS(alias,sym) \
	.global alias; .set alias,sym
#define WEAK_ALIAS(alias,sym) \
	.weak alias; .set alias,sym

#if defined(_RET_PROTECTOR)
# define RETGUARD_SETUP(x, reg)						\
	RETGUARD_SYMBOL(x);						\
	mflr %r0;							\
	addis reg, %r2, (__retguard_ ## x)@toc@ha;			\
	ld reg, ((__retguard_ ## x)@toc@l)(reg);			\
	xor reg, reg, %r0
# define RETGUARD_CHECK(x, reg)						\
	mflr %r0;							\
	xor reg, reg, %r0;						\
	addis %r12, %r2, (__retguard_ ## x)@toc@ha;			\
	ld %r12, ((__retguard_ ## x)@toc@l)(%r12);			\
	tdne reg, %r12
# define RETGUARD_SAVE(reg, loc)					\
	std reg, loc
# define RETGUARD_LOAD(reg, loc)					\
	ld reg, loc
# define RETGUARD_SYMBOL(x)						\
	.ifndef __retguard_ ## x;					\
	.hidden __retguard_ ## x;					\
	.type   __retguard_ ## x,@object;				\
	.pushsection .openbsd.randomdata.retguard,"aw",@progbits; 	\
	.weak   __retguard_ ## x;					\
	.p2align 3;							\
	__retguard_ ## x: ;						\
	.quad 0;							\
	.size __retguard_ ## x, 8;					\
	.popsection;							\
	.endif
#else
# define RETGUARD_SETUP(x, reg)
# define RETGUARD_CHECK(x, reg)
# define RETGUARD_SAVE(reg, loc)
# define RETGUARD_LOAD(reg, loc)
# define RETGUARD_SYMBOL(x)
#endif
#endif /* __powerpc32__ */

#endif /* !_POWERPC64_ASM_H_ */
