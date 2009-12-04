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


// Header for CNewSearchDialog class

#ifndef __CNEWSEARCHDIALOG__MULBERRY__
#define __CNEWSEARCHDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_NewSearchDialog = 4002;
const	PaneIDT		paneid_NewSearchAccount = 'ACCT';
const	PaneIDT		paneid_NewSearchCriteria = 'CRIT';
const	PaneIDT		paneid_NewSearchText = 'TEXT';
const	PaneIDT		paneid_NewSearchFullHierarchy = 'FULL';
const	PaneIDT		paneid_NewSearchTopHierarchy = 'TOPH';

// Mesages
const	MessageT	msg_NewSearchCriteria = 'CRIT';

// Resources
const	PaneIDT		RidL_CNewSearchDialogBtns = 4002;

class CMboxProtocol;
class CTextFieldX;
class LPopupButton;
class LRadioButton;

class CNewSearchDialog : public LDialogBox
{
public:
	enum { class_ID = 'NewH' };

					CNewSearchDialog();
					CNewSearchDialog(LStream *inStream);
	virtual 		~CNewSearchDialog();

	static bool PoseDialog(bool single, unsigned long& index, cdstring& wd);

	virtual	void	SetDetails(bool single, unsigned long index);
	virtual	void	GetDetails(cdstring& name, unsigned long& index);

protected:
	LPopupButton* mAccount;
	LPopupButton* mCriteria;
	CTextFieldX* mText;
	LRadioButton* mFullHierarchy;
	LRadioButton* mTopHierarchy;
	bool mSingle;

	virtual void	FinishCreateSelf(void);
	virtual void	ListenToMessage(MessageT inMessage,
											void *ioParam);	// Respond to clicks in the buttons
	
private:
	void InitAccountMenu(void);
};

#endif
