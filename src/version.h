#ifndef VERSION_H
# define VERSION_H


/* Application name. */
#define APP_NAME	"BASIC"
#define APP_TITLE	"Simple BASIC Interpreter"

/* Version info. */
#define APP_VER_MAJOR	1
#define APP_VER_MINOR	1
#define APP_VER_REV	0
#define APP_VER_PATCH	0


/* Standard C preprocessor macros. */
#define STR_STRING(x)	#x
#define STR(x)		STR_STRING(x)
#define STR_RC(a,e)	a ## , ## e


/* These are used in the application. */
#define APP_VER_NUM	APP_VER_MAJOR.APP_VER_MINOR.APP_VER_REV
#if defined(APP_VER_PATCH) && APP_VER_PATCH > 0
# define APP_VER_NUM_4	APP_VER_MAJOR.APP_VER_MINOR.APP_VER_REV.APP_VER_PATCH
#else
# define APP_VER_NUM_4	APP_VER_MAJOR.APP_VER_MINOR.APP_VER_REV.0
#endif
#define APP_VERSION	STR(APP_VER_NUM)
#define APP_VERSION_4	STR(APP_VER_NUM_4)


#endif	/*VERSION_H*/
