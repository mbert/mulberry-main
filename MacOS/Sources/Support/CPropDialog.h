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


// Header for CPropDialog class

#ifndef __CPROPDIALOG__MULBERRY__
#define __CPROPDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "templs.h"

// Constants

// Panes
const	PaneIDT		paneid_PropPanel = 'AREA';
const	PaneIDT		paneid_PropCancelBtn = 'CANC';
const	PaneIDT		paneid_PropOKBtn = 'OKBT';

// Resources

// Mesages

// Classes

class CPropDialog : public LDialogBox
{
public:
					CPropDialog();
					CPropDialog(LStream *inStream);
	virtual 		~CPropDialog();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	virtual bool	DoOK(void);														// Handle success OK
	virtual void	DoCancel(void);													// Handle cancel

protected:
	LView*				mPanelContainer;									// The container panel
	LView*				mCurrentPanel;										// Current panel view
	short				mCurrentPanelNum;									// Current prefs panel index
	ResIDT				mRidl;												// Ridl resource id
	ulvector			mPanelList;											// List of panel ids
	ulvector			mMsgPanelList;										// List of panel messages

	virtual void	FinishCreateSelf(void);											// Do odds & ends

	virtual void	SetUpPanels(void) = 0;											// About to start dialog
	virtual void	DoPanelInit(void) = 0;											// About to display a panel

private:
	void	InitPropDialog(void);											// Standard init
	void	SetPanel(short panel);											// Set input panel

};

#endif
