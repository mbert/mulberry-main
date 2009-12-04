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


// Header for CMachOFunctions.h

#ifndef __CMACHOFUNCTIONS__MULBERRY__
#define __CMACHOFUNCTIONS__MULBERRY__

#include <stddef.h>
//#include <va_list.h>
	//using std::va_list;

class CMachOLoader
{
public:
	CMachOLoader(const char* bundle)
		{ mBundle = NULL; LoadFramework(bundle); }
	~CMachOLoader();

	int LoadFunction(const char* fn_name, void** fn_ptr);
	void* LoadSymbol(const char* name);

	bool IsLoaded() const
		{ return mBundle != NULL; }

private:
	CFBundleRef		mBundle;
	
	int	LoadFramework(const char* bundle);
	void* GetFrameworkProc(const char* name);
	void* GetFrameworkSymbol(const char* name);
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

#define IMPORT_FUNCTION_PTR(dll, ret, name, args, vars) \
extern "C" { \
typedef ret (*FP_##name) args; \
extern FP_##name p##name; \
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

#define IMPORT_FUNCTION_VOID_PTR(dll, ret, name, args, vars) \
extern "C" { \
typedef ret (*FP_##name) args; \
extern FP_##name p##name; \
}

#define IMPORT_SYMBOL(type, name) \
extern "C" { \
type name = NULL; \
}

#define LOAD_FUNC_PTR(dll, name) \
if (p##name == NULL) \
	dll->LoadFunction(#name, (void**)&p##name);

#define LOAD_SYMBOL(dll, type, name) \
if (##name == NULL) \
	##name = *(##type*) dll->LoadSymbol(#name);

#define IMPORT_SYMBOL_PTR(type, name) \
extern "C" { \
typedef ##type* P_##type; \
P_##type p_##name = NULL; \
}

#define LOAD_SYMBOL_PTR(dll, type, name) \
if (##name == NULL) \
	##name = (P_##type) dll->LoadSymbol(#name);


#endif