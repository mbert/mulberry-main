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

#include "CDayEvent.h"

#include "CFontCache.h"

// ---------------------------------------------------------------------------
//	CDayEvent														  [public]
/**
	Default constructor */

CDayEvent::CDayEvent()
{
	mColumnOffset = 0;
	mColumnTotal = 1;
	mSpanToEdge = false;
	mRelativeOffset = 0.0;
	mRelativeWidth = 1.0;
}


CDayEvent* CDayEvent::Create(CWnd* parent, const CRect& frame)
{
	CDayEvent* result = new CDayEvent;
	result->CreateEx(WS_EX_CONTROLPARENT, NULL, NULL, WS_CHILD, frame, parent, 0);
	result->SetFont(CFontCache::GetListFont());
	return result;
}

