/*	$NetBSD: headersize.c,v 1.3.4.1 1996/06/13 18:35:33 cgd Exp $	*/

/*
 * Copyright (c) 1995, 1996 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 * 
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND 
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

#include <sys/types.h>
#include <sys/exec.h>
#include <sys/exec_ecoff.h>

#define	HDR_BUFSIZE	512

main()
{
	char buf[HDR_BUFSIZE];
	struct ecoff_exechdr *execp;

	if (read(0, &buf, HDR_BUFSIZE) < HDR_BUFSIZE) {
		perror("read");
		exit(1);
	}
	execp = (struct ecoff_exechdr *)buf;

	printf("%d\n", ECOFF_TXTOFF(execp));
}
