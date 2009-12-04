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
#include "CPreferencesDialog.h"

#include "JXColorButton.h"

#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsDisplayMailbox::CPrefsDisplayMailbox(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	: CPrefsDisplayPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsDisplayMailbox::OnCreate()
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
        new JXStaticText("Recent:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,33, 67,20);
    assert( obj6 != NULL );

    mRecentColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,30, 40,20);
    assert( mRecentColour != NULL );

    mRecentBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,30, 20,20);
    assert( mRecentBold != NULL );

    mRecentItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,30, 20,20);
    assert( mRecentItalic != NULL );

    mRecentStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,30, 20,20);
    assert( mRecentStrike != NULL );

    mRecentUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,30, 20,20);
    assert( mRecentUnderline != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Unseen:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,58, 77,20);
    assert( obj7 != NULL );

    mUnseenColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,55, 40,20);
    assert( mUnseenColour != NULL );

    mUnseenBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,55, 20,20);
    assert( mUnseenBold != NULL );

    mUnseenItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,55, 20,20);
    assert( mUnseenItalic != NULL );

    mUnseenStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,55, 20,20);
    assert( mUnseenStrike != NULL );

    mUnseenUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,55, 20,20);
    assert( mUnseenUnderline != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Open:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,83, 77,20);
    assert( obj8 != NULL );

    mOpenColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,80, 40,20);
    assert( mOpenColour != NULL );

    mOpenBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,80, 20,20);
    assert( mOpenBold != NULL );

    mOpenItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,80, 20,20);
    assert( mOpenItalic != NULL );

    mOpenStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,80, 20,20);
    assert( mOpenStrike != NULL );

    mOpenUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,80, 20,20);
    assert( mOpenUnderline != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Favourite:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,108, 77,20);
    assert( obj9 != NULL );

    mFavouriteColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,105, 40,20);
    assert( mFavouriteColour != NULL );

    mFavouriteBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,105, 20,20);
    assert( mFavouriteBold != NULL );

    mFavouriteItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,105, 20,20);
    assert( mFavouriteItalic != NULL );

    mFavouriteStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,105, 20,20);
    assert( mFavouriteStrike != NULL );

    mFavouriteUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,105, 20,20);
    assert( mFavouriteUnderline != NULL );

    JXStaticText* obj10 =
        new JXStaticText("Closed:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,133, 77,20);
    assert( obj10 != NULL );

    mClosedColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,130, 40,20);
    assert( mClosedColour != NULL );

    mClosedBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,130, 20,20);
    assert( mClosedBold != NULL );

    mClosedItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,130, 20,20);
    assert( mClosedItalic != NULL );

    mClosedStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,130, 20,20);
    assert( mClosedStrike != NULL );

    mClosedUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,130, 20,20);
    assert( mClosedUnderline != NULL );

    mIgnoreRecent =
        new JXTextCheckbox("Use Recent State only for Recent and Unseen", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,170, 295,20);
    assert( mIgnoreRecent != NULL );

// end JXLayout1

	// Get controls
	GetControls(mRecent, mRecentColour, mRecentBold, mRecentItalic, mRecentStrike, mRecentUnderline);
	GetControls(mUnseen, mUnseenColour, mUnseenBold, mUnseenItalic, mUnseenStrike, mUnseenUnderline);
	GetControls(mOpen, mOpenColour, mOpenBold, mOpenItalic, mOpenStrike, mOpenUnderline);
	GetControls(mFavourite, mFavouriteColour, mFavouriteBold, mFavouriteItalic, mFavouriteStrike, mFavouriteUnderline);
	GetControls(mClosed, mClosedColour, mClosedBold, mClosedItalic, mClosedStrike, mClosedUnderline);

	mRecentColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mUnseenColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mOpenColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mFavouriteColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mClosedColour->SetDirector(CPreferencesDialog::sPrefsDlog);
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
	
	mIgnoreRecent->SetState(JBoolean(copyPrefs->mIgnoreRecent.GetValue()));
}

// Force update of prefs
bool CPrefsDisplayMailbox::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	SStyleTraits traits;

	// Copy info from panel into prefs
	copyPrefs->mMboxRecentStyle.SetValue(GetStyle(mRecent, traits));
	copyPrefs->mMboxUnseenStyle.SetValue(GetStyle(mUnseen, traits));
	copyPrefs->mMboxOpenStyle.SetValue(GetStyle(mOpen, traits));
	copyPrefs->mMboxFavouriteStyle.SetValue(GetStyle(mFavourite, traits));
	copyPrefs->mMboxClosedStyle.SetValue(GetStyle(mClosed, traits));

	copyPrefs->mIgnoreRecent.SetValue(mIgnoreRecent->IsChecked());
	
	return true;
}
