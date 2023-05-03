/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Implement and handle "lines" of BASIC code.
 *
 * Version:	@(#)lines.c	1.1.0	2023/05/01
 *
 * Authors:	Fred N. van Kempen, <waltje@varcem.com>
 *		Johan Van den Brande <johan@vandenbrande.com>
 *
 *		Copyright 2023 Fred N. van Kempen.
 *		Copyright 2015,2016 Johan Van den Brande.
 *
 *		Redistribution and  use  in source  and binary forms, with
 *		or  without modification, are permitted  provided that the
 *		following conditions are met:
 *
 *		1. Redistributions of  source  code must retain the entire
 *		   above notice, this list of conditions and the following
 *		   disclaimer.
 *
 *		2. Redistributions in binary form must reproduce the above
 *		   copyright  notice,  this list  of  conditions  and  the
 *		   following disclaimer in  the documentation and/or other
 *		   materials provided with the distribution.
 *
 *		3. Neither the  name of the copyright holder nor the names
 *		   of  its  contributors may be used to endorse or promote
 *		   products  derived from  this  software without specific
 *		   prior written permission.
 *
 * THIS SOFTWARE  IS  PROVIDED BY THE  COPYRIGHT  HOLDERS AND CONTRIBUTORS
 * "AS IS" AND  ANY EXPRESS  OR  IMPLIED  WARRANTIES,  INCLUDING, BUT  NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE  ARE  DISCLAIMED. IN  NO  EVENT  SHALL THE COPYRIGHT
 * HOLDER OR  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES;  LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON  ANY
 * THEORY OF  LIABILITY, WHETHER IN  CONTRACT, STRICT  LIABILITY, OR  TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING  IN ANY  WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "arch.h"
#ifdef _DEBUG
//# include "hexdump.h"
#endif
#include "basic.h"
#include "private.h"


/* NOTE: we should probably PACK this thing, just to be sure. */
typedef struct line {
    uint16_t	number;
    uint8_t	length;
    char	contents;
} line;


static char	*__memory_end;


static line *
_next(line *l)
{
    char *p = (char *)l;

    p += sizeof(line) - 1 + l->length;

    return (line *)p;
}


static bool
_is_end(line *l)
{
    return l && l->number == 0 && l->length == 0;
}


static line *
_find_end(line *l)
{
    line *n = l;

    while (! _is_end(n))
	n = _next(n);

    return n;
}


void
lines_init(void)
{
    line *l;

    __memory_end = __memory;

    // Signal end
    l = (line *)__memory;
    l->number = 0;
    l->length = 0;
}


size_t
lines_memory_used(void)
{
    char *p = __memory;
    line *start = (line*)p;
    line *end = _find_end(start);

    end = _next(end);

    char *m_start = (char *)start;
    char *m_end = (char *)end;

    return m_end - m_start;
}


size_t
lines_memory_available(void)
{
    return __memory_size - lines_memory_used();
}


bool
lines_store(uint16_t number, const char *contents)
{
    char *p = __memory;
    line *l = (line *)p;
    line *end, *next, *ins;
    size_t sz;
    char *foo;

    while (! _is_end(l)) {
	next = _next(l);

	// Find line that is to be inserted after.
	// That line has a line number < insert and the next line has a >
	if (l->number < number && next->number > number) {
		// We need to insert
		// printf("insert %d\n", number);

		// The address of the insert is the same as the next line
		ins = next;

		// Need to move the memory block holding the rest to the right.
		end = _find_end(ins);

		// Move to next empty slot (we keep the sentinel in the copy)
		end = _next(end);

		// We have the end*,  calculate size to move
		char *m_src = (char *)ins;
		char *m_end = (char *)end;
		size_t m_size = m_end - m_src;

		// Calculate offset to move 
		sz = sizeof(line) - 1 + strlen(contents) + 1;
		char *m_dst = m_src + sz;

		// Move the memory block
		memmove(m_dst, m_src, m_size);

		// Set the data of the insert
		ins->number = number;
		ins->length = (uint8_t)strlen(contents) + 1;

		/*
		 * Current GCC seriously barfs on the original code, where
		 * we 'strcpy' the contents directly into the line struct
		 * at the address of the 'contents' member.  GCC *knows*
		 * that this member is only 1 byte in size, and so when
		 * we do our strcpy into it, the underlying strcpy_chk
		 * will see that its overrunning that buffer...
		 *
		 * Workaround is to use a generic byte pointer derived from
		 * that same address.
		 *
		 * We see this several times in this function.
		 */
#if 0
		strcpy(ins->contents, contents);
#else
		foo = (char *)ins;
		foo += ((uintptr_t)&ins->contents - (uintptr_t)ins);
		strcpy(foo, contents);
#endif
//		hexdump("insert", __memory, 256);

		return true;
	}

	// Replace
	if (l->number == number) {
//		printf("replace %d\n", number);

		// We need to shift the memory to the new offset determined
		// by the size of the line to be inserted
		end = _find_end(l);

		// Move to next empty slot (we keep the sentinel in the copy)
		end = _next(end);

		// Calculate size of bloack
		char *m_src = (char *)next;
		char *m_end = (char *)end;
		size_t m_size = m_end - m_src;

		// Calculate offset to move 
		sz = sizeof(line) - 1 + strlen(contents) + 1;
		size_t act_size = sizeof(line) - 1 + strlen(&l->contents) + 1;
		int offset = (int)(sz - act_size);
		char *m_dst = m_src + offset;

		// Move the memory block
		memmove(m_dst, m_src, m_size);

		// Set the data of the replace
		l->length = (uint8_t)strlen(contents) + 1;
#if 0
		strcpy(&l->contents, contents);
#else
		foo = (char *)l;
		foo += ((uintptr_t)&l->contents - (uintptr_t)l);
		strcpy(foo, contents);
#endif
//		hexdump("replace", __memory, 256);

		return true;
	}

	// Prepend
	if (l->number > number) {
//		printf("prepend %d\n", number);

		// The address of the insert is the same as the actual line
		ins = l;

		// But we need to move the memory block holding the rest to the right.
		end = _find_end(ins);

		// Move to next empty slot (we keep the sentinel in the copy)
		end = _next(end);

		// We have the end*,  calculate size to move
		char *m_src = (char *)ins;
		char *m_end = (char *)end;
		size_t m_size = m_end - m_src;

		// Calculate offset to move
		sz = sizeof(line) - 1 + strlen(contents) + 1;
		char *m_dst = m_src + sz;

		// Move the memory block
		memmove(m_dst, m_src, m_size);

		// Set the data of the insert
		ins->number = number;
		ins->length = (uint8_t)strlen(contents) + 1;
#if 0
		strcpy(&ins->contents, contents);
#else
		foo = (char *)ins;
		foo += ((uintptr_t)&ins->contents - (uintptr_t)ins);
		strcpy(foo, contents);
#endif
//		hexdump("prepend", __memory, 256);
 
		return true;
	}

	l = next;
    }

    l->number = number;
    l->length = (uint8_t)strlen(contents) + 1; // Length is offset to next line
#if 0
    strcpy(&l->contents, contents);
#else
    foo = (char *)l;
    foo += ((uintptr_t)&l->contents - (uintptr_t)l);
    strcpy(foo, contents);
#endif
//  hexdump("append", __memory, 256);

    end = _next(l);
    end->number = 0;
    end->length = 0;

    return true;
}


bool
lines_delete(uint16_t number)
{
    line *l, *next;

//  printf("delete line %i\n", number);

    // find the line
    l = (line *)__memory;

    while (! _is_end(l) && l->number != number)
	l = _next(l);

    if (_is_end(l)) {
//	printf("line %i not found\n", number);
	return false;
    }

    // l is the line to delete
    // check if this is the last line
    next = _next(l);
    if (_is_end(next)) {
	// printf("delete last line\n");
	memset(l, 0x00, sizeof(line) - 1 + strlen(&l->contents) + 1);
	l->number = 0;
	l->length = 0;
	strcpy(&l->contents, "");
    } else {
	// printf("delete not last line\n");
	char *dst = (char *)l;
	char *src = (char *)next;
 
	line *lend = _find_end(next);

	// Move to next empty slot (we keep the sentinel in the copy)
	lend = _next(lend);
	char *end = (char *)lend;
	size_t size = (char*)end - src;
	memmove(dst, src, size);

	size_t rest = src - dst;
	memset(end - rest, 0x00, rest);
    }

//  hexdump("delete", __memory, 256);

    return true;
}


static bool
_in_range(uint16_t i, uint16_t low, uint16_t high)
{
    if (low == 0 && high == 0)
	return true;

    if (low == 0 && i <= high)
	return true;

    if (high == 0 && i >= low)
	return true;

    if (i >= low && i <= high)
	return true;

    return false;
} 


void
lines_list(uint16_t start, uint16_t end, lines_list_cb out)
{
    char *p = __memory;
    line *l = (line *)p;

    while (! _is_end(l)) {
	if (_in_range(l->number, start, end))
		out(l->number, &(l->contents) );

	l = _next(l);
    }
}


void
lines_clear(void)
{
    char *end = (char *)_next(_find_end((line *)__memory));

    memset( __memory, 0x00, end - __memory );
    line *l = (line *)__memory;
    l->number = 0;
    l->length = 0;

//  hexdump("clear", __memory, 256);
}


char *
lines_get_contents(uint16_t number)
{
    line *l = (line *)__memory;

    while (! _is_end( l ) && l->number != number)
	l = _next(l);

    if (_is_end(l))
	return NULL;

    return &l->contents;
}


uint16_t
lines_first(void)
{
  line *l = (line *)__memory;

  return l->number;
}


uint16_t
lines_next(uint16_t number)
{
    line *l = (line *)__memory;

    while (! _is_end(l) && l->number <= number)
	l = _next( l );

    if (number == l->number)
	return 0;

    return l->number;
}
