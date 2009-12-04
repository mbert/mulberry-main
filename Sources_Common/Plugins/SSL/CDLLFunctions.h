/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


// Header for CDLLFunctions.h

#ifndef __CDLLFUNCTIONS__MULBERRY__
#define __CDLLFUNCTIONS__MULBERRY__

#include <stddef.h>
#if __dest_os == __mac_os
#include <va_list.h>
	using std::va_list;
#include <CodeFragments.h>
#endif

class CDLLLoader
{
#if __dest_os == __mac_os
	typedef CFragConnectionID TDLLInstance;
#elif __dest_os == __mac_os_x
	typedef CFBundleRef TDLLInstance;
#elif __dest_os == __win32_os
	typedef HANDLE TDLLInstance;
#elif __dest_os == __linux_os
	typedef void* TDLLInstance;
#else
#error __dest_os
#endif

public:
	CDLLLoader(const char* path)
		{ mInstance = NULL; mUnload = true; LoadDLL(path); }
	CDLLLoader(TDLLInstance instance)
		{ mInstance = instance; mUnload = false; }
	~CDLLLoader();

	int LoadFunction(const char* fn_name, void** fn_ptr);

	bool IsLoaded() const
		{ return mInstance != NULL; }

private:
	TDLLInstance	mInstance;
	bool			mUnload;
	
	int	LoadDLL(const char* path);
	void* GetDLLSymbol(const char* name);
};

#define IMPORT_FUNCTION(dll, ret, name, args, vars) \
extern "C" { \
typedef ret (*FP_##name) args; \
FP_##name p##name = NULL; \
ret name args \
{ \
	if ((p##name == NULL) && (dll->LoadFunction(#name, (void**)&p##name) == 0)) \
		return 0; \
\
	return (*p##name) vars; \
} \
}

#define IMPORT_FUNCTION_VOID(dll, ret, name, args, vars) \
extern "C" { \
typedef ret (*FP_##name) args; \
FP_##name p##name = NULL; \
ret name args \
{ \
	if ((p##name == NULL) && (dll->LoadFunction(#name, (void**)&p##name) == 0)) \
		return; \
\
	(*p##name) vars; \
} \
}

#endif
