/*	$OpenBSD: Locore.c,v 1.17 2019/09/02 23:40:29 kettenis Exp $	*/
/*	$NetBSD: Locore.c,v 1.1 1997/04/16 20:29:11 thorpej Exp $	*/

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

#include <lib/libsa/stand.h>
#include <macppc64/stand/openfirm.h>
#include <dev/cons.h>   
     

/*
#include "machine/cpu.h"
*/

#include "fdt.h"

int main(void);
void syncicache(void *, int);
void * fdt_setup(void);
char * fdt_path_simplify(char *path);
char * fdt_get_str(uint32_t);
#define ENABLE_DECREMENTER_WORKAROUND

/* void bat_init(void); */
void patch_dec_intr(void);

__dead void exit(void);

static int (*openfirmware)(void *);

static void setup(void);
static void ofw2fdt_property(int, void *);
static void recurse_ofw2fdt(int, char *, void *);
static void * fdt_alloc(size_t, size_t, size_t);

size_t strlcpy(char*,const char*,size_t);

asm (".text; .globl _entry; _entry: .long _start,0,0");
#if 0
asm("   .text			\n"
"	.globl	bat_init	\n"
"bat_init:			\n"
"				\n"
"	mfmsr   8		\n"
"	li      0,0		\n"
"	mtmsr   0		\n"
"	isync			\n"
"				\n"
"	mtibatu 0,0		\n"	
"	mtibatu 1,0		\n"
"	mtibatu 2,0		\n"
"	mtibatu 3,0		\n"
"	mtdbatu 0,0		\n"
"	mtdbatu 1,0		\n"
"	mtdbatu 2,0		\n"
"	mtdbatu 3,0		\n"
"				\n"
"	li      9,0x12         	\n"	 /* BATL(0, BAT_M, BAT_PP_RW) */	
"	mtibatl 0,9		\n"
"	mtdbatl 0,9		\n"
"	li      9,0x1ffe        \n"	/* BATU(0, BAT_BL_256M, BAT_Vs) */
"	mtibatu 0,9		\n"
"	mtdbatu 0,9		\n"
"	isync			\n"
"				\n"
"	mtmsr 8  		\n"
"	isync			\n"
"	blr			\n");
#endif

#if 0
static int stack[0x1e0000 / 4  + 4] __attribute__((__used__));
#endif

__dead void
_start(void *vpd, int res, int (*openfirm)(void *), char *arg, int argl)
{
	extern char etext[];

#if 0
	asm(
	"sync			\n"
	"isync			\n"
	"lis	%r1,stack@ha	\n"
	"addi	%r1,%r1,stack@l	\n"
	"addis  %r1,%r1,0x1e0000@ha\n");
#endif

	syncicache((void *)RELOC, etext - (char *)RELOC);

	/* bat_init(); */
	openfirmware = openfirm;	/* Save entry to Open Firmware */
#ifdef ENABLE_DECREMENTER_WORKAROUND
	patch_dec_intr();
#endif
	/* turn  DR and IR off */
	setup();
	main();
	exit();
}

#ifdef ENABLE_DECREMENTER_WORKAROUND
void handle_decr_intr();
__asm (	"	.globl handle_decr_intr\n"
	"       .type handle_decr_intr@function\n"
	"handle_decr_intr:\n"
	"	rfid\n");

void
patch_dec_intr()
{
	int time;
	unsigned int *decr_intr = (unsigned int *)0x900;
	unsigned int br_instr;

	/* this hack is to prevent unexpected Decrementer Exceptions
	 * when Apple openfirmware enables interrupts
	 */
	time = 0x40000000;
	asm("mtdec %0" :: "r"(time));

	/* we assume that handle_decr_intr is in the first 128 Meg */
	br_instr = (18 << 23) | (unsigned int)handle_decr_intr;
	*decr_intr = br_instr;
}
#endif

__dead void
_rtt()
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
	} args = {
		"exit",
		0,
		0
	};

	openfirmware(&args);
	while (1);			/* just in case */
}

