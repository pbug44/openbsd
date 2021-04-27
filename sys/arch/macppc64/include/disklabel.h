/* $OpenBSD: disklabel.h,v 1.1 2020/05/16 17:11:14 kettenis Exp $ */

/*
 * Copyright (c) 2014 Patrick Wildt <patrick@blueri.se>
 * Copyright (c) 1994 Christopher G. Demetriou
 * All rights reserved.
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

#ifndef _MACHINE_DISKLABEL_H_
#define _MACHINE_DISKLABEL_H_

#define LABELSECTOR	1		/* sector containing label */
#define LABELOFFSET	0		/* offset of label in sector */
#define MAXPARTITIONS	16		/* number of partitions */

/* HFS/DPME */

/* partition map structure from Inside Macintosh: Devices, SCSI Manager
 * pp. 13-14.  The partition map always begins on physical block 1.
 *
 * With the exception of block 0, all blocks on the disk must belong to
 * exactly one partition.  The partition map itself belongs to a partition
 * of type `APPLE_PARTITION_MAP', and is not limited in size by anything
 * other than available disk space.  The partition map is not necessarily
 * the first partition listed.
 */
struct part_map_entry {
#define PART_ENTRY_MAGIC        0x504d
	u_int16_t       pmSig;          /* partition signature */
	u_int16_t       pmSigPad;       /* (reserved) */
	u_int32_t       pmMapBlkCnt;    /* number of blocks in partition map */
	u_int32_t       pmPyPartStart;  /* first physical block of partition */
	u_int32_t       pmPartBlkCnt;   /* number of blocks in partition */
	char            pmPartName[32]; /* partition name */
	char            pmPartType[32]; /* partition type */
	u_int32_t       pmLgDataStart;  /* first logical block of data area */
	u_int32_t       pmDataCnt;      /* number of blocks in data area */
	u_int32_t       pmPartStatus;   /* partition status information */
	u_int32_t       pmLgBootStart;  /* first logical block of boot code */
	u_int32_t       pmBootSize;     /* size of boot code, in bytes */
	u_int32_t       pmBootLoad;     /* boot code load address */
	u_int32_t       pmBootLoad2;    /* (reserved) */
	u_int32_t       pmBootEntry;    /* boot code entry point */
	u_int32_t       pmBootEntry2;   /* (reserved) */
	u_int32_t       pmBootCksum;    /* boot code checksum */
	char            pmProcessor[16]; /* processor type (e.g. "68020") */
	u_int8_t        pmBootArgs[128]; /* A/UX boot arguments */
	/* we do not index the disk image as an array,
	 * leave out the on disk padding
	 */
#if 0
	u_int8_t        pad[248];       /* pad to end of block */
#endif
};

#define PART_TYPE_DRIVER        "APPLE_DRIVER"
#define PART_TYPE_DRIVER43      "APPLE_DRIVER43"
#define PART_TYPE_DRIVERATA     "APPLE_DRIVER_ATA"
#define PART_TYPE_DRIVERIOKIT   "APPLE_DRIVER_IOKIT"
#define PART_TYPE_FWDRIVER      "APPLE_FWDRIVER"
#define PART_TYPE_FWB_COMPONENT "FWB DRIVER COMPONENTS"
#define PART_TYPE_FREE          "APPLE_FREE"
#define PART_TYPE_MAC           "APPLE_HFS"
#define PART_TYPE_OPENBSD       "OPENBSD"

#endif /* _MACHINE_DISKLABEL_H_ */
