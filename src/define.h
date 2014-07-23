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
#include "EASTL/fixed_hash_map.h"
// #include "EASTL/hash_map.h"
// #include "EASTL/fixed_list.h"
// #include "EASTL/string.h"
//typedef eastl_size_t	stl_size_t;

#else	//_USE_EASTL

typedef size_t			stl_size_t;

#endif // _USE_EASTL


#endif	//_DEFINE_H_