int
OF_child(int phandle)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		int phandle;
		int child;
	} args = {
		"child",
		1,
		1,
	};

	int ret;

	args.phandle = phandle;

        if (openfirmware(&args) == -1)
		ret = 0;
	else
		ret = args.child;

	return ret;
}

int
OF_peer(int handle)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		int phandle;
		int sibling;
	} args = {
		"peer",
		1,
		1,
	};
	
	uint32_t s;
	int ret;

	args.phandle = handle;
	
	if (openfirmware(&args) == -1)
		ret = 0;
	else
		ret = args.sibling;

	return (ret);
}
	

int
OF_finddevice(char *name)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		char *device;
		int phandle;
	} args = {
		"finddevice",
		1,
		1,
	};

	args.device = name;
	if (openfirmware(&args) == -1)
		return -1;
	return args.phandle;
}

int
OF_instance_to_package(int ihandle)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		int ihandle;
		int phandle;
	} args = {
		"instance-to-package",
		1,
		1,
	};

	args.ihandle = ihandle;
	if (openfirmware(&args) == -1)
		return -1;
	return args.phandle;
}

int
OF_package_to_path(int phandle, char *buf, int buflen)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		int phandle;
		char *buf;
		int buflen;
		int length;
	} args = {
		"package-to-path",
		3,
		1,
	};

	args.phandle = phandle;
	args.buf = buf;
	args.buflen = buflen;
	
	if (openfirmware(&args) < 0)
		return -1;

	return (args.length);
}


int
OF_getproplen(int handle, char *prop)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		int phandle;
		char *prop;
		int size;
	} args = {
		"getproplen",
		2,
		1,
	};

	args.phandle = handle;
	args.prop = prop;

	if (openfirmware(&args) == -1)
		return -1;

	return args.size;
}

int
OF_getprop(int handle, char *prop, void *buf, int buflen)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		int phandle;
		char *prop;
		void *buf;
		int buflen;
		int size;
	} args = {
		"getprop",
		4,
		1,
	};

	args.phandle = handle;
	args.prop = prop;
	args.buf = buf;
	args.buflen = buflen;
	if (openfirmware(&args) == -1)
		return -1;
	return args.size;
}

int
OF_nextprop(int handle, char *prop, void *nextprop, int nextlen)
{
	static struct {
		const char *name;
		int nargs;
		int nreturns;
		int phandle;
		const char *prop;
		char *buf;
		int flag;
	} args = {
		"nextprop",
		3,
		1,
	};

	static char obuf[32];

	args.phandle = handle;
	args.prop = prop;
	args.buf = &obuf[0];
	
	if (openfirmware(&args) == -1)
		return -1;
	
	strlcpy(nextprop, obuf, nextlen);

	return (args.flag);
}

int
OF_open(char *dname)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		char *dname;
		int handle;
	} args = {
		"open",
		1,
		1,
	};

	args.dname = dname;
	if (openfirmware(&args) == -1)
		return -1;
	return args.handle;
}

void
OF_close(int handle)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		int handle;
	} args = {
		"close",
		1,
		0,
	};

	args.handle = handle;
	openfirmware(&args);
}

int
OF_write(int handle, void *addr, int len)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		int ihandle;
		void *addr;
		int len;
		int actual;
	} args = {
		"write",
		3,
		1,
	};

	args.ihandle = handle;
	args.addr = addr;
	args.len = len;
	if (openfirmware(&args) == -1)
		return -1;
	return args.actual;
}

int
OF_read(int handle, void *addr, int len)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		int ihandle;
		void *addr;
		int len;
		int actual;
	} args = {
		"read",
		3,
		1,
	};

	args.ihandle = handle;
	args.addr = addr;
	args.len = len;
	if (openfirmware(&args) == -1)
		return -1;
	return args.actual;
}

int
OF_seek(int handle, u_quad_t pos)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		int handle;
		int poshi;
		int poslo;
		int status;
	} args = {
		"seek",
		3,
		1,
	};

	args.handle = handle;
	args.poshi = (int)(pos >> 32);
	args.poslo = (int)pos;
	if (openfirmware(&args) == -1)
		return -1;
	return args.status;
}

