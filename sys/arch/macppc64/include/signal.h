/*	$OpenBSD: signal.h,v 1.5 2020/07/13 22:37:37 kettenis Exp $	*/

/*
 * Copyright (C) 1995, 1996 Wolfgang Solfrank.
 * Copyright (C) 1995, 1996 TooLs GmbH.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by TooLs GmbH.
 * 4. The name of TooLs GmbH may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY TOOLS GMBH ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TOOLS GMBH BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef	_MACHDEP_SIGNAL_H_
#define	_MACHDEP_SIGNAL_H_

#include <sys/cdefs.h>

typedef int sig_atomic_t;

#if __BSD_VISIBLE || __XPG_VISIBLE >= 420

#include <machine/_types.h>

#ifdef __powerpc32__
/* from /sys/arch/powerpc/include/signal.h */
/*
 * We have to save all registers on every trap, because
 *	1. user could attach this process every time
 *	2. we must be able to restore all user registers in case of fork
 * Actually, we do not save the fp registers on trap, since
 * these are not used by the kernel. They are saved only when switching
 * between processes using the FPU.
 *
 */
struct trapframe {
	__register_t fixreg[32];
	__register_t lr;
	__register_t cr;
	__register_t xer;
	__register_t ctr;
	int srr0;
	int srr1;
	int dar;			/* dar & dsisr are only filled on a DSI trap */
	int dsisr;
	__register_t exc;
};

struct sigcontext {
	long sc_cookie;
	int sc_mask;			/* saved signal mask */
	struct trapframe sc_frame;	/* saved registers */
};
#else
struct sigcontext {
	long		sc_cookie;
	int		sc_mask;	/* saved signal mask */
	__register_t	sc_reg[32];	/* saved registers */
	__register_t	sc_lr;
	__register_t	sc_cr;
	__register_t	sc_xer;
	__register_t	sc_ctr;
	__register_t	sc_pc;
	__register_t	sc_ps;
	__register_t	sc_vrsave;
	__uint64_t	sc_vsx[128];
	__uint64_t	sc_fpscr;
	__uint64_t	sc_vscr;
};

#define sc_sp	sc_reg[1]
#endif /* __powerpc32__ */

#endif /* __BSD_VISIBLE || __XPG_VISIBLE >= 420 */

#endif	/* _MACHDEP_SIGNAL_H_ */
