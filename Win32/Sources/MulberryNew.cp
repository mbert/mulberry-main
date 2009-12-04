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

#include <new>

// 960830: Added this for malloc declaration
#include <stdlib.h>

#pragma exceptions on

__using_namespace(std);

// 960715: This code is in mexcept.cpp
//const char * bad_alloc::what() const /*throw()*/
//{
//	return("bad_alloc");
//}


static void default_new_handler() throw(bad_alloc)
{
	throw(bad_alloc());
}

static new_handler	new_handler_func = default_new_handler;

new_handler set_new_handler(new_handler newer_handler)
{
	new_handler	old_handler = new_handler_func;

	new_handler_func = newer_handler;

	return(old_handler);
}

void * operator new(size_t size) throw(bad_alloc)
{
	void * p;

	// Patch for malloc(0) == NULL
	if (!size) size = 4;

	while ((p = malloc(size)) == NULL)
		if (new_handler_func)
			new_handler_func();
		else
			throw(bad_alloc());

	return(p);
}

// 960830: Changed the second parameter type from nothrow to nothrow_t.
// Now it matches the declaration and compiles.
void * operator new(size_t size, const nothrow_t&) throw()
{
	void * p;

	// Patch for malloc(0) == NULL
	if (!size) size = 4;

	while ((p = malloc(size)) == NULL)
		if (new_handler_func)
		{
			try
			{
				new_handler_func();
			}
			catch (bad_alloc)
			{
				return(NULL);
			}
		}
		else
			return(NULL);

	return(p);
}

void operator delete(void * p)
{
	free(p);
}

