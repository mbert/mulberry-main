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


// Source for CPrefsDisplayMailbox class

#include "CPrefsDisplayMailbox.h"

#include "CPreferences.h"

#include <LCheckBox.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsDisplayMailbox::CPrefsDisplayMailbox()
{
}

// Constructor from stream
CPrefsDisplayMailbox::CPrefsDisplayMailbox(LStream *inStream)
		: CPrefsDisplayPanel(inStream)
{
}

// Default destructor
CPrefsDisplayMailbox::~CPrefsDisplayMailbox()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsDisplayMailbox::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsDisplayPanel::FinishCreateSelf();

	// Get controls
	GetControls(mRecent, paneid_DMBRecentColour, paneid_DMBRecentBold, paneid_DMBRecentItalic, paneid_DMBRecentStrike, paneid_DMBRecentUnderline);
	GetControls(mUnseen, paneid_DMBUnseenColour, paneid_DMBUnseenBold, paneid_DMBUnseenItalic, paneid_DMBUnseenStrike, paneid_DMBUnseenUnderline);
	GetControls(mOpen, paneid_DMBOpenColour, paneid_DMBOpenBold, paneid_DMBOpenItalic, paneid_DMBOpenStrike, paneid_DMBOpenUnderline);
	GetControls(mFavourite, paneid_DMBFavouriteColour, paneid_DMBFavouriteBold, paneid_DMBFavouriteItalic, paneid_DMBFavouriteStrike, paneid_DMBFavouriteUnderline);
	GetControls(mClosed, paneid_DMBClosedColour, paneid_DMBClosedBold, paneid_DMBClosedItalic, paneid_DMBClosedStrike, paneid_DMBClosedUnderline);

	mIgnoreRecent = (LCheckBox*) FindPaneByID(paneid_DMBIgnoreRecent);
}

// Set prefs
void CPrefsDisplayMailbox::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	SetStyle(mRecent, copyPrefs->mMboxRecentStyle.GetValue());
	SetStyle(mUnseen, copyPrefs->mMboxUnseenStyle.GetValue());
	SetStyle(mOpen, copyPrefs->mMboxOpenStyle.GetValue());
	SetStyle(mFavourite, copyPrefs->mMboxFavouriteStyle.GetValue());
	SetStyle(mClosed, copyPrefs->mMboxClosedStyle.GetValue());
	
	mIgnoreRecent->SetValue(copyPrefs->mIgnoreRecent.GetValue());
}

// Force update of prefs
void CPrefsDisplayMailbox::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	SStyleTraits traits;

	// Copy info from panel into prefs
	copyPrefs->mMboxRecentStyle.SetValue(GetStyle(mRecent, traits));
	copyPrefs->mMboxUnseenStyle.SetValue(GetStyle(mUnseen, traits));
	copyPrefs->mMboxOpenStyle.SetValue(GetStyle(mOpen, traits));
	copyPrefs->mMboxFavouriteStyle.SetValue(GetStyle(mFavourite, traits));
	copyPrefs->mMboxClosedStyle.SetValue(GetStyle(mClosed, traits));

	copyPrefs->mIgnoreRecent.SetValue(mIgnoreRecent->GetValue());
}
