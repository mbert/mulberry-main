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


// Header for CSynchroniseDialog class

#ifndef __CSYNCHRONISEDIALOG__MULBERRY__
#define __CSYNCHRONISEDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "templs.h"

// Constants

// Panes
const	PaneIDT		paneid_SynchroniseDialog = 1002;
const	PaneIDT		paneid_SynchroniseAll = 'MALL';
const	PaneIDT		paneid_SynchroniseNew = 'MNEW';
const	PaneIDT		paneid_SynchroniseSelected = 'MSEL';
const	PaneIDT		paneid_SynchroniseFull = 'FULL';
const	PaneIDT		paneid_SynchroniseBelow = 'BELO';
const	PaneIDT		paneid_SynchroniseSize = 'SIZE';
const	PaneIDT		paneid_SynchronisePartial = 'PART';

// Mesages
const	MessageT	msg_SynchroniseFull = 'FULL';
const	MessageT	msg_SynchroniseBelow = 'BELO';
const	MessageT	msg_SynchronisePartial = 'PART';

// Resources
const	ResIDT		RidL_CSynchroniseDialogBtns = 1002;

class LRadioButton;
class CMboxList;
class CMbox;
class CTextFieldX;

class	CSynchroniseDialog : public LDialogBox
{
private:
	LRadioButton*		mAll;
	LRadioButton*		mNew;
	LRadioButton*		mSelected;
	LRadioButton*		mFull;
	LRadioButton*		mBelow;
	CTextFieldX*		mSize;
	LRadioButton*		mPartial;

public:
	enum { class_ID = 'SyMa' };

					CSynchroniseDialog();
					CSynchroniseDialog(LStream *inStream);
	virtual 		~CSynchroniseDialog();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	virtual void	SetDetails(bool has_selection);								// Set the dialogs info
	virtual void	GetDetails(bool& fast, bool& partial, unsigned long& size, bool& selection);		// Get the dialogs return info

	static  void	PoseDialog(CMboxList* mbox_list);
	static  void	PoseDialog(CMbox* mbox, ulvector& selection);
	static  bool	PoseDialog(bool& fast, bool& partial, unsigned long& size, bool& selected);
};

#endif
