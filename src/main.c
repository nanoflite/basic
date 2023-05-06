/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		This is the "main program" for the BASIC interpreter. We
 *		set up as needed, and then call the actual BASIC module
 *		for the real work.
 *
 * Usage:	basic [-qd] [-C] [-m memsize] [-s stacksize] [file]
 *
 * Version:	@(#)main.c	1.1.1	2023/05/05
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
#include <getopt.h>
#include <signal.h>
#ifndef _WIN32
# include <unistd.h>
#endif
#ifdef USE_READLINE
# include <readline/readline.h>
# include <readline/history.h>
#endif
#include "arch.h"
#include "basic.h"
#include "private.h"		// TBR! need API cleanup!
#include "version.h"


int		opt_C,
		opt_d,
		opt_q;

static const char	*last_error = NULL;


static void
sigint_handler(int signum)
{
    signal(SIGINT, sigint_handler);

    basic_stop();
}


static void
my_error(const char *msg)
{
    last_error = msg;

    bprintf("--- ERROR: ");
    if (__line != 0)
	bprintf("%i ", __line);
    bprintf("%s\n", msg);
}


static void
my_ready(const char *msg)
{
    if (msg == NULL)
	msg = "READY.";

    bputs(msg);
}


static void
upper_case(char *str)
{
    int quote = 0;

    while (*str != '\0') {
	if (*str == '"') {
		quote ^= 1;
		str++;
		continue;
	}

	if (!quote && (*str >= 'a' && *str <= 'z'))
		*str = *str - 'a' + 'A';

	str++;
    }
}


static void
repl(void)
{
#ifndef USE_READLINE
    char line[1024];
#endif
    char *input;
 
    if (! opt_q) {
	bprintf("%s version %s\n", APP_TITLE, APP_VERSION);
	bprintf("Copyright 2015-2016 Johan Van den Brande\n");
	bprintf("Copyright 2023 Fred N. van Kempen\n");
	bprintf(" _               _      \n");
	bprintf("| |__   __ _ ___(_) ___ \n");
	bprintf("| '_ \\ / _` / __| |/ __|\n");
	bprintf("| |_) | (_| \\__ \\ | (__ \n");
	bprintf("|_.__/ \\__,_|___/_|\\___|\n\n");
    }

#ifdef USE_READLINE
    using_history();
#endif

    my_ready(NULL);

    for (;;) {
#ifdef USE_READLINE
	input = readline("");
	if (input == NULL)
		break;
#else
	memset(line, 0, sizeof(line));
	input = bgets(line, sizeof(line));
	if (input == NULL)
		continue;
#endif

	if (opt_C)
		upper_case(input);

	if (input && *input) {
#ifdef USE_READLINE
		add_history(input);
#endif

		if (! strcmp(input, "QUIT")) {
#ifndef USE_READLINE
			memset(line, 0, sizeof(line));
#endif
			break;
		}

		basic_eval(input);

		if (last_error != NULL) {
			bprintf("ERROR: %s\n", last_error);
			last_error = NULL;
		}
	} else
		input = NULL;
    }

#ifdef USE_READLINE
    clear_history();
#endif
}


static void
run(const char *file_name)
{
    char line[BASIC_STR_LEN];
    FILE *file = fopen(file_name, "r");

    if (file == NULL) {
	fprintf(stderr, "Can't open %s\n", file_name);
	return;  
    }  

    while (fgets(line, sizeof(line), file)) {
	if (line[strlen(line)-1] != '\n') {
		printf("ERROR: NO EOL\n");
		exit(1);      
	}

	basic_eval(line);
    }

    (void)fclose(file);

    basic_start();
}


static void
usage(void)
{
    fprintf(stderr, "Usage: basic [-qd] [-C] [-m memsize] [-s stacksize] [file]\n");
    exit(EXIT_FAILURE);
    /*NOTREACHED*/
}


int
main(int argc, char *argv[])
{
    int mem, stk;
    int c;

    /* Initialize defaults. */
    opt_d = opt_q = 0;
    opt_C = 1;	// force to uppercase
    mem = 8;	// 8 KiB
    stk = 2;	// 2 KiB

    /* Parse commandline options. */
    opterr = 0;
    while ((c = getopt(argc, argv, "Cdqm:s:")) != EOF) switch (c) {
	case 'C':	// flip 'uppercase' flag
		opt_C ^= 1;
		break;

	case 'd':	// debug level
		opt_d++;
		break;

	case 'q':	// quiet
		opt_q ^= 1;
		break;

	case 'm':	// set memory size
		mem = atoi(optarg);
		if (! opt_q)
			printf("Memory size set to %i bytes.\n", mem * 1024);
		break;

	case 's':	// set stack size
		stk = atoi(optarg);
		if (! opt_q)
			printf("Stack size set to %i bytes.\n", stk * 1024);
		break;

	default:
		usage();
		/*NOTREACHED*/
    }

    /* Create an instance of the interpreter. */
    basic_init(mem*1024, stk*1024, my_error, my_ready);

#if 0
    /* These are for platforms that (can) implement them. */
    register_function_1(FUNC_NUMERIC, "PEEK", my_peek, KIND_NUMERIC);
    register_function(FUNC_KEYWORD, "POKE", my_poke);
    register_function(FUNC_KEYWORD, "SYS", my_sys);
    register_function(FUNC_KEYWORD, "WAIT", my_wait);
    register_function_1(FUNC_NUMERIC, "USR", my_usr, KIND_NUMERIC);
#endif

    signal(SIGINT, sigint_handler);

    if (optind < argc)
	run(argv[optind++]);
    else
	repl();

    basic_destroy();

    return EXIT_SUCCESS;
}
