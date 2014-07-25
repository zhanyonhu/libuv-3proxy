#ifndef _DEFINE_H_
#define _DEFINE_H_

//EASTL
#define _USE_EASTL
#ifdef _USE_EASTL

#ifdef _DEBUG
#define EASTL_DLL	0
#else	//_DEBUG
#define EASTL_DLL	1
#define EASTL_API
#define EASTL_TEMPLATE_API
#endif	//_DEBUG

#define STL				eastl
#include "EASTL/fixed_hash_set.h"
typedef eastl_size_t	stl_size_t;

#else	//_USE_EASTL

typedef size_t			stl_size_t;

#endif // _USE_EASTL

#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <process.h>
#if !defined(__MINGW32__)
# include <crtdbg.h>
#endif
#else /*WIN32*/
#include <stdint.h> /* uintptr_t */

#include <errno.h>
#include <unistd.h> /* usleep */
#include <string.h> /* strdup */
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <assert.h>

#include <sys/select.h>
#include <pthread.h>
#endif /*WIN32*/

#include "uv.h"

#define MAX_PROXY_NUMBER				5000




#define FATAL(msg)                                        \
	do {\
	\
	fprintf(stderr, \
	"Fatal error in %s on line %d: %s\n", \
	__FILE__, \
	__LINE__, \
	msg);                                         \
	fflush(stderr);                                       \
	getchar();                                              \
	abort();                                              \
	} while (0)

#ifdef _DEBUG
#define ASSERT(expr)                                      \
	do {\
	\
	if (!(expr)) {\
	\
	fprintf(stderr, \
	"Assertion failed in %s on line %d: %s\n", \
	__FILE__, \
	__LINE__, \
#expr);                                       \
	getchar();                                              \
	abort();                                              \
	}                                                       \
	} while (0)
#else	/* _DEBUG*/
#define ASSERT(expr)   
#endif	/* _DEBUG*/

#ifdef _DEBUG
#define VERIFY(expr)		ASSERT(expr)
#else	/* _DEBUG*/
#define VERIFY(expr)		expr
#endif	/* _DEBUG*/

#define LOG_ERR			logerr

#endif	//_DEFINE_H_
