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


// Source for CPrefsMailboxAdvanced class

#include "CPrefsMailboxAdvanced.h"

#include "CCopyToMenu.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LRadioButton.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsMailboxAdvanced::CPrefsMailboxAdvanced()
{
}

// Constructor from stream
CPrefsMailboxAdvanced::CPrefsMailboxAdvanced(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsMailboxAdvanced::~CPrefsMailboxAdvanced()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsMailboxAdvanced::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Set backgrounds
	mUnseenNew = (LRadioButton*) FindPaneByID(paneid_MAUnseenNew);
	mRecentNew = (LRadioButton*) FindPaneByID(paneid_MARecentNew);
	mRecentUnseenNew = (LRadioButton*) FindPaneByID(paneid_MARecentUnseenNew);

	mFavouriteCopyTo = (LCheckBox*) FindPaneByID(paneid_MAFavouriteCopyTo);
	mFavouriteAppendTo = (LCheckBox*) FindPaneByID(paneid_MAFavouriteAppendTo);
	mMaximumMRU = (CTextFieldX*) FindPaneByID(paneid_MAMaximumMRU);

	mRLoCache = (CTextFieldX*) FindPaneByID(paneid_MARLoCache);

	mRNoLimit = (LRadioButton*) FindPaneByID(paneid_MARNoCacheLimit);
	mRUseHiCache = (LRadioButton*) FindPaneByID(paneid_MARUseHiCache);
	mRHiCache = (CTextFieldX*) FindPaneByID(paneid_MARHiCache);

	mRAutoResize = (LRadioButton*) FindPaneByID(paneid_MARAutoResize);
	mRUseIncrement = (LRadioButton*) FindPaneByID(paneid_MARUseIncrement);
	mRCacheIncrement = (CTextFieldX*) FindPaneByID(paneid_MARCacheIncrement);

	mRSortCache = (CTextFieldX*) FindPaneByID(paneid_MARSortCache);

	mLLoCache = (CTextFieldX*) FindPaneByID(paneid_MALLoCache);

	mLNoLimit = (LRadioButton*) FindPaneByID(paneid_MALNoCacheLimit);
	mLUseHiCache = (LRadioButton*) FindPaneByID(paneid_MALUseHiCache);
	mLHiCache = (CTextFieldX*) FindPaneByID(paneid_MALHiCache);

	mLAutoResize = (LRadioButton*) FindPaneByID(paneid_MALAutoResize);
	mLUseIncrement = (LRadioButton*) FindPaneByID(paneid_MALUseIncrement);
	mLCacheIncrement = (CTextFieldX*) FindPaneByID(paneid_MALCacheIncrement);

	mLSortCache = (CTextFieldX*) FindPaneByID(paneid_MALSortCache);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsMailboxAdvancedBtns);
}

// Handle buttons
void CPrefsMailboxAdvanced::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_MAFavouriteCopyTo:
	case msg_MAFavouriteAppendTo:
		// Must force reset of menu lists
		CCopyToMenu::SetMenuOptions(mFavouriteCopyTo->GetValue() == Button_On,
									mFavouriteAppendTo->GetValue() == Button_On);

		// Also set flag to do menu reset on dialog exit (OK or Cancel)
		CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
		while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
			prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
		prefs_dlog->SetForceMenuListReset(true);
		break;

	case msg_MARNoCacheLimit:
		if (*((long*) ioParam))
			mRHiCache->Disable();
		break;

	case msg_MARUseHiCache:
		if (*((long*) ioParam))
		{
			mRHiCache->Enable();
			LCommander::SwitchTarget(mRHiCache);
			mRHiCache->SelectAll();
		}
		break;

	case msg_MARAutoResize:
		if (*((long*) ioParam))
			mRCacheIncrement->Disable();
		break;

	case msg_MARUseIncrement:
		if (*((long*) ioParam))
		{
			mRCacheIncrement->Enable();
			LCommander::SwitchTarget(mRCacheIncrement);
			mRCacheIncrement->SelectAll();
		}
		break;

	case msg_MALNoCacheLimit:
		if (*((long*) ioParam))
			mLHiCache->Disable();
		break;

	case msg_MALUseHiCache:
		if (*((long*) ioParam))
		{
			mLHiCache->Enable();
			LCommander::SwitchTarget(mLHiCache);
			mLHiCache->SelectAll();
		}
		break;

	case msg_MALAutoResize:
		if (*((long*) ioParam))
			mLCacheIncrement->Disable();
		break;

	case msg_MALUseIncrement:
		if (*((long*) ioParam))
		{
			mLCacheIncrement->Enable();
			LCommander::SwitchTarget(mLCacheIncrement);
			mLCacheIncrement->SelectAll();
		}
		break;
	}
}

