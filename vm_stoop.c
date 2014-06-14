/*
 * Copyright (c) 1999-2009 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * "Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *	File:	vm_stoop.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Copyright (C) 1986, Avadis Tevanian, Jr.
 *
 *
 *	Display Mach VM statistics.
 *
 ************************************************************************
 * HISTORY
 *  6-Jun-86  Avadis Tevanian, Jr. (avie) at Carnegie-Mellon University
 *	Use official Mach interface.
 *
 *  25-mar-99	A.Ramesh at Apple
 *		Ported to MacOS X
 *  
 *  22-Jan-09	R.Branche at Apple
 *  		Changed some fields to 64-bit to alleviate overflows
 *
 *  6-Jun-14 G.Dawgz at Github
 *    Butchered vm_stat source to display info formatted as bytes
 *    rather than pages.
 ************************************************************************
 */

#include <err.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <mach/mach.h>

vm_statistics64_data_t	vm_stoop, last;
char	*pgmname;
mach_port_t myHost;
vm_size_t pageSize = 4096; 	/* set to 4k default */

void usage(void);
void easystat(char* desc, uint64_t n);
void get_stats(vm_statistics64_t stat);
void showStats(char* modeline);

int main(int argc, char *argv[]) {
	pgmname = argv[0];

	if (argc != 2) {
		usage();
	}

	setlinebuf (stdout);
	myHost = mach_host_self();

	if(host_page_size(mach_host_self(), &pageSize) != KERN_SUCCESS) {
		fprintf(stderr, "%s: failed to get pagesize; defaulting to 4K.\n", pgmname);
		pageSize = 4096;
	}	
	showStats(argv[1]);
    char lastChar = argv[1][strlen(argv[1]) - 1]; 
    if (lastChar != '-') putchar('\n');
    exit(EXIT_SUCCESS);
}

void usage(void) {
	fprintf(stderr, "usage: %s modeline\n", pgmname);
	exit(EXIT_FAILURE);
}

void showStats(char* modeline) {
	get_stats(&vm_stoop);

    int mLen = strlen(modeline);
    for (int i = 0; i < mLen; i++) {
        switch (modeline[i]) {
            case 'f': easystat("Memory free:", 
              (uint64_t) ( pageSize * (vm_stoop.free_count - vm_stoop.speculative_count)));
              break;
            case 'a': easystat("Memory active:", 
              (uint64_t) ( pageSize * (vm_stoop.active_count)));
              break;
            case 'i': easystat("Memory inactive:", 
              (uint64_t) ( pageSize * (vm_stoop.inactive_count)));
              break;
            case '~': easystat("Memory speculative:", 
              (uint64_t) ( pageSize * (vm_stoop.speculative_count)));
              break;
            case 't': easystat("Memory throttled:", 
              (uint64_t) ( pageSize * (vm_stoop.throttled_count)));
              break;
            case 'w': easystat("Memory wired down:", 
              (uint64_t) ( pageSize * (vm_stoop.wire_count)));
              break;
            case 'u': easystat("Memory purgeable:", 
              (uint64_t) ( pageSize * (vm_stoop.purgeable_count)));
              break;
            case '7': easystat("\"Translation faults\":", 
              (uint64_t) (vm_stoop.faults));
              break;
            case 'c': easystat("Memory copy-on-write:", 
              (uint64_t) ( pageSize * (vm_stoop.cow_faults)));
              break;
            case '0': easystat("Memory zero filled:", 
              (uint64_t) ( pageSize * (vm_stoop.zero_fill_count)));
              break;
            case 'r': easystat("Memory reactivated:", 
              (uint64_t) ( pageSize * (vm_stoop.reactivations)));
              break;
            case 'U': easystat("Memory purged:", 
              (uint64_t) ( pageSize * (vm_stoop.purges)));
              break;
            case 'b': easystat("File-backed memory:", 
              (uint64_t) ( pageSize * (vm_stoop.external_page_count)));
              break;
            case '?': easystat("Anonymous memory:", 
              (uint64_t) ( pageSize * (vm_stoop.internal_page_count)));
              break;
            case '.': easystat("Memory stored in compressor:", 
              (uint64_t) ( pageSize * (vm_stoop.total_uncompressed_pages_in_compressor)));
              break;
            case 'o': easystat("Memory occupied by compressor:", 
              (uint64_t) ( pageSize * (vm_stoop.compressor_page_count)));
              break;
        }
	}
}

#define STAT_WIDTH 6
void easystat(char* desc, uint64_t n) {
	char buf[( STAT_WIDTH + 1 )];

    int w = snprintf(buf, sizeof(buf), "%*llu", STAT_WIDTH, n);
    if (w > STAT_WIDTH) {
        w = snprintf(buf, sizeof(buf), "%*lluK", STAT_WIDTH -1, n / 1000);
        if (w > STAT_WIDTH) {
            w = snprintf(buf, sizeof(buf), "%*lluM", STAT_WIDTH -1, n / 1000000);
            if (w > STAT_WIDTH) {
                w = snprintf(buf, sizeof(buf), "%*lluG", STAT_WIDTH -1, n / 1000000000);
            }
        }
    }
    fputs(desc, stdout);
	fputs(buf, stdout);
	putchar(' ');
}

void get_stats(vm_statistics64_t stat) {
	unsigned int count = HOST_VM_INFO64_COUNT;
	kern_return_t ret;
	if ((ret = host_statistics64(myHost, HOST_VM_INFO64, 
		(host_info64_t)stat, &count) != KERN_SUCCESS)) {
		fprintf(stderr, "%s: failed to get statistics. error %d\n", 
			pgmname, ret);
		exit(EXIT_FAILURE);
	}
}
