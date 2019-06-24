/*
 * Copyright © 2011 Siarhei Siamashka <siarhei.siamashka@gmail.com>
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

#if defined(__amd64__)

.intel_syntax noprefix
.text

#define PREFETCH_DISTANCE 256

.macro asm_function_helper function_name
    .global \function_name
.func \function_name
\function_name:
#ifdef _WIN64
    .set DST,  rcx
    .set SRC,  rdx
    .set SIZE, r8
#else
    .set DST,  rdi
    .set SRC,  rsi
    .set SIZE, rdx
#endif
.endm

.macro asm_function function_name
asm_function_helper \function_name
.endm

.macro push3 a, b, c
    push \a
    push \b
    push \c
.endm

.macro pop3 a, b, c
    pop \c
    pop \b
    pop \a
.endm

/*****************************************************************************/

asm_function aligned_block_copy_movsb
0:
    push3       rdi rsi rcx
    push3       DST SRC SIZE
    pop3        rdi rsi rcx
    rep movsb
    pop3        rdi rsi rcx
    ret
.endfunc

asm_function aligned_block_copy_movsd
0:
    push3       rdi rsi rcx
    push3       DST SRC SIZE
    pop3        rdi rsi rcx
    sar         rcx, 2
    rep movsd
    pop3        rdi rsi rcx
    ret
.endfunc

asm_function aligned_block_copy_avx2
0:
    vmovdqa      ymm0,       [SRC + (0 * 32)]
    vmovdqa      ymm1,       [SRC + (1 * 32)]
    vmovdqa      ymm2,       [SRC + (2 * 32)]
    vmovdqa      ymm3,       [SRC + (3 * 32)]
    vmovdqa      [DST + (0 * 32)], ymm0
    vmovdqa      [DST + (1 * 32)], ymm1
    vmovdqa      [DST + (2 * 32)], ymm2
    vmovdqa      [DST + (3 * 32)], ymm3
    add         SRC,        (4 * 32)
    add         DST,        (4 * 32)
    sub         SIZE, (4 * 32)
    jg          0b
    ret
.endfunc

asm_function aligned_block_copy_nt_avx2
0:
    vmovdqa      ymm0,       [SRC + (0 * 32)]
    vmovdqa      ymm1,       [SRC + (1 * 32)]
    vmovdqa      ymm2,       [SRC + (2 * 32)]
    vmovdqa      ymm3,       [SRC + (3 * 32)]
    vmovntdq     [DST + (0 * 32)], ymm0
    vmovntdq     [DST + (1 * 32)], ymm1
    vmovntdq     [DST + (2 * 32)], ymm2
    vmovntdq     [DST + (3 * 32)], ymm3
    add         SRC,        (4 * 32)
    add         DST,        (4 * 32)
    sub         SIZE, (4 * 32)
    jg          0b
    ret
.endfunc

asm_function aligned_block_copy_pf32_avx2
0:
    prefetchnta [SRC + PREFETCH_DISTANCE]
    prefetchnta [SRC + PREFETCH_DISTANCE + 32]
    vmovdqa      ymm0,       [SRC + (0 * 32)]
    vmovdqa      ymm1,       [SRC + (1 * 32)]
    vmovdqa      ymm2,       [SRC + (2 * 32)]
    vmovdqa      ymm3,       [SRC + (3 * 32)]
    vmovdqa      [DST + (0 * 32)], ymm0
    vmovdqa      [DST + (1 * 32)], ymm1
    vmovdqa      [DST + (2 * 32)], ymm2
    vmovdqa      [DST + (3 * 32)], ymm3
    add         SRC,        (4 * 32)
    add         DST,        (4 * 32)
    sub         SIZE,       (4 * 32)
    jg          0b
    ret
.endfunc

asm_function aligned_block_copy_nt_pf32_avx2
0:
    prefetchnta [SRC + PREFETCH_DISTANCE]
    prefetchnta [SRC + PREFETCH_DISTANCE + 32]
    vmovdqa      ymm0,       [SRC + (0 * 32)]
    vmovdqa      ymm1,       [SRC + (1 * 32)]
    vmovdqa      ymm2,       [SRC + (2 * 32)]
    vmovdqa      ymm3,       [SRC + (3 * 32)]
    vmovntdq     [DST + (0 * 32)], ymm0
    vmovntdq     [DST + (1 * 32)], ymm1
    vmovntdq     [DST + (2 * 32)], ymm2
    vmovntdq     [DST + (3 * 32)], ymm3
    add         SRC,        (4 * 32)
    add         DST,        (4 * 32)
    sub         SIZE,       (4 * 32)
    jg          0b
    ret
.endfunc

asm_function aligned_block_copy_pf64_avx2
0:
    prefetchnta [SRC + PREFETCH_DISTANCE]
    vmovdqa      ymm0,       [SRC + (0 * 32)]
    vmovdqa      ymm1,       [SRC + (1 * 32)]
    vmovdqa      ymm2,       [SRC + (2 * 32)]
    vmovdqa      ymm3,       [SRC + (3 * 32)]
    vmovdqa      [DST + (0 * 32)], ymm0
    vmovdqa      [DST + (1 * 32)], ymm1
    vmovdqa      [DST + (2 * 32)], ymm2
    vmovdqa      [DST + (3 * 32)], ymm3
    add         SRC,        (4 * 32)
    add         DST,        (4 * 32)
    sub         SIZE,       (4 * 32)
    jg          0b
    ret
.endfunc

asm_function aligned_block_copy_nt_pf64_avx2
0:
    prefetchnta [SRC + PREFETCH_DISTANCE]
    vmovdqa      ymm0,       [SRC + (0 * 32)]
    vmovdqa      ymm1,       [SRC + (1 * 32)]
    vmovdqa      ymm2,       [SRC + (2 * 32)]
    vmovdqa      ymm3,       [SRC + (3 * 32)]
    vmovntdq     [DST + (0 * 32)], ymm0
    vmovntdq     [DST + (1 * 32)], ymm1
    vmovntdq     [DST + (2 * 32)], ymm2
    vmovntdq     [DST + (3 * 32)], ymm3
    add         SRC,        (4 * 32)
    add         DST,        (4 * 32)
    sub         SIZE,       (4 * 32)
    jg          0b
    ret
.endfunc

asm_function aligned_block_fill_sse2
    movdqa      xmm0,       [SRC + 0]
0:
    movdqa      [DST + 0],  xmm0
    movdqa      [DST + 16], xmm0
    movdqa      [DST + 32], xmm0
    movdqa      [DST + 48], xmm0
    add         DST,        64
    sub         SIZE,       64
    jg          0b
    ret
.endfunc

asm_function aligned_block_fill_nt_sse2
    movdqa      xmm0,       [SRC + 0]
0:
    movntdq     [DST + 0],  xmm0
    movntdq     [DST + 16], xmm0
    movntdq     [DST + 32], xmm0
    movntdq     [DST + 48], xmm0
    add         DST,        64
    sub         SIZE,       64
    jg          0b
    ret
.endfunc

/*****************************************************************************/

#endif
