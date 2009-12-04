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


// Source for CPrefsDisplayServer class

#include "CPrefsDisplayServer.h"

#include "CPreferences.h"

#include <LCheckBox.h>
#include <LGAColorSwatchControl.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsDisplayServer::CPrefsDisplayServer()
{
}

// Constructor from stream
CPrefsDisplayServer::CPrefsDisplayServer(LStream *inStream)
		: CPrefsDisplayPanel(inStream)
{
}

// Default destructor
CPrefsDisplayServer::~CPrefsDisplayServer()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsDisplayServer::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsDisplayPanel::FinishCreateSelf();

	// Get controls
	GetControls(mOpen, paneid_DMSOpenColour, paneid_DMSOpenBold, paneid_DMSOpenItalic, paneid_DMSOpenStrike, paneid_DMSOpenUnderline);
	GetControls(mClosed, paneid_DMSClosedColour, paneid_DMSClosedBold, paneid_DMSClosedItalic, paneid_DMSClosedStrike, paneid_DMSClosedUnderline);

	mServerColour = (LGAColorSwatchControl*) FindPaneByID(paneid_DMSServerColour);
	mUseServer = (LCheckBox*) FindPaneByID(paneid_DMSUseServer);
	mFavouriteColour = (LGAColorSwatchControl*) FindPaneByID(paneid_DMSFavouriteColour);
	mUseFavourite = (LCheckBox*) FindPaneByID(paneid_DMSUseFavourite);
	mHierarchyColour = (LGAColorSwatchControl*) FindPaneByID(paneid_DMSHierarchyColour);
	mUseHierarchy = (LCheckBox*) FindPaneByID(paneid_DMSUseHierarchy);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsDisplayServerBtns);
}

// Handle buttons
void CPrefsDisplayServer::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_DMSUseServer:
		if (*(long*) ioParam)
			mServerColour->Enable();
		else
			mServerColour->Disable();
		break;
	case msg_DMSUseFavourite:
		if (*(long*) ioParam)
			mFavouriteColour->Enable();
		else
			mFavouriteColour->Disable();
		break;
	case msg_DMSUseHierarchy:
		if (*(long*) ioParam)
			mHierarchyColour->Enable();
		else
			mHierarchyColour->Disable();
		break;
	}
}

// Set prefs
void CPrefsDisplayServer::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	SetStyle(mOpen, copyPrefs->mServerOpenStyle.GetValue());
	SetStyle(mClosed, copyPrefs->mServerClosedStyle.GetValue());

	mServerColour->SetSwatchColor((RGBColor&) copyPrefs->mServerBkgndStyle.GetValue().color);
	mUseServer->SetValue(copyPrefs->mServerBkgndStyle.GetValue().style);
	if (!copyPrefs->mServerBkgndStyle.GetValue().style)
		mServerColour->Disable();
	mFavouriteColour->SetSwatchColor((RGBColor&) copyPrefs->mFavouriteBkgndStyle.GetValue().color);
	mUseFavourite->SetValue(copyPrefs->mFavouriteBkgndStyle.GetValue().style);
	if (!copyPrefs->mFavouriteBkgndStyle.GetValue().style)
		mFavouriteColour->Disable();
	mHierarchyColour->SetSwatchColor((RGBColor&) copyPrefs->mHierarchyBkgndStyle.GetValue().color);
	mUseHierarchy->SetValue(copyPrefs->mHierarchyBkgndStyle.GetValue().style);
	if (!copyPrefs->mHierarchyBkgndStyle.GetValue().style)
		mHierarchyColour->Disable();
}

// Force update of prefs
void CPrefsDisplayServer::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	SStyleTraits traits;

	// Copy info from panel into prefs
	copyPrefs->mServerOpenStyle.SetValue(GetStyle(mOpen, traits));
	copyPrefs->mServerClosedStyle.SetValue(GetStyle(mClosed, traits));

	mServerColour->GetSwatchColor(traits.color);
	traits.style = mUseServer->GetValue();
	copyPrefs->mServerBkgndStyle.SetValue(traits);

	mFavouriteColour->GetSwatchColor(traits.color);
	traits.style = mUseFavourite->GetValue();
	copyPrefs->mFavouriteBkgndStyle.SetValue(traits);

	mHierarchyColour->GetSwatchColor(traits.color);
	traits.style = mUseHierarchy->GetValue();
	copyPrefs->mHierarchyBkgndStyle.SetValue(traits);
}
