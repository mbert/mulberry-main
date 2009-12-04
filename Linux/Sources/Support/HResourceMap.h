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

#ifndef _H_HRESOURCEMAP
#define _H_HRESOURCEMAP

#include <map>

class JXImage;

class HResourceMap
{
public:
	HResourceMap();

	void* GetResource(ResIDT resId);
	void AddResource(ResIDT resId, void* res);

private:
	std::map<int, void*> resMap_;
};

class HPredefResourceMap : public HResourceMap
{
public:
	HPredefResourceMap();
};

extern HPredefResourceMap globalMap;
     
template <class T> T& GetGlobalResource(ResIDT resId)
{
	return *(static_cast<T*>(globalMap.GetResource(resId)));
}

template <class T> void AddGlobalResource(ResIDT resId, T& res)
{
	globalMap.AddResource(resId, static_cast<void*>(&res));
}

class JXContainer;
class JXImage;

JXImage* iconFromResource(ResIDT resId, JXContainer* pCont, unsigned long size, unsigned long bkgnd, unsigned long state = 0);
JXImage* bmpFromResource(ResIDT resId, JXContainer* pCont);
const char* stringFromResource(ResIDT resId);

#endif
