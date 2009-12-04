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


// Header for CAddressNotesPanel class

#ifndef __CADDRESSNOTESPANEL__MULBERRY__
#define __CADDRESSNOTESPANEL__MULBERRY__

#include "CAddressPanelBase.h"

// Constants
const	PaneIDT		paneid_AddressNotes = 1737;
const	PaneIDT		paneid_AddressNotesNotes = 'NOTE';

// Messages

// Classes
class CTextDisplay;

class CAddressNotesPanel : public CAddressPanelBase
{
public:
	enum { class_ID = 'Anot' };

					CAddressNotesPanel(LStream *inStream);
	virtual 		~CAddressNotesPanel();

	virtual void	Focus();

protected:
	CTextDisplay*	mNotes;

	virtual void	FinishCreateSelf();					// Do odds & ends

	virtual void	SetFields(const CAdbkAddress* addr);	// Set fields in dialog
	virtual bool	GetFields(CAdbkAddress* addr);			// Get fields from dialog
};

#endif
