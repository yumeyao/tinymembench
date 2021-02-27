
/*
 * Copyright © 2021 Tony Mason <fsgeek@cs.ubc.ca>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "pmem.h"

static const size_t pmem_1gb = 1 << 30;
static const size_t pmem_2mb = 2 << 20;
static const size_t pmem_4kb = 4 << 10;
static int pmem_fd = -1;
static char *pmem_map = NULL;
static size_t pmem_off = 0;
static size_t pmem_size = 0;

static size_t align_up1gb(size_t size)
{
    return ((size + (pmem_1gb - 1)) & ~(pmem_1gb - 1));
}

static void *alloc_pmem_buffer(size_t size)
{
    char *buffer = pmem_map + pmem_off;

    if ((MAP_FAILED == pmem_map) || (NULL == pmem_map))
    {
        return NULL;
    }

    if (pmem_off >= pmem_size)
    {
        return NULL;
    }

    size = align_up1gb(size);

    pmem_off += size;

    return (void *)buffer;
}

void *alloc_four_pmem_buffers(void **buf1_, size_t size1,
                              void **buf2_, size_t size2,
                              void **buf3_, size_t size3,
                              void **buf4_, size_t size4, const char *daxname)
{
    int page1gb = 0;
    int page2mb = 0;
    int page4kb = 0;
    size_t space_needed = ~0;

    space_needed = align_up1gb(size1) + align_up1gb(size2) + align_up1gb(size3) + align_up1gb(size4);

    // Gross overestimate
    space_needed = size1 + size2 + size3 + size4 + (pmem_1gb * page1gb) + (pmem_2mb * page2mb) + (pmem_4kb * page4kb);

    pmem_fd = open(daxname, O_RDWR);
    if (0 > pmem_fd)
    {
        return NULL;
    }

    pmem_map = mmap(NULL, space_needed, PROT_READ | PROT_WRITE, MAP_SHARED, pmem_fd, 0);
    if (MAP_FAILED == pmem_map)
    {
        close(pmem_fd);
        pmem_fd = -1;
        return NULL;
    }

    pmem_size = space_needed;

    *buf1_ = alloc_pmem_buffer(size1);
    *buf2_ = alloc_pmem_buffer(size2);
    *buf3_ = alloc_pmem_buffer(size3);
    *buf4_ = alloc_pmem_buffer(size4);

    return NULL;
}
