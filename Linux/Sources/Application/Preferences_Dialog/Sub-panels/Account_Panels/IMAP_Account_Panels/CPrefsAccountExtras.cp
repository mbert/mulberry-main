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


// Source for CPrefsAccountExtras class

#include "CPrefsAccountExtras.h"

#include "CCalendarAccount.h"
#include "CIdentityPopup.h"
#include "CMailAccount.h"
#include "CPreferencesDialog.h"

#include <JXDownRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccountExtras::OnCreate()
{
// begin JXLayout1

    JXDownRect* obj1 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 340,77);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Account Identity:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,5, 105,20);
    assert( obj2 != NULL );

    mTieIdentity =
        new JXTextCheckbox("Tie to Identity", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,10, 105,20);
    assert( mTieIdentity != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Identity:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,38, 50,20);
    assert( obj3 != NULL );

    mIdentityPopup =
        new CIdentityPopup("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,35, 200,20);
    assert( mIdentityPopup != NULL );

// end JXLayout1
	
	// Start listening
	ListenTo(mTieIdentity);
	ListenTo(mIdentityPopup);
}

void CPrefsAccountExtras::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mTieIdentity)
		{
			if (mTieIdentity->IsChecked())
				mIdentityPopup->Activate();
			else
				mIdentityPopup->Deactivate();
			return;
		}
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
    	if (sender == mIdentityPopup)
    	{
			JIndex item = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
			switch(item)
			{
			// New identity wanted
			case eIdentityPopup_New:
				mIdentityPopup->DoNewIdentity(CPreferencesDialog::sPrefsDlog->GetCopyPrefs());
				break;
			
			// New identity wanted
			case eIdentityPopup_Edit:
				mIdentityPopup->DoEditIdentity(CPreferencesDialog::sPrefsDlog->GetCopyPrefs());
				break;
			
			// Delete existing identity
			case eIdentityPopup_Delete:
				mIdentityPopup->DoDeleteIdentity(CPreferencesDialog::sPrefsDlog->GetCopyPrefs());
				break;
			
			// Select an identity
			default:
				// We do nothing here
				break;
			}
		} 
	}

	CTabPanel::Receive(sender, message);
}

// Set prefs
void CPrefsAccountExtras::SetData(void* data)
{
	CMailAccount* maccount = dynamic_cast<CMailAccount*>(static_cast<CINETAccount*>(data));
	CCalendarAccount* caccount = dynamic_cast<CCalendarAccount*>(static_cast<CINETAccount*>(data));

	bool tie = false;
	cdstring id;
	if (maccount != NULL)
	{
		tie = maccount->GetTieIdentity();
		id = maccount->GetTiedIdentity();
	}
	else if (caccount != NULL)
	{
		tie = caccount->GetTieIdentity();
		id = caccount->GetTiedIdentity();
	}
	
	// Get new prefs
	CPreferences* new_prefs = CPreferencesDialog::sPrefsDlog->GetCopyPrefs();
	mIdentityPopup->Reset(new_prefs->mIdentities.GetValue());

	// Set first identity
	mTieIdentity->SetState(JBoolean(tie));

	// Set it in popup
	mIdentityPopup->SetIdentity(new_prefs, id);

	// Disable if not in use
	mIdentityPopup->SetActive(tie ? kTrue : kFalse);
}

// Force update of prefs
bool CPrefsAccountExtras::UpdateData(void* data)
{
	CMailAccount* maccount = dynamic_cast<CMailAccount*>(static_cast<CINETAccount*>(data));
	CCalendarAccount* caccount = dynamic_cast<CCalendarAccount*>(static_cast<CINETAccount*>(data));

	// Get new prefs
	CPreferences* new_prefs = CPreferencesDialog::sPrefsDlog->GetCopyPrefs();

	if (maccount != NULL)
	{
		maccount->SetTieIdentity(mTieIdentity->IsChecked());
		maccount->SetTiedIdentity(mIdentityPopup->GetIdentity(new_prefs).GetIdentity());
	}
	else if (caccount != NULL)
	{
		caccount->SetTieIdentity(mTieIdentity->IsChecked());
		caccount->SetTiedIdentity(mIdentityPopup->GetIdentity(new_prefs).GetIdentity());
	}
	return true;
}
