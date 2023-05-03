/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Handle dictionaries.
 *
 * Version:	@(#)dict.c	1.1.0	2023/05/01
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
#include "basic.h"
#include "private.h"


#if PLATFORM == PLATFORM_XMEGA
# define HASHSIZE 101
#else
# define HASHSIZE 13
#endif


struct entry;
typedef struct entry {
    struct entry *next;
    const char	*name;
    void	*value;
} entry;

struct dictionary {
    entry	*hashtab[HASHSIZE];
};

typedef struct {
    dictionary	*d;
    dictionary_each_cb cb;
} _free_s;


static unsigned int
hash(const char *name)
{
    unsigned int hashval;

    for (hashval = 0; *name != '\0'; name++) {
	hashval = *name + 31 * hashval;
    }

    return hashval % HASHSIZE;
}


static entry *
_get(const dictionary *d, const char *name)
{
    entry *entry;

    for (entry = d->hashtab[hash(name)]; entry != NULL; entry = entry->next) {
	if (! strcmp(name, entry->name))
		return entry;
    }

    return NULL;
}


void *
dict_get(dictionary *d, const char *name)
{
    entry *entry = _get(d, name);
  
    if (entry)
	return entry->value;

    return NULL;
}


bool
dict_has(dictionary *d, const char *name)
{
    entry *entry = _get(d, name);

    if (entry)
	return true;

    return false;
}


void
dict_put(dictionary *d, const char *name, void *value)
{
    entry *element;
    unsigned int hashval;

    element = _get(d, name);
    if (element == NULL) {
	element = (entry *)malloc(sizeof(*element));
	if (element == NULL || (element->name = C_STRDUP(name)) == NULL)
		return;
	hashval = hash(name);
	element->next = d->hashtab[hashval];
	d->hashtab[hashval] = element;
    }

    element->value = value;
}


void *
dict_del(dictionary *d, const char *name)
{
    entry *root = d->hashtab[hash(name)];

    if (root == NULL)
	return NULL;

    if (strcmp(name, root->name) == 0) {
	void *value = root->value;

	d->hashtab[hash(name)] = root->next; 
	free((char *)root->name);
	free(root);

	return value;
    }

    entry *element = root;
    while (element->next) {
	entry *next = element->next;

	if (! strcmp(name, next->name)) {
		void *value = next->value;

		element->next = next->next;
		free((char *)next->name);
		free(next); 

		return value;
	}
    } 

    return NULL;
}


void
dict_each(dictionary *d, dictionary_each_cb cb, void *context)
{
    entry *next_entry = NULL;
    size_t i;
  
    if (cb == NULL)
	return;

    for (i = 0; i < HASHSIZE; i++) {
	entry *entry = d->hashtab[i];

	while (entry != NULL) {
		next_entry = entry->next;

		cb(entry->name, entry->value, context);

		entry = next_entry;
	}
    }
}


dictionary *
dict_new(void)
{
    dictionary *d = (dictionary *)malloc(sizeof(dictionary));
    size_t i;

    if (d == NULL)
	return NULL;

    for (i = 0; i < HASHSIZE; i++)
	d->hashtab[i] = NULL;

    return d;
}


static void
destroy_cb_pass_1(const char *name, void *value, void *context)
{
    _free_s *ctx = (_free_s *)context;
//  dictionary *d = ctx->d;

    dictionary_each_cb free_cb = ctx->cb;

    free_cb(name, value, NULL); 

//  dictionary_del(d, name);
}


static void
destroy_cb_pass_2(const char *name, void *value, void *context)
{
    _free_s *ctx = (_free_s *)context;
    dictionary *d = ctx->d;

//  dictionary_each_cb free_cb = ctx->cb;
//  free_cb(name, value, NULL); 

    dict_del(d, name);
}


void
dict_destroy(dictionary *d, dictionary_each_cb free_cb)
{
    _free_s ctx = {
	.d = d,
	.cb = free_cb
    };

    dict_each(d, destroy_cb_pass_1, &ctx);
    dict_each(d, destroy_cb_pass_2, &ctx);

    free(d);
}
