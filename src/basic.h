/*
 * BASIC	A simple, extendable BASIC interpreter in C.
 *
 *		This file is part of the VARCem Project.
 *
 *		Public (API) definitions for the program.
 *
 * Version:	@(#)basic.h	1.1.1	2023/05/05
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
#ifndef BASIC_H
# define BASIC_H


#define BASIC_STR_LEN	1024
#define BASIC_VAR_LEN	8
#define MAX_VECTOR	5			// max array dimensions
#define MAX_ARGUMENT	5			// max function arguments


typedef enum {
    FUNC_KEYWORD = 0,				// standard keyword
    FUNC_VARIABLE,				// standard variable
    FUNC_PRINT,					// print function
    FUNC_OP,					// standard operator
    FUNC_NUMERIC = 0x80,			// return value is numeric
    FUNC_STRING					// return value is a string
} func_type;

typedef enum {
    KIND_NUMERIC,
    KIND_STRING
} var_kind;

typedef enum {
    OP_NOP,
    OP_LT,
    OP_LE,
    OP_EQ,
    OP_GE,
    OP_GT,
    OP_NE
} relop;

typedef union {
    float	number;
    char	*string;
} value;

typedef struct {
    var_kind	kind;
    bool	empty;
    bool	mallocd;
    value	value;
} basic_var;

typedef unsigned short token;

typedef int (*function_0)(basic_var *);
typedef int (*function_1)(basic_var *, const basic_var *);
typedef int (*function_2)(basic_var *, const basic_var *, const basic_var *);
typedef int (*function_3)(basic_var *, const basic_var *, const basic_var *, const basic_var *);
typedef int (*function_4)(basic_var *, const basic_var *, const basic_var *, const basic_var *, const basic_var *);
typedef int (*function_5)(basic_var *, const basic_var *, const basic_var *, const basic_var *, const basic_var *, const basic_var *);

typedef void (*error_t)(const char *);
typedef void (*ready_t)(const char *);


#ifdef _DEBUG
extern volatile bool	__DEBUG;
#endif
extern volatile bool	__RUNNING;
extern volatile bool	__EVALUATING;
extern volatile bool	__REPL;
extern volatile bool	__STOPPED;
extern volatile bool	__ERROR;

extern uint16_t		__line,
			__errline;


#ifdef __cplusplus
extern "C" {
#endif

/* Standard BASIC interface functions. */
extern void	basic_init(size_t, size_t, error_t, ready_t);
extern void	basic_destroy(void);
extern void	basic_start(void);
extern void	basic_stop(void);
extern void	basic_eval(const char *line);
extern void	basic_error(const char *msg);
extern void	basic_ready(const char *msg);

/* These are used for implementing extensions. */
extern token	register_token(token, const char *name);

extern token	register_function(func_type, const char *, function_0);
extern token	register_function_1(func_type, const char *,
				    function_1, var_kind);
extern token	register_function_2(func_type, const char *,
				    function_2, var_kind, var_kind);
extern token	register_function_3(func_type, const char *,
				    function_3 function, var_kind,
				    var_kind, var_kind);
extern token	register_function_4(func_type, const char *,
				    function_4 function,
				    var_kind, var_kind, var_kind, var_kind);
extern token	register_function_5(func_type, const char *,
				    function_5 function,
				    var_kind, var_kind, var_kind,
				    var_kind, var_kind);

#ifdef __cplusplus
};
#endif


#endif	/*BASIC_H*/
