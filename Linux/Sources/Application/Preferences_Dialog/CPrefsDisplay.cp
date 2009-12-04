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


// CPrefsDisplay.cp : implementation file
//


#include "CPrefsDisplay.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"

#include "CTabController.h"

#include "CPrefsDisplayServer.h"
#include "CPrefsDisplayMailbox.h"
#include "CPrefsDisplayMessage.h"
#include "CPrefsDisplayLabel.h"
#include "CPrefsDisplayQuotes.h"
#include "CPrefsDisplayStyles.h"

#include <JXColormap.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplay property page

CPrefsDisplay::CPrefsDisplay(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	:CPrefsPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Set up params for DDX
void CPrefsDisplay::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;

	mTabs->SetData(prefs);
	
	mAllowKeyboardShortcuts->SetState(JBoolean(prefs->mAllowKeyboardShortcuts.GetValue()));
}

// Get params from DDX
void CPrefsDisplay::UpdatePrefs(CPreferences* prefs)
{
	// Get values
	mTabs->UpdateData(prefs);
	
	prefs->mAllowKeyboardShortcuts.SetValue(mAllowKeyboardShortcuts->IsChecked());
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplay message handlers

void CPrefsDisplay::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Display Preferences", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 130,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    mTabs =
        new CTabController(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,25, 380,330);
    assert( mTabs != NULL );

    mDisplayResetBtn =
        new JXTextButton("Reset Window Defaults", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,365, 155,25);
    assert( mDisplayResetBtn != NULL );

    mAllowKeyboardShortcuts =
        new JXTextCheckbox("Allow Single Key Shortcuts", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 195,368, 195,20);
    assert( mAllowKeyboardShortcuts != NULL );

// end JXLayout1

	// Create tab panels
	CTabPanel* card = new CPrefsDisplayServer(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 305);
	mTabs->AppendCard(card, "Server");
	card = new CPrefsDisplayMailbox(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 305);
	mTabs->AppendCard(card, "Mailbox");
	card = new CPrefsDisplayMessage(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 305);
	mTabs->AppendCard(card, "Message");
	card = new CPrefsDisplayLabel(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 305);
	mTabs->AppendCard(card, "Label");
	card = new CPrefsDisplayQuotes(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 305);
	mTabs->AppendCard(card, "Quotes");
	card = new CPrefsDisplayStyles(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 305);
	mTabs->AppendCard(card, "Styles");

	// Listen to items
	ListenTo(mDisplayResetBtn);
}

void CPrefsDisplay::Receive(JBroadcaster*	sender, const Message&	message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mDisplayResetBtn)
		{
			OnDisplayReset();
			return;
		}
	}
	
	CPrefsPanel::Receive(sender, message);
}

void CPrefsDisplay::OnDisplayReset()
{
	mCopyPrefs->ResetAllStates(true);
	CPreferencesDialog::sPrefsDlog->SetForceWindowReset(true);
}