void *
OF_claim(void *virt, u_int size, u_int align)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		void *virt;
		u_int size;
		u_int align;
		void *baseaddr;
	} args = {
		"claim",
		3,
		1,
	};

	args.virt = virt;
	args.size = size;
	args.align = align;
	if (openfirmware(&args) == -1)
		return (void *)-1;
	if (virt != 0)
		return virt;
	return args.baseaddr;
}

void
OF_release(void *virt, u_int size)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		void *virt;
		u_int size;
	} args = {
		"release",
		2,
		0,
	};

	args.virt = virt;
	args.size = size;
	openfirmware(&args);
}

int
OF_milliseconds()
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		int ms;
	} args = {
		"milliseconds",
		0,
		1,
	};

	openfirmware(&args);
	return args.ms;
}

#ifdef __notyet__
void
OF_chain(void *virt, u_int size, void (*entry)(), void *arg, u_int len)
{
	static struct {
		char *name;
		int nargs;
		int nreturns;
		void *virt;
		u_int size;
		void (*entry)();
		void *arg;
		u_int len;
	} args = {
		"chain",
		5,
		0,
	};

	args.virt = virt;
	args.size = size;
	args.entry = entry;
	args.arg = arg;
	args.len = len;
	openfirmware(&args);
}
#else
void
OF_chain(void *virt, u_int size, void (*entry)(), void *arg, u_int len, void *fdt)
{
	/*
	 * This is a REALLY dirty hack till the firmware gets this going
	OF_release(virt, size);
	 */
	asm(
        "slbia			\n"
	"sync			\n"
	"isync			\n"
	"mfmsr   %r8		\n"
	"li	 %r9,0xffffffcf	\n"
	"and     %r0,%r8,%r9	\n"
        "li      %r21, 1	\n"
        "insrdi  %r0, %r21, 1, 0\n"
	"insrdi  %r0, %r21, 4, 0\n"
        "mtmsrd  %r0		\n"
	"isync			\n");

	/* r3=fdt, r4=nothing, r5=nothing */
	entry(fdt, 0, 0, arg, len);
	printf("entry failed\n");
}
#endif

int
OF_call_method(char *method, int ihandle, int nargs, int nreturns, ...)
{
	va_list ap;
	static struct {
		char *name;
		int nargs;
		int nreturns;
		char *method;
		int ihandle;
		int args_n_results[12];
	} args = {
		"call-method",
		2,
		1,
	};
	int *ip, n;

	if (nargs > 6)
		return -1;
	args.nargs = nargs + 2;
	args.nreturns = nreturns + 1;
	args.method = method;
	args.ihandle = ihandle;
	va_start(ap, nreturns);
	for (ip = args.args_n_results + (n = nargs); --n >= 0;)
		*--ip = va_arg(ap, int);

	if (openfirmware(&args) == -1) {
		va_end(ap);
		return -1;
	}
	if (args.args_n_results[nargs]) {
		va_end(ap);
		return args.args_n_results[nargs];
	}
	for (ip = args.args_n_results + nargs + (n = args.nreturns); --n > 0;)
		*va_arg(ap, int *) = *--ip;
	va_end(ap);
	return 0;
}

static int stdin;
static int stdout;

static void
setup()
{
	int chosen;

	if ((chosen = OF_finddevice("/chosen")) == -1)
		_rtt();
	if (OF_getprop(chosen, "stdin", &stdin, sizeof(stdin)) != sizeof(stdin)
	    || OF_getprop(chosen, "stdout", &stdout, sizeof(stdout)) !=
	    sizeof(stdout))
		_rtt();
	if (stdout == 0) {
		/* screen should be console, but it is not open */
		stdout = OF_open("screen");
	}
}

