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

//	CCarbonMBAR.h

#ifndef _H_CCarbonMBAR
#define _H_CCarbonMBAR
#pragma once

#include <LMenuBar.h>

// ---------------------------------------------------------------------------

class CCarbonMBAR : public LMenuBar
{
public:
			CCarbonMBAR(ResIDT inMBARid);
	virtual ~CCarbonMBAR();

	virtual CommandT	FindKeyCommand(
								const EventRecord&	inKeyEvent,
								SInt32&				outMenuChoice) const;

protected:
	enum
	{
		kDynamicItemsID = 12346
	};

	LMenu*  		mDynamicItems;
	TArray<SInt16>* mDynamicItemsMap;

			void ProcessMenu(LMenu* menu);
			void AddDynamicItem(LMenu* menu, SInt16 index);
};

#endif
