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


// Source for CPrefsMailboxBasic class

#include "CPrefsMailboxBasic.h"

#include "CMailboxPopup.h"
//#include "CMbox.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LCheckBoxGroupBox.h>
#include <LRadioButton.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsMailboxBasic::CPrefsMailboxBasic()
{
}

// Constructor from stream
CPrefsMailboxBasic::CPrefsMailboxBasic(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsMailboxBasic::~CPrefsMailboxBasic()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsMailboxBasic::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Set backgrounds
	mOpenFirst = (LRadioButton*) FindPaneByID(paneid_MBOpenFirst);
	mOpenLast = (LRadioButton*) FindPaneByID(paneid_MBOpenLast);
	mOpenFirstNew = (LRadioButton*) FindPaneByID(paneid_MBOpenFirstNew);
	mNoOpenPreview = (LCheckBox*) FindPaneByID(paneid_MBNoOpenPreview);

	mOldestToNewest = (LRadioButton*) FindPaneByID(paneid_MBOldestToNewest);
	mNewestToOldest = (LRadioButton*) FindPaneByID(paneid_MBNewestToOldest);

	mExpungeOnClose = (LCheckBox*) FindPaneByID(paneid_MBExpungeOnClose);
	mWarnOnExpunge = (LCheckBox*) FindPaneByID(paneid_MBWarnOnExpunge);

	mWarnPuntUnseen = (LCheckBox*) FindPaneByID(paneid_MBWarnPuntUnseen);

	mDoRollover = (LCheckBox*) FindPaneByID(paneid_MBDoRollover);
	mRolloverWarn = (LCheckBox*) FindPaneByID(paneid_MBRolloverWarn);

	mScrollForUnseen = (LCheckBox*) FindPaneByID(paneid_MBScrollForUnseen);

	mMoveFromINBOX = (LCheckBoxGroupBox*) FindPaneByID(paneid_MBMoveFromINBOX);

	mClearMailbox = (CTextFieldX*) FindPaneByID(paneid_MBClearMailbox);
	mClearMailboxPopup = (CMailboxPopup*) FindPaneByID(paneid_MBClearMailboxPopup);
	mClearMailboxPopup->SetCopyTo(true);

	mWarnOnClear = (LCheckBox*) FindPaneByID(paneid_MBWarnOnClear);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsMailboxBasicBtns);
}

// Handle buttons
void CPrefsMailboxBasic::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
		case msg_MBAllowMove:
			if (*((long*) ioParam))
			{
				mClearMailbox->Enable();
				mClearMailboxPopup->Enable();
				mWarnOnClear->Enable();
				LCommander::SwitchTarget(mClearMailbox);
				mClearMailbox->SelectAll();
			}
			else
			{
				mClearMailbox->Disable();
				mClearMailboxPopup->Disable();
				mWarnOnClear->Disable();
			}
			Refresh();
			break;

		case msg_MBClearMailboxPopup:
			{
				cdstring mbox_name;
				if (mClearMailboxPopup->GetSelectedMboxName(mbox_name))
				{
					if (mbox_name.empty())
					{
						mClearMailbox->SetText(cdstring::null_str);
					}
					else if (*mbox_name.c_str() != 0x01)
					{
						mClearMailbox->SetText(mbox_name);
					}
				}
			}
			break;
	}
}

// Set prefs
void CPrefsMailboxBasic::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// No messages
	StopListening();

	// Copy info
	mOpenFirst->SetValue((copyPrefs->openAtFirst.GetValue() ? Button_On : Button_Off));
	mOpenLast->SetValue((copyPrefs->openAtLast.GetValue() ? Button_On : Button_Off));
	mOpenFirstNew->SetValue((copyPrefs->openAtFirstNew.GetValue() ? Button_On : Button_Off));
	mNoOpenPreview->SetValue((copyPrefs->mNoOpenPreview.GetValue() ? Button_On : Button_Off));

	mOldestToNewest->SetValue(copyPrefs->mNextIsNewest.GetValue() ? Button_On : Button_Off);
	mNewestToOldest->SetValue(copyPrefs->mNextIsNewest.GetValue() ? Button_Off : Button_On);

	mExpungeOnClose->SetValue((copyPrefs->expungeOnClose.GetValue() ? Button_On : Button_Off));
	mWarnOnExpunge->SetValue((copyPrefs->warnOnExpunge.GetValue() ? Button_On : Button_Off));

	mWarnPuntUnseen->SetValue(copyPrefs->mWarnPuntUnseen.GetValue());

	mDoRollover->SetValue((copyPrefs->mDoRollover.GetValue() ? Button_On : Button_Off));
	mRolloverWarn->SetValue((copyPrefs->mRolloverWarn.GetValue() ? Button_On : Button_Off));

	mScrollForUnseen->SetValue((copyPrefs->mScrollForUnseen.GetValue() ? Button_On : Button_Off));

	mClearMailbox->SetText(copyPrefs->clear_mailbox.GetValue());

	mWarnOnClear->SetValue((copyPrefs->clear_warning.GetValue() ? Button_On : Button_Off));

	mMoveFromINBOX->SetValue((copyPrefs->mDoMailboxClear.GetValue() ? Button_On : Button_Off));

	if (!copyPrefs->mDoMailboxClear.GetValue())
	{
		mClearMailbox->Disable();
		mClearMailboxPopup->Disable();
		mWarnOnClear->Disable();
	}

	// Allow messages
	StartListening();
}

// Force update of prefs
void CPrefsMailboxBasic::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->openAtFirst.SetValue((mOpenFirst->GetValue() == Button_On));
	copyPrefs->openAtLast.SetValue((mOpenLast->GetValue() == Button_On));
	copyPrefs->openAtFirstNew.SetValue((mOpenFirstNew->GetValue() == Button_On));
	copyPrefs->mNoOpenPreview.SetValue((mNoOpenPreview->GetValue() == Button_On));

	copyPrefs->mNextIsNewest.SetValue(mOldestToNewest->GetValue() == Button_On);

	copyPrefs->expungeOnClose.SetValue((mExpungeOnClose->GetValue() == Button_On));
	copyPrefs->warnOnExpunge.SetValue((mWarnOnExpunge->GetValue() == Button_On));

	copyPrefs->mWarnPuntUnseen.SetValue(mWarnPuntUnseen->GetValue());

	copyPrefs->mDoRollover.SetValue((mDoRollover->GetValue() == Button_On));
	copyPrefs->mRolloverWarn.SetValue((mRolloverWarn->GetValue() == Button_On));

	copyPrefs->mScrollForUnseen.SetValue((mScrollForUnseen->GetValue() == Button_On));

	copyPrefs->mDoMailboxClear.SetValue((mMoveFromINBOX->GetValue() == Button_On));

	copyPrefs->clear_mailbox.SetValue(mClearMailbox->GetText());

	copyPrefs->clear_warning.SetValue((mWarnOnClear->GetValue() == Button_On));
}