void *
fdt_setup(void)
{
	uint len, node, devnode, chosen, savenode;
	size_t fdt_size = (1024 * 1024);
	int error = 0;
	char nextprop[1024], walkbuf[33];
	char *property, *p;
	void *fnode, *fchild, *child;
	static void *fdt = NULL;
		void *mynode;
		char *prop;
	char alias[1024];

#ifdef DEBUG
	printf("[Scanning OpenFirmware, please wait...]\n");
#endif

	fdt = fdt_alloc(fdt_size, fdt_size, fdt_size);
	if (fdt == NULL) {
		printf("could not alloc fdt\n");
		_rtt();
	}

	if (! fdt_init(fdt)) {
		printf("could not init fdt\n");
		_rtt();
	}

	if ((chosen = OF_finddevice("/")) == -1)
		_rtt();

	fnode = fdt_next_node(NULL);

	for (devnode = chosen; devnode != 0; devnode = OF_peer(devnode)) {
		len = OF_package_to_path(devnode, (char *)&nextprop, sizeof(nextprop) - 1);
		if (len == -1)
			continue;	/* XXX - we really shouldn't skip */

		nextprop[len] = '\0';
		p = nextprop;

		while (*p == '/')
			p++;

		fdt_node_add_node(fnode, p, &fchild);
		ofw2fdt_property(devnode, fchild);

		if (OF_child(devnode) != 0) {
			recurse_ofw2fdt(devnode, nextprop, fnode);
		}
	}

	fdt_finalize();

#if DEBUG
	printf("printing tree\n");
	fdt_print_tree();

	printf("[fdt is at 0x%x]\n", fdt);
#endif
	printf("\n");

	
		mynode = fdt_find_node("/aliases");
		if (! mynode)
			_rtt();

		printf("looking for screen on /aliases\n");
		printf("return value of fdt_node_property = %d\n", fdt_node_property(mynode, "screen", &prop));

		strlcpy(alias, fdt_path_simplify(prop), sizeof(alias));
		p = alias;
		if (*p == '/')
			p++;
		printf("prop = %s\n", p);
		

        	for (child = fdt_find_node(prop); child; child = fdt_next_node(child)) {
			if (strcmp(p, fdt_path_simplify(fdt_node_name(child))) == 0) {
				fdt_print_node(child, 1);
				break;
			}
		}

		printf("getting device_type property\n");
                fdt_node_property(child, "device_type", &prop);
                if (strcmp(prop, "display") == 0) {
			printf("display found!\n");	
		} else
			printf("no display found\n");

	return (fdt);
}

void
putchar(int c)
{
	char ch = c;
	if (c == '\177') {
		ch = '\b';
		OF_write(stdout, &ch, 1);
		ch = ' ';
		OF_write(stdout, &ch, 1);
		ch = '\b';
	}
	if (c == '\n')
		putchar('\r');
	OF_write(stdout, &ch, 1);
}

void
ofc_probe(struct consdev *cn)
{
	cn->cn_pri = CN_LOWPRI;
	cn->cn_dev = makedev(0,0); /* WTF */
}


void
ofc_init(struct consdev *cn)
{
}

char buffered_char;
int
ofc_getc(dev_t dev)
{
	u_int8_t ch;
	int l;

	if (dev & 0x80)  {
		if (buffered_char != 0)
			return 1;

		l = OF_read(stdin, &ch, 1);
		if (l == 1) {
			buffered_char = ch;
			return 1;
		}
		return 0;
	}

	if (buffered_char != 0) {
		ch = buffered_char;
		buffered_char = 0;
		return ch;
	}

	while ((l = OF_read(stdin, &ch, 1)) != 1)
		if (l != -2 && l != 0)
			return 0;
	return ch;
		
}

void
ofc_putc(dev_t dev, int c)
{
	char ch;

	ch = 'a';
	OF_write(stdout, &ch, 1);
	ch = c;
	if (c == '\177' && c == '\b') {
		ch = 'A';
	}
	OF_write(stdout, &ch, 1);
}


void 
machdep()
{
	cninit();
}

