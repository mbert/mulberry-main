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


// Header for CMailboxPropDialog class

#ifndef __CMAILBOXPROPDIALOG__MULBERRY__
#define __CMAILBOXPROPDIALOG__MULBERRY__

#include "CPropDialog.h"

// Classes
class CMboxList;
class CMboxProtocol;
class JXTextButton;
class JXIconTextButton;

class CMailboxPropDialog : public CPropDialog
{
public:
	
	CMailboxPropDialog(JXDirector* supervisor);

	static bool PoseDialog(CMboxList* mbox_list)
		{ return PoseDialog(mbox_list, NULL); }
	static bool PoseDialog(CMboxProtocol* proto)
		{ return PoseDialog(NULL, proto); }
	static bool PoseDialog(CMboxList* mbox_list, CMboxProtocol* proto);

protected:
// begin JXLayout

    JXIconTextButton* mOptionsBtn;
    JXIconTextButton* mGeneralBtn;
    JXIconTextButton* mServerBtn;
    JXIconTextButton* mAccessBtn;
    JXIconTextButton* mQuotaBtn;
    JXCardFile*       mCards;
    JXTextButton*     mOKBtn;

// end JXLayout
	virtual void	OnCreate();								// Do odds & ends
	virtual void	Receive(JBroadcaster* sender, const Message& message);

	virtual void	SetUpPanels(void);									// About to start dialog
	virtual void	DoPanelInit(void);									// About to display a panel

private:
	CMboxList*		mMboxList;											// List of selected mboxes
	CMboxProtocol*		mMboxProtocol;									// List of selected mboxes

	void	SetMboxList(CMboxList* mbox_list);							// Set mbox list
	void	SetProtocol(CMboxProtocol* proto);							// Set mbox protocol
};

#endif
