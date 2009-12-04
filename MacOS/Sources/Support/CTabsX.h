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


// CTabsX

#ifndef __CTABSX__MULBERRY__
#define __CTABSX__MULBERRY__

#include <LTabsControl.h>

struct SStyleTraits;

class CTabsX : public LTabsControl
{
public:
	enum { class_ID = 'Xtab' };

					CTabsX(LStream *inStream) : LTabsControl(inStream) {}
	virtual 		~CTabsX() {}

	void	AddTabButton(const char* name, short icon);
	void	SelectTabButtonAt(unsigned long index, bool silent = false);
	void	ChangeTabButtonAt(unsigned long index, const char* name, short icon);
	void	ChangeTabButtonIconAt(unsigned long index, short icon);
	void	RemoveTabButtonAt(unsigned long index);
	void	MoveTabButtonAt(unsigned long old_index, unsigned long new_index);

};

#endif
