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


// CDisconnectDialog.h : header file
//

#ifndef __CDISCONNECTDIALOG__MULBERRY__
#define __CDISCONNECTDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "CMboxRefList.h"

#include "templs.h"

/////////////////////////////////////////////////////////////////////////////
// CDisconnectDialog dialog

class CBarPane;
class CDisconnectListPanel;
class JXFlatRect;
class JXIntegerInput;
class JXRadioGroup;
class JXTextButton;
class JXTextCheckbox;
template<class T> class CInputField;

class CDisconnectDialog : public CDialogDirector
{
// Construction
public:
	CDisconnectDialog(JXDirector* supervisor);
	virtual ~CDisconnectDialog();

	static  void	PoseDialog();

	static CDisconnectDialog* GetDisconnectDialog()
		{ return sDisconnectDialog; }

	CMboxRefList&	GetTargets()
		{ return mSyncTargets; }
	ulvector&		GetTargetHits()
		{ return mSyncTargetHits; }

	void 			NextItem(unsigned long item);
	void 			DoneItem(unsigned long item, bool hit);

protected:
	enum
	{
		eStayConnected = 0,
		eUpdateDisconnect
	};
	enum
	{
		eAll = 0,
		eNew,
		eNone
	};
	enum
	{
		eFull = 0,
		eBelow,
		ePartial
	};
	
	static CDisconnectDialog* sDisconnectDialog;

// begin JXLayout

    CDisconnectListPanel* mMailboxListPanel;
    JXTextButton*         mOKBtn;
    JXTextButton*         mCancelBtn;

// end JXLayout
// begin JXLayout2

    JXFlatRect*                  mConnectingPanel;
    JXRadioGroup*                mConnectingGroup;
    JXTextCheckbox*              mPlayback;
    JXTextCheckbox*              mPOP3;
    JXTextCheckbox*              mSend;
    JXTextCheckbox*              mAdbk1;
    JXFlatRect*                  mMessagePanel;
    JXRadioGroup*                mMessageGroup;
    CInputField<JXIntegerInput>* mSize;

// end JXLayout2
// begin JXLayout3

    JXFlatRect*     mDisconnectingPanel;
    JXRadioGroup*   mDisconnectingGroup;
    JXTextCheckbox* mList;
    JXTextCheckbox* mWait;
    JXTextCheckbox* mAdbk2;
    JXFlatRect*     mProgressPanel;
    JXRadioGroup*   mProgressGroup;
    CBarPane*       mProgress1;
    CBarPane*       mProgress2;

// end JXLayout3

	bool 				mConnecting;
	CMboxRefList		mSyncTargets;
	ulvector			mSyncTargetHits;
	static bool			sIsTwisted;

	virtual void	OnCreate();
	virtual void	Receive(JBroadcaster* sender, const Message& message);
	virtual JBoolean OKToDeactivate();

			void	SetDetails(bool connecting);								// Set the dialogs info
			void	GetDetailsDisconnect();										// Get the dialogs return info
			void	GetDetailsConnect();										// Get the dialogs return info

			void	ShowProgressPanel(bool show);			// Show or hide the progress panel

	// Generated message map functions
			void OnOK();
};

#endif
