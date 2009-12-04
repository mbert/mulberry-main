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


// C3PaneItemsTabs

#ifndef __C3PANEITEMSTABS__MULBERRY__
#define __C3PANEITEMSTABS__MULBERRY__

#include "CTabsX.h"

class C3PaneItems;

class C3PaneItemsTabs : public CTabsX
{
public:
	enum { class_ID = '3tab' };

					C3PaneItemsTabs(LStream *inStream);
	virtual 		~C3PaneItemsTabs() {}

			void	SetOwner(C3PaneItems* owner)
		{ mOwner = owner; }

	virtual void	Click(SMouseDownEvent &inMouseDown);

	virtual Boolean		IsHitBy(SInt32		inHorizPort,
								SInt32		inVertPort);

protected:
	C3PaneItems*	mOwner;
	MenuHandle		mMenuChoice;
	
			void	MenuChoice(SInt32 choice, long btn);
};

#endif
