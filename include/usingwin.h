#ifndef	__USINGWIN__
#define __USINGWIN__

#ifdef _WIN32
#define C_STRDUP	_strdup
#else
#define C_STRDUP	strdup
#endif

#endif
