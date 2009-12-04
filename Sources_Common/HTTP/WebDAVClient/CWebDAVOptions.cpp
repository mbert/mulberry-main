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

/* 
	CWebDAVOptions.cpp

	Author:			
	Description:	<describe the CWebDAVOptions class here>
*/

#include "CWebDAVOptions.h"

#include <cstring>

using namespace webdav; 

CWebDAVOptions::CWebDAVOptions(CWebDAVSession* session, const cdstring& ruri) :
	CWebDAVRequestResponse(session, eRequest_OPTIONS, ruri)
{
}

CWebDAVOptions::~CWebDAVOptions()
{
}

void CWebDAVOptions::GetAllowed(cdstrvect& methods) const
{
	// Get the ETag header from response headers
	if (GetResponseHeaders().count(cHeaderAllow) != 0)
	{
		// Look at each one and accumlate allowed methods
		for(cdstrmultimapcasei::const_iterator iter = GetResponseHeaders().lower_bound(cHeaderAllow); iter != GetResponseHeaders().upper_bound(cHeaderAllow); iter++)
		{
			cdstring hdr = (*iter).second;
			const char* p = std::strtok(hdr.c_str_mod(), ", \t");
			while(p && *p)
			{
				methods.push_back(p);
				p = std::strtok(NULL, ", \t");
			}
		}
	}
}

bool CWebDAVOptions::IsAllowed(const cdstring& method) const
{
	// Get the ETag header from response headers
	if (GetResponseHeaders().count(cHeaderAllow) != 0)
	{
		// Look at each one and accumlate allowed methods
		for(cdstrmultimapcasei::const_iterator iter = GetResponseHeaders().lower_bound(cHeaderAllow); iter != GetResponseHeaders().upper_bound(cHeaderAllow); iter++)
		{
			cdstring hdr = (*iter).second;
			const char* p = std::strtok(hdr.c_str_mod(), ", \t");
			while(p && *p)
			{
				if (method.compare(p, true) == 0)
					return true;
				p = std::strtok(NULL, ", \t");
			}
		}
	}
	
	return false;
}
