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

#include "HResourceMap.h"

#include "CGeneralException.h"
#include "CLog.h"
#include "CWindowsIcon.h"

#include <JXContainer.h>
#include <JXImage.h>

#include <stdlib.h>

struct JICO
{
	JICO(const unsigned long* data) : _d(data) {}
	
	const unsigned long* _d;
};

HResourceMap::HResourceMap() 
{
}

#include "icos.res"
#include "xpms.res"
#include "string.res"

HPredefResourceMap::HPredefResourceMap()
{
	#include "icos.cp"
	#include "xpms.cp"
	#include "stringinit.cp"
}

HPredefResourceMap globalMap;

void* HResourceMap::GetResource(ResIDT resId) 
{
	std::map<int, void*>::iterator f = resMap_.find(resId);
	if (f == resMap_.end())
	{
		//cerr << "Unable to find resource with id number " << resId << "." << std::endl;
		CLOG_LOGTHROW(CGeneralException, resId);
		throw CGeneralException(-1L);
	}
	return (*f).second;
}

void HResourceMap::AddResource(ResIDT resId, void* res)
{
	resMap_[resId] = res;
}

JXImage* iconFromResource(ResIDT resId, JXContainer* pCont, unsigned long size, unsigned long bkgnd, unsigned long state)
{
	try
	{
		return new CWindowsIcon(pCont->GetDisplay(), pCont->GetColormap(), GetGlobalResource<JICO>(resId)._d, size, bkgnd, state);
	}
	catch (CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException&);

		try
		{
			return new JXImage(pCont->GetDisplay(), pCont->GetColormap(), GetGlobalResource<JXPM>(IDI_DEFAULT));
		}
		catch (CGeneralException& ex)
		{
			CLOG_LOGCATCH(CGeneralException&);

			//cerr << "Couldn't find default icon." << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

JXImage* bmpFromResource(ResIDT resId, JXContainer* pCont)
{
	try
	{
		return new JXImage(pCont->GetDisplay(), pCont->GetColormap(), GetGlobalResource<JXPM>(resId));
	}
	catch (CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException&);

		try
		{
			return new JXImage(pCont->GetDisplay(), pCont->GetColormap(), GetGlobalResource<JXPM>(IDI_DEFAULT));
		}
		catch (CGeneralException& ex)
		{
			CLOG_LOGCATCH(CGeneralException&);

			//cerr << "Couldn't find default icon." << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

const char* stringFromResource(ResIDT resId)
{
	static const char* default_string = "String not found";
	try
	{
		return GetGlobalResource<const char*>(resId);
	}
	catch (CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException&);

		return default_string;
	}
}