static void *
fdt_alloc(size_t struct_len, size_t strings_len, size_t res_len)
{
	size_t len = struct_len + strings_len + res_len;
	void *fdt;
        struct fdt_head *fh;
        uint32_t *ptr;
	int sizehead = sizeof(struct fdt_head);
	int sizefdt = sizeof(struct fdt);

	fdt = alloc(len * 2);	/* just in case becuase fdt goes byond buffer */

        fh = fdt;
        ptr = (uint32_t *)fdt;

	fh->fh_magic = htobe32(FDT_MAGIC);
	fh->fh_comp_ver = fh->fh_version = htobe32(FDT_CODE_VERSION);

	fh->fh_struct_off = htobe32(sizehead);
	ptr += (sizehead / 4);
	*ptr = htobe32(FDT_NODE_BEGIN);

	fh->fh_struct_size = htobe32(struct_len - sizehead - 4);
	ptr += (((struct_len - sizehead - 4) / 4) - 1);
	*ptr = htobe32(FDT_END);

	fh->fh_strings_off = htobe32(sizehead + struct_len);
	fh->fh_strings_size = htobe32(strings_len);
	fh->fh_reserve_off = htobe32(sizehead + struct_len + strings_len);

	fh->fh_size = htobe32(len);


	return (fdt);
}

void
ofw2fdt_property(int node, void *fchild)
{
	char buf[2048];
	int buflen = 2048;
	int use_dynbuf = 0;
	char nextprop[32], property[32];
	int len, nlen = 32;
	int error;
	char *val;

	strlcpy(property, "name", sizeof(property));
	len = OF_getproplen(node, property);		
	if (len < 0) {
		printf("could not OF_getproplen() property\n");
		_rtt();
	}

	if (len > (buflen - 1)) {
		val = alloc(len + 1);		
		OF_getprop(node, property, val, len);	
		val[len] = 0;
		fdt_node_add_property(fchild, property, val, len);
		use_dynbuf = 1;
	} else {
		OF_getprop(node, property, buf, len);
		buf[len] = 0;		/* for some reason we need this */
		fdt_node_add_property(fchild, property, buf, len);
	}
		

	while ((error = OF_nextprop(node, property, (char *)&nextprop, sizeof(nextprop))) > 0) {
		if (use_dynbuf) {
			free(val, len + 1);
			use_dynbuf = 0;
		}

		nlen = strlen(nextprop) + 1;
		strlcpy(property, nextprop, sizeof(property));

		len = OF_getproplen(node, property);		
		if (len < 0) {
			printf("could not OF_getproplen() property\n");
			_rtt();
		}

		if (len > (buflen - 1)) { 
			val = alloc(len + 1);	
			OF_getprop(node, property, val, len);	
			val[len] = 0;
			fdt_node_add_property(fchild, property, val, len);
			use_dynbuf = 1;
		} else {
			OF_getprop(node, property, buf, len);	
			buf[len] = 0;
			fdt_node_add_property(fchild, property, buf, len);
		}
	}

	if (use_dynbuf == 1) 
		free(val, len + 1);
	

	printf(".");

	return;
}

static void
recurse_ofw2fdt(int devnode, char *val, void *fnode)
{
	char nextprop[1024];
	char *cval, *p;
	void *fchild;
	int clen, node, savenode;

	for (node = OF_child(devnode); node != 0; node = OF_peer(node)) {

		clen = OF_package_to_path(node, (char *)&nextprop, sizeof(nextprop) - 1);
		nextprop[clen] = '\0';
		p = nextprop;

		while (*p == '/')
			p++;

		fdt_node_add_node(fnode, p, &fchild);
		ofw2fdt_property(node, fchild);

		if (OF_child(node) != 0)
			recurse_ofw2fdt(node, nextprop, fnode);

	}
}

char *
fdt_path_simplify(char *path)
{
	static char result[1024];
	char *output = &result[0];
	char *p;
	
	for (p = path; *p; p++) {
		if (*p == '@')
			while (*p && *++p != '/');
	
		*output++ = *p;
	}	

	*output = '\0';

	return (&result[0]);
}