// Set prefs
void CPrefsMailboxAdvanced::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// No messages
	StopListening();

	// Copy info
	mUnseenNew->SetValue(copyPrefs->mUnseenNew.GetValue() && !copyPrefs->mRecentNew.GetValue() ? Button_On : Button_Off);
	mRecentNew->SetValue(!copyPrefs->mUnseenNew.GetValue() && copyPrefs->mRecentNew.GetValue() ? Button_On : Button_Off);
	mRecentUnseenNew->SetValue(copyPrefs->mUnseenNew.GetValue() && copyPrefs->mRecentNew.GetValue() ? Button_On : Button_Off);

	mFavouriteCopyTo->SetValue((copyPrefs->mUseCopyToMboxes.GetValue() ? Button_On : Button_Off));
	mFavouriteAppendTo->SetValue((copyPrefs->mUseAppendToMboxes.GetValue() ? Button_On : Button_Off));
	mMaximumMRU->SetNumber(copyPrefs->mMRUMaximum.GetValue());

	mRLoCache->SetNumber(copyPrefs->mRLoCache.GetValue());

	mRHiCache->SetNumber(copyPrefs->mRHiCache.GetValue());
	if (copyPrefs->mRUseHiCache.GetValue())
	{
		mRUseHiCache->SetValue(1);
		mRHiCache->Enable();
	}
	else
	{
		mRNoLimit->SetValue(1);
		mRHiCache->Disable();
	}

	mRCacheIncrement->SetNumber(copyPrefs->mRCacheIncrement.GetValue());
	if (copyPrefs->mRAutoCacheIncrement.GetValue())
	{
		mRAutoResize->SetValue(1);
		mRCacheIncrement->Disable();
	}
	else
	{
		mRUseIncrement->SetValue(1);
		mRCacheIncrement->Enable();
	}

	mRSortCache->SetNumber(copyPrefs->mRSortCache.GetValue());

	mLLoCache->SetNumber(copyPrefs->mLLoCache.GetValue());

	mLHiCache->SetNumber(copyPrefs->mLHiCache.GetValue());
	if (copyPrefs->mLUseHiCache.GetValue())
	{
		mLUseHiCache->SetValue(1);
		mLHiCache->Enable();
	}
	else
	{
		mLNoLimit->SetValue(1);
		mLHiCache->Disable();
	}

	mLCacheIncrement->SetNumber(copyPrefs->mLCacheIncrement.GetValue());
	if (copyPrefs->mLAutoCacheIncrement.GetValue())
	{
		mLAutoResize->SetValue(1);
		mLCacheIncrement->Disable();
	}
	else
	{
		mLUseIncrement->SetValue(1);
		mLCacheIncrement->Enable();
	}

	mLSortCache->SetNumber(copyPrefs->mLSortCache.GetValue());

	// Allow messages
	StartListening();
}

// Force update of prefs
void CPrefsMailboxAdvanced::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->mUnseenNew.SetValue((mUnseenNew->GetValue() == Button_On) || (mRecentUnseenNew->GetValue() == Button_On));
	copyPrefs->mRecentNew.SetValue((mRecentNew->GetValue() == Button_On) || (mRecentUnseenNew->GetValue() == Button_On));

	copyPrefs->mUseCopyToMboxes.SetValue(mFavouriteCopyTo->GetValue() == Button_On);
	copyPrefs->mUseAppendToMboxes.SetValue(mFavouriteAppendTo->GetValue() == Button_On);
	copyPrefs->mMRUMaximum.SetValue(mMaximumMRU->GetNumber());

	copyPrefs->mRLoCache.SetValue(mRLoCache->GetNumber());

	copyPrefs->mRUseHiCache.SetValue((mRUseHiCache->GetValue() == 1));
	copyPrefs->mRHiCache.SetValue(mRHiCache->GetNumber());

	copyPrefs->mRAutoCacheIncrement.SetValue((mRAutoResize->GetValue() == 1));
	copyPrefs->mRCacheIncrement.SetValue(mRCacheIncrement->GetNumber());

	copyPrefs->mRSortCache.SetValue(mRSortCache->GetNumber());

	copyPrefs->mLLoCache.SetValue(mLLoCache->GetNumber());

	copyPrefs->mLUseHiCache.SetValue((mLUseHiCache->GetValue() == 1));
	copyPrefs->mLHiCache.SetValue(mLHiCache->GetNumber());

	copyPrefs->mLAutoCacheIncrement.SetValue((mLAutoResize->GetValue() == 1));
	copyPrefs->mLCacheIncrement.SetValue(mLCacheIncrement->GetNumber());

	copyPrefs->mLSortCache.SetValue(mLSortCache->GetNumber());
}
