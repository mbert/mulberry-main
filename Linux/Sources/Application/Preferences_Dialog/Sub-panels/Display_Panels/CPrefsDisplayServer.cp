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
#include "CPreferencesDialog.h"

#include "JXColorButton.h"

#include <JXEngravedRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsDisplayServer::CPrefsDisplayServer(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	: CPrefsDisplayPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsDisplayServer::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Colour:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,10, 50,20);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Bold", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 150,10, 35,20);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Italic", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,10, 35,20);
    assert( obj3 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Strike", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 227,10, 40,20);
    assert( obj4 != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Under", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 266,10, 45,20);
    assert( obj5 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Logged in:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,33, 67,20);
    assert( obj6 != NULL );

    mOpenColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,30, 40,20);
    assert( mOpenColour != NULL );

    mOpenBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,30, 20,20);
    assert( mOpenBold != NULL );

    mOpenItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,30, 20,20);
    assert( mOpenItalic != NULL );

    mOpenStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,30, 20,20);
    assert( mOpenStrike != NULL );

    mOpenUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,30, 20,20);
    assert( mOpenUnderline != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Logged out:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,58, 77,20);
    assert( obj7 != NULL );

    mClosedColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,55, 40,20);
    assert( mClosedColour != NULL );

    mClosedBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,55, 20,20);
    assert( mClosedBold != NULL );

    mClosedItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,55, 20,20);
    assert( mClosedItalic != NULL );

    mClosedStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,55, 20,20);
    assert( mClosedStrike != NULL );

    mClosedUnder =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,55, 20,20);
    assert( mClosedUnder != NULL );

    JXEngravedRect* obj8 =
        new JXEngravedRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,85, 360,3);
    assert( obj8 != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Server:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,103, 67,20);
    assert( obj9 != NULL );

    mServerColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,100, 40,20);
    assert( mServerColour != NULL );

    mUseServerBtn =
        new JXTextCheckbox("Colour Background", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,100, 140,20);
    assert( mUseServerBtn != NULL );

    JXStaticText* obj10 =
        new JXStaticText("Cabinet:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,128, 67,20);
    assert( obj10 != NULL );

    mFavouriteColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,125, 40,20);
    assert( mFavouriteColour != NULL );

    mUseFavouriteBtn =
        new JXTextCheckbox("Colour Background", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,125, 140,20);
    assert( mUseFavouriteBtn != NULL );

    JXStaticText* obj11 =
        new JXStaticText("Hierarchy:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,153, 67,20);
    assert( obj11 != NULL );

    mHierarchyColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,150, 40,20);
    assert( mHierarchyColour != NULL );

    mUseHierarchyBtn =
        new JXTextCheckbox("Colour Background", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,150, 140,20);
    assert( mUseHierarchyBtn != NULL );

// end JXLayout1

	// Get controls
	GetControls(mOpen, mOpenColour, mOpenBold, mOpenItalic, mOpenStrike, mOpenUnderline);
	GetControls(mClosed, mClosedColour, mClosedBold, mClosedItalic, mClosedStrike, mClosedUnder);

	mOpenColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mClosedColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mServerColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mFavouriteColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mHierarchyColour->SetDirector(CPreferencesDialog::sPrefsDlog);

	// Start listening
	ListenTo(mUseServerBtn);
	ListenTo(mUseFavouriteBtn);
	ListenTo(mUseHierarchyBtn);
	
}

void CPrefsDisplayServer::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mUseServerBtn)
		{
			OnUseServer();
			return;
		}
		else if (sender == mUseFavouriteBtn)
		{
			OnUseFavourite();
			return;
		}
		else if (sender == mUseHierarchyBtn)
		{
			OnUseHierarchy();
			return;
		}
	}
	
	CPrefsDisplayPanel::Receive(sender, message);
}

// Set prefs
void CPrefsDisplayServer::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	SetStyle(mOpen, copyPrefs->mServerOpenStyle.GetValue());
	SetStyle(mClosed, copyPrefs->mServerClosedStyle.GetValue());

	mServerColour->SetColor(copyPrefs->mServerBkgndStyle.GetValue().color);
	mUseServerBtn->SetState(JBoolean(copyPrefs->mServerBkgndStyle.GetValue().style));
	if (!copyPrefs->mServerBkgndStyle.GetValue().style)
		mServerColour->Deactivate();

	mFavouriteColour->SetColor(copyPrefs->mFavouriteBkgndStyle.GetValue().color);
	mUseFavouriteBtn->SetState(JBoolean(copyPrefs->mFavouriteBkgndStyle.GetValue().style));
	if (!copyPrefs->mFavouriteBkgndStyle.GetValue().style)
		mFavouriteColour->Deactivate();

	mHierarchyColour->SetColor(copyPrefs->mHierarchyBkgndStyle.GetValue().color);
	mUseHierarchyBtn->SetState(JBoolean(copyPrefs->mHierarchyBkgndStyle.GetValue().style));
	if (!copyPrefs->mHierarchyBkgndStyle.GetValue().style)
		mHierarchyColour->Deactivate();
}

// Force update of prefs
bool CPrefsDisplayServer::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	SStyleTraits traits;

	// Copy info from panel into prefs
	copyPrefs->mServerOpenStyle.SetValue(GetStyle(mOpen, traits));
	copyPrefs->mServerClosedStyle.SetValue(GetStyle(mClosed, traits));

	traits.color = mServerColour->GetColor();
	traits.style = mUseServerBtn->IsChecked();
	copyPrefs->mServerBkgndStyle.SetValue(traits);

	traits.color = mFavouriteColour->GetColor();
	traits.style = mUseFavouriteBtn->IsChecked();
	copyPrefs->mFavouriteBkgndStyle.SetValue(traits);

	traits.color = mHierarchyColour->GetColor();
	traits.style = mUseHierarchyBtn->IsChecked();
	copyPrefs->mHierarchyBkgndStyle.SetValue(traits);
	
	return true;
}

void CPrefsDisplayServer::OnUseServer()
{
	if (mUseServerBtn->IsChecked())
		mServerColour->Activate();
	else
		mServerColour->Deactivate();
}

void CPrefsDisplayServer::OnUseFavourite()
{
	if (mUseFavouriteBtn->IsChecked())
		mFavouriteColour->Activate();
	else
		mFavouriteColour->Deactivate();
}

void CPrefsDisplayServer::OnUseHierarchy()
{
	if (mUseHierarchyBtn->IsChecked())
		mHierarchyColour->Activate();
	else
		mHierarchyColour->Deactivate();
}
