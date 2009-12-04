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

#include "CDialogDirector.h"

#include <vector>
#include "templs.h"

// Classes
class JXCardFile;
class JXWidgetSet;

class CPropDialog : public CDialogDirector
{
public:
	CPropDialog(JXDirector* supervisor);
	virtual ~CPropDialog();

	virtual bool	DoOK(void);														// Handle success OK
	virtual void	DoCancel(void);													// Handle cancel

protected:
	JXCardFile*				mPanelContainer;									// The container panel
	JXWidgetSet*			mCurrentPanel;										// Current panel view
	short					mCurrentPanelNum;									// Current prefs panel index
	std::vector<JXWidgetSet*>	mPanelList;											// List of panel ids
	ulvector				mMsgPanelList;										// List of panel messages

	virtual void	OnCreate();												// Do odds & ends

	virtual void	SetUpPanels(void) = 0;											// About to start dialog
	virtual void	DoPanelInit(void) = 0;											// About to display a panel
			void	SetPanel(short panel);											// Set input panel

private:
	void	InitPropDialog(void);											// Standard init

};

#endif
