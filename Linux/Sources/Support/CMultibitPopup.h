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


// Header for CMultibitPopup class

#ifndef __CMULTIBITPOPUP__MULBERRY__
#define __CMULTIBITPOPUP__MULBERRY__

#include "TPopupMenu.h"
#include "HPopupMenu.h"

#include "templs.h"

// Classes

class CMultibitPopup : public HPopupMenu
{

public:
			CMultibitPopup(const JCharacter* title, JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y, const JCoordinate w, const JCoordinate h);
	virtual ~CMultibitPopup();

	virtual void Receive(JBroadcaster* sender, const Message& message);

	void InitBits();
	boolvector& GetBits()
		{ return mBits; }
	const boolvector& GetBits() const
		{ return mBits; }

	void SetBit(JIndex index, bool set = true);
	bool GetBit(JIndex index) const;
	void ToggleBit(JIndex index);

 protected:
	boolvector	mBits;

	virtual void	SetupCurrentMenuItem(); // Check items before doing popup
	virtual void	AdjustPopupChoiceTitle(const JIndex index);
};

#endif
