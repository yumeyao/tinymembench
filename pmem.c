
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
#include <errno.h>
#include <assert.h>

#include "pmem.h"

static const size_t pmem_1gb = 1 << 30;
static const size_t pmem_2mb = 2 << 20;
static const size_t pmem_4kb = 4 << 10;
static int pmem_fd = -1;
static char *pmem_map = NULL;
static size_t pmem_off = 0;
static size_t pmem_size = 0;

int open_pmem_device(const char *daxname)
{
    char buffer[512];
    struct stat st;
    int fd = -1;

    do
    {
        if (0 != strcmp(daxname, "/dev/"))
        {
            buffer[0] = '\0';
        }
        else
        {
            strcpy(buffer, "/dev/");
        }
        strncat(buffer, daxname, sizeof(buffer) - 1);

        if (stat(daxname, &st) != -1)
        {
            if (S_ISBLK(st.st_mode))
            {
                fprintf(stderr, "Block device is unsupported\n");
                break;
            }
            if (S_ISDIR(st.st_mode))
            {
                fprintf(stderr, "Directory is unsupported\n");
                break;
            }
        }

        fd = open(buffer, O_RDWR);
        break;

    } while (0);

    return fd;
}

size_t align_up1gb(size_t size)
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
                              void **buf4_, size_t size4, int memfd)
{
    // int page1gb = 0;
    // int page2mb = 0;
    // int page4kb = 0;
    size_t space_needed = ~0;
    (void)pmem_4kb;
    (void)pmem_2mb;

    space_needed = align_up1gb(size1) + align_up1gb(size2) + align_up1gb(size3) + align_up1gb(size4);

    // Gross overestimate
    // space_needed = size1 + size2 + size3 + size4 + (pmem_1gb * page1gb) + (pmem_2mb * page2mb) + (pmem_4kb * page4kb);

    if (memfd < 0)
    {
        return NULL;
    }
    pmem_fd = memfd;

    pmem_off = 0;
    pmem_map = mmap(NULL, space_needed, PROT_READ | PROT_WRITE, MAP_SHARED, pmem_fd, 0);
    if (MAP_FAILED == pmem_map)
    {
        fprintf(stderr, "%s: mmap failed (%d): %s ", __func__, errno, strerror(errno));
        fprintf(stderr, "%s: space_needed = %zu\n", __func__, space_needed);
        return NULL;
    }

    pmem_size = space_needed;

    if (NULL != buf1_)
    {
        *buf1_ = alloc_pmem_buffer(size1);
    }
    if (NULL != buf2_)
    {
        *buf2_ = alloc_pmem_buffer(size2);
    }
    if (NULL != buf3_)
    {
        *buf3_ = alloc_pmem_buffer(size3);
    }
    if (NULL != buf4_)
    {
        *buf4_ = alloc_pmem_buffer(size4);
    }

    return pmem_map;
}

void free_pmem_buffers(void *pmem_buf)
{
    assert(NULL != pmem_buf);
    assert(pmem_buf == pmem_map);

    if (0 != munmap(pmem_buf, pmem_off))
    {
        fprintf(stderr, "%s: failed (%d): %s\n", __func__, errno, strerror(errno));
        exit(1);
    }
}
