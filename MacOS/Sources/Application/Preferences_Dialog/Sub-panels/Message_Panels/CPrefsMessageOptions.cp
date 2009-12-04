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


// Source for CPrefsMessageOptions class

#include "CPrefsMessageOptions.h"

#include "CAdminLock.h"
#include "CPreferences.h"

#include <LCheckBox.h>
#include <LRadioButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S M E S S A G E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsMessageOptions::CPrefsMessageOptions()
{
}

// Constructor from stream
CPrefsMessageOptions::CPrefsMessageOptions(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsMessageOptions::~CPrefsMessageOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsMessageOptions::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Set backgrounds
	mShowHeader = (LCheckBox*) FindPaneByID(paneid_ShowHeader);
	mSaveHeader = (LCheckBox*) FindPaneByID(paneid_SaveHeader);
	mDeleteAfterCopy = (LCheckBox*) FindPaneByID(paneid_DeleteAfterCopy);
	mOpenDeleted = (LCheckBox*) FindPaneByID(paneid_OpenDeleted);
	mCloseDeleted = (LCheckBox*) FindPaneByID(paneid_CloseDeleted);
	mOpenReuse = (LCheckBox*) FindPaneByID(paneid_OpenReuse);
	mQuoteSelection = (LCheckBox*) FindPaneByID(paneid_QuoteSelection);
	mAlwaysQuote = (LCheckBox*) FindPaneByID(paneid_AlwaysQuote);
	mAutoDigest = (LCheckBox*) FindPaneByID(paneid_AutoDigest);
	mExpandHeader = (LCheckBox*) FindPaneByID(paneid_ExpandHeader);
	mExpandParts = (LCheckBox*) FindPaneByID(paneid_ExpandParts);
	mShowStyled = (LCheckBox*) FindPaneByID(paneid_ShowStyled);
	mMDNAlwaysSend = (LRadioButton*) FindPaneByID(paneid_MDNAlwaysSend);
	mMDNNeverSend = (LRadioButton*) FindPaneByID(paneid_MDNNeverSend);
	mMDNPromptSend = (LRadioButton*) FindPaneByID(paneid_MDNPromptSend);
}

// Set prefs
void CPrefsMessageOptions::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	mShowHeader->SetValue((copyPrefs->showMessageHeader.GetValue()) ? 1 : 0);
	mSaveHeader->SetValue((copyPrefs->saveMessageHeader.GetValue()) ? 1 : 0);
	mShowStyled->SetValue((copyPrefs->showStyled.GetValue()) ? 1 : 0);
	mDeleteAfterCopy->SetValue((copyPrefs->deleteAfterCopy.GetValue()) ? 1 : 0);
	mOpenDeleted->SetValue((copyPrefs->mOpenDeleted.GetValue()) ? 1 : 0);
	mCloseDeleted->SetValue((copyPrefs->mCloseDeleted.GetValue()) ? 1 : 0);
	mOpenReuse->SetValue((copyPrefs->mOpenReuse.GetValue()) ? 1 : 0);
	mQuoteSelection->SetValue((copyPrefs->mQuoteSelection.GetValue()) ? 1 : 0);
	mAlwaysQuote->SetValue((copyPrefs->mAlwaysQuote.GetValue()) ? 1 : 0);
	mAutoDigest->SetValue((copyPrefs->mAutoDigest.GetValue()) ? 1 : 0);
	mExpandHeader->SetValue((copyPrefs->mExpandHeader.GetValue()) ? 1 : 0);
	mExpandParts->SetValue((copyPrefs->mExpandParts.GetValue()) ? 1 : 0);
	
	// Admin lock
	if (CAdminLock::sAdminLock.mLockMDN)
	{
		mMDNAlwaysSend->SetValue(1);
		mMDNNeverSend->SetValue(0);
		mMDNPromptSend->SetValue(0);
		
		mMDNAlwaysSend->Disable();
		mMDNNeverSend->Disable();
		mMDNPromptSend->Disable();
	}
	else
	{
		mMDNAlwaysSend->SetValue(copyPrefs->mMDNOptions.GetValue().GetValue() == eMDNAlwaysSend);
		mMDNNeverSend->SetValue(copyPrefs->mMDNOptions.GetValue().GetValue() == eMDNNeverSend);
		mMDNPromptSend->SetValue(copyPrefs->mMDNOptions.GetValue().GetValue() == eMDNPromptSend);
	}
}

// Force update of prefs
void CPrefsMessageOptions::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->showMessageHeader.SetValue((mShowHeader->GetValue()==1));
	copyPrefs->saveMessageHeader.SetValue((mSaveHeader->GetValue()==1));
	copyPrefs->showStyled.SetValue((mShowStyled->GetValue()==1));
	copyPrefs->deleteAfterCopy.SetValue((mDeleteAfterCopy->GetValue()==1));
	copyPrefs->mOpenDeleted.SetValue((mOpenDeleted->GetValue()==1));
	copyPrefs->mCloseDeleted.SetValue((mCloseDeleted->GetValue()==1));
	copyPrefs->mOpenReuse.SetValue((mOpenReuse->GetValue()==1));
	copyPrefs->mQuoteSelection.SetValue((mQuoteSelection->GetValue()==1));
	copyPrefs->mAlwaysQuote.SetValue((mAlwaysQuote->GetValue()==1));
	copyPrefs->mAutoDigest.SetValue((mAutoDigest->GetValue()==1));
	copyPrefs->mExpandHeader.SetValue((mExpandHeader->GetValue()==1));
	copyPrefs->mExpandParts.SetValue((mExpandParts->GetValue()==1));
	
	// Admin lock
	if (CAdminLock::sAdminLock.mLockMDN || mMDNAlwaysSend->GetValue())
		copyPrefs->mMDNOptions.SetValue(eMDNAlwaysSend);
	else if (mMDNNeverSend->GetValue())
		copyPrefs->mMDNOptions.SetValue(eMDNNeverSend);
	else if (mMDNPromptSend->GetValue())
		copyPrefs->mMDNOptions.SetValue(eMDNPromptSend);
}
