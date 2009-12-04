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


// Source for C3PaneMailbox class

#include "C3PaneMailbox.h"

#include "CMbox.h"
#include "CPreferences.h"

BEGIN_MESSAGE_MAP(C3PaneMailbox, C3PanePanel)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneMailbox::C3PaneMailbox()
{
	mDoneInit = false;
	mUseSubstitute = false;
}

// Default destructor
C3PaneMailbox::~C3PaneMailbox()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int C3PaneMailbox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (C3PaneViewPanel::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Create mailbox view
	CRect rect = CRect(0, 0, width, height);
	mMailboxView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	AddAlignment(new CWndAlignment(&mMailboxView, CWndAlignment::eAlign_WidthHeight));
	
	// Hide it until a mailbox is set
	ShowWindow(SW_HIDE);

	return 0;
}

CBaseView* C3PaneMailbox::GetBaseView() const
{
	return const_cast<CMailboxInfoView*>(&mMailboxView);
}

void C3PaneMailbox::Init()
{
	// Check for use of tabs
	if (!mDoneInit &&
		CPreferences::sPrefs->m3PaneOptions.GetValue().GetMailViewOptions().GetUseTabs())
	{
		// Turn on use of tabs
		mUseSubstitute = true;
		mMailboxView.SetUseSubstitute(true);
		
		// Restore last set of tabs if requested
		if (CPreferences::sPrefs->m3PaneOptions.GetValue().GetMailViewOptions().GetRestoreTabs())
			mMailboxView.SetSubstituteItems(CPreferences::sPrefs->m3PaneOpenMailboxes.GetValue());
	}
	mDoneInit = true;
}

void C3PaneMailbox::GetOpenItems(cdstrvect& items) const
{
	// Check to see whether tabs in use
	if (mMailboxView.GetUseSubstitute())
		mMailboxView.GetSubstituteItems(items);
	else
		mMailboxView.GetStaticItems(items);
}

void C3PaneMailbox::CloseOpenItems()
{
	mMailboxView.DoCloseAll();
}

void C3PaneMailbox::SetOpenItems(const cdstrvect& items)
{
	// Check to see whether tabs in use
	if (mMailboxView.GetUseSubstitute())
		mMailboxView.SetSubstituteItems(items);
	else
		mMailboxView.SetStaticItems(items);
}

void C3PaneMailbox::DoClose()
{
	// Get the list of mailboxes currently open
	cdstrvect opened;
	mMailboxView.GetSubstituteItems(opened);
	CPreferences::sPrefs->m3PaneOpenMailboxes.SetValue(opened);

	// Close the mailbox view
	mMailboxView.DoClose();
}

bool C3PaneMailbox::TestCloseAll()
{
	// Get the mailbox view to do the test but ignore locked items
	return mMailboxView.TestCloseAll(true);
}

void C3PaneMailbox::DoCloseAll()
{
	// Close the mailbox view but ignore locked items
	mMailboxView.DoCloseAll(true);
}

bool C3PaneMailbox::TestCloseOne(unsigned long index)
{
	// Get the mailbox view to do the test
	return mMailboxView.TestCloseOne(index);
}

void C3PaneMailbox::DoCloseOne(unsigned long index)
{
	// Close the mailbox view
	mMailboxView.DoCloseOne(index);
}

bool C3PaneMailbox::TestCloseOthers(unsigned long index)
{
	// Get the mailbox view to do the test
	return mMailboxView.TestCloseOthers(index);
}

void C3PaneMailbox::DoCloseOthers(unsigned long index)
{
	// Close the mailbox view
	mMailboxView.DoCloseOthers(index);
}

void C3PaneMailbox::SetUseSubstitute(bool subs)
{
	mUseSubstitute = subs;
	mMailboxView.SetUseSubstitute(subs);
}

bool C3PaneMailbox::IsSubstituteLocked(unsigned long index) const
{
	// Test the view
	return mMailboxView.IsSubstituteLocked(index);
}

bool C3PaneMailbox::IsSubstituteDynamic(unsigned long index) const
{
	// Test the view
	return mMailboxView.IsSubstituteDynamic(index);
}

bool C3PaneMailbox::IsSpecified() const
{
	return mMailboxView.GetMbox() != NULL;
}

cdstring C3PaneMailbox::GetTitle() const
{
	// Get mailbox name
	CMbox* mbox = mMailboxView.GetMbox();
	cdstring result = mbox ? mbox->GetAccountName() : cdstring::null_str;
	
	// Add read-only string if required
	if (mbox && mbox->IsReadOnly())
	{
		cdstring readonly;
		readonly.FromResource("UI::Mailbox::ReadOnlyMailbox");
		result += readonly;
	}
	return result;
}

UINT C3PaneMailbox::GetIconID() const
{
	return IDI_3PANEPREVIEWMAILBOX;
}
