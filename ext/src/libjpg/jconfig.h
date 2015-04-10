
#define HAVE_PROTOTYPES
#define HAVE_UNSIGNED_CHAR
#define HAVE_UNSIGNED_SHORT
#undef CHAR_IS_UNSIGNED

#ifndef HAVE_STDDEF_H
#define HAVE_STDDEF_H 1
#endif

#ifndef HAVE_STDLIB_H
#define HAVE_STDLIB_H 1
#endif

#undef NEED_BSD_STRINGS
#undef NEED_SYS_TYPES_H
#undef NEED_FAR_POINTERS
#undef NEED_SHORT_EXTERNAL_NAMES
#undef INCOMPLETE_TYPES_BROKEN

/* Define "boolean" as unsigned char, not enum, on Windows systems.
 */
#ifdef _WIN32
#ifndef __RPCNDR_H__		/* don't conflict if rpcndr.h already read */
typedef unsigned char boolean;
#endif
#ifndef FALSE			/* in case these macros already exist */
#define FALSE	0		/* values of boolean */
#endif
#ifndef TRUE
#define TRUE	1
#endif
#define HAVE_BOOLEAN		/* prevent jmorecfg.h from redefining it */
#endif

#if WINAPI_FAMILY == WINAPI_FAMILY_PC_APP || WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
#define NO_GETENV 1
#endif


#ifdef JPEG_INTERNALS
#undef RIGHT_SHIFT_IS_UNSIGNED
#endif /* JPEG_INTERNALS */
