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


// Header for CPrefsEditMappings class

#ifndef __CPREFSEDITMAPPINGS__MULBERRY__
#define __CPREFSEDITMAPPINGS__MULBERRY__

#include <LDialogBox.h>


#include "CMessageWindow.h"
#include "CMIMEMap.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsEditMappings = 5120;
const	PaneIDT		paneid_MappingsTable = 'TABL';
const	PaneIDT		paneid_MappingsNewBtn = 'NEWB';
const	PaneIDT		paneid_MappingsChangeBtn = 'CHNB';
const	PaneIDT		paneid_MappingsDeleteBtn = 'DELB';

// Mesages
const	MessageT	msg_NewMapping = 'NEWB';
const	MessageT	msg_ChangeMapping = 'CHNB';
const	MessageT	msg_DeleteMapping = 'DELB';

// Resources
const	ResIDT		RidL_CPrefsEditMappingsBtns = 5120;

// Classes

class	CEditMappingsTable;
class	LPushButton;

class	CPrefsEditMappings : public LDialogBox
{
public:
	enum { class_ID = 'MAPP' };

					CPrefsEditMappings();
					CPrefsEditMappings(LStream *inStream);
	virtual 		~CPrefsEditMappings();

	static bool		PoseDialog(CMIMEMapVector* mappings);

			void	SetMappings(CMIMEMapVector* mappings);
			void	ShowMapping(unsigned long mapping);

	virtual void	ListenToMessage(MessageT inMessage,
											void *ioParam);	// Respond to clicks in the buttons
protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

private:
	CMIMEMapVector*		mMappings;
	CEditMappingsTable*	mItsTable;
	LPushButton*		mNewBtn;
	LPushButton*		mChangeBtn;
	LPushButton*		mDeleteBtn;
	unsigned long		mCurrentMap;
	
			void	DoNewMapping();				// Create a new mapping entry
			void	DoChangeMapping();			// Change the selected entry
			void	DoDeleteMapping();			// Delete the selected entry
};

#endif
