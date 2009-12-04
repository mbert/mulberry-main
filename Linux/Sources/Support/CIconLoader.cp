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


// Source for CIconLoader class

#include "CIconLoader.h"

#include "HResourceMap.h"

CIconLoader::CIconMap CIconLoader::sIcons;

JXImage* CIconLoader::GetIcon(resType resid, JXContainer* pCont, unsigned long size, unsigned long bkgnd, EState state)
{
	// Generate index
	long long map_index = ((resid & 0x0000FFFF) << 16) | (size & 0x0000FFFF);
	map_index <<= 32;
	unsigned long lower = bkgnd & 0x00FFFFFF;
	switch(state)
	{
	case eNormal:
	default:
		break;
	case eDisabled:
		lower |= 0x01000000;
		break;
	case eInverted:
		lower |= 0x02000000;
		break;
	}
	map_index |= lower;
	
	// Try to find an entry
	CIconMap::const_iterator found = sIcons.find(map_index);
	if (found != sIcons.end())
		return (*found).second;
		
	// Create new cache entry
	JXImage* icon = iconFromResource(resid, pCont, size, bkgnd, state);
	sIcons.insert(CIconMap::value_type(map_index, icon));
	
	return icon;
}
