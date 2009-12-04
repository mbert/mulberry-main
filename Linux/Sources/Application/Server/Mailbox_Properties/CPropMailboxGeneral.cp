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


// Source for CPropMailboxGeneral class

#include "CPropMailboxGeneral.h"

#include "CIconLoader.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CStaticText.h"

#include <JXColormap.h>
#include <JXDownRect.h>
#include <JXImageWidget.h>
#include <JXStaticText.h>
#include <JXTextButton.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropMailboxGeneral::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("General", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 55,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    mIconState =
        new JXImageWidget(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 414,4, 32,32);
    assert( mIconState != NULL );
    mIconState->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj2 =
        new JXStaticText("Name:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,20, 45,20);
    assert( obj2 != NULL );

    mName =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,18, 315,20);
    assert( mName != NULL );
    mName->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj3 =
        new JXStaticText("Server:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,43, 60,20);
    assert( obj3 != NULL );

    mServer =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,41, 315,20);
    assert( mServer != NULL );
    mServer->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj4 =
        new JXStaticText("Hierarchy:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,66, 65,20);
    assert( obj4 != NULL );

    mHierarchy =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,64, 315,20);
    assert( mHierarchy != NULL );
    mHierarchy->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj5 =
        new JXStaticText("Hierarchy\nSeparator:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,89, 70,35);
    assert( obj5 != NULL );

    mSeparator =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,100, 20,20);
    assert( mSeparator != NULL );
    mSeparator->SetBorderWidth(kJXDefaultBorderWidth);

    JXDownRect* obj6 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,135, 440,75);
    assert( obj6 != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Messages:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,125, 70,20);
    assert( obj7 != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Total:", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 13,17, 40,20);
    assert( obj8 != NULL );

    mTotal =
        new CStaticText("", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 48,15, 55,20);
    assert( mTotal != NULL );
    mTotal->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj9 =
        new JXStaticText("Recent:", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 119,17, 50,20);
    assert( obj9 != NULL );

    mRecent =
        new CStaticText("", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 166,15, 51,20);
    assert( mRecent != NULL );
    mRecent->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj10 =
        new JXStaticText("Unseen:", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 222,17, 52,20);
    assert( obj10 != NULL );

    mUnseen =
        new CStaticText("", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 272,15, 51,20);
    assert( mUnseen != NULL );
    mUnseen->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj11 =
        new JXStaticText("Deleted:", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 327,17, 53,20);
    assert( obj11 != NULL );

    mDeleted =
        new CStaticText("", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 378,15, 51,20);
    assert( mDeleted != NULL );
    mDeleted->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj12 =
        new JXStaticText("Synchronised State:", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 8,47, 125,20);
    assert( obj12 != NULL );

    JXStaticText* obj13 =
        new JXStaticText("Full:", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 137,47, 33,20);
    assert( obj13 != NULL );

    mFullSync =
        new CStaticText("", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 166,45, 51,20);
    assert( mFullSync != NULL );
    mFullSync->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj14 =
        new JXStaticText("Partial:", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 228,47, 47,20);
    assert( obj14 != NULL );

    mPartialSync =
        new CStaticText("", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 272,45, 51,20);
    assert( mPartialSync != NULL );
    mPartialSync->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj15 =
        new JXStaticText("Missing:", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 327,47, 53,20);
    assert( obj15 != NULL );

    mMissingSync =
        new CStaticText("", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 378,45, 51,20);
    assert( mMissingSync != NULL );
    mMissingSync->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj16 =
        new JXStaticText("Size:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,218, 40,18);
    assert( obj16 != NULL );

    mSize =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,216, 75,19);
    assert( mSize != NULL );
    mSize->SetBorderWidth(kJXDefaultBorderWidth);

    mCalculateBtn =
        new JXTextButton("Calculate", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 175,216, 70,19);
    assert( mCalculateBtn != NULL );
    mCalculateBtn->SetFontSize(10);

    JXStaticText* obj17 =
        new JXStaticText("Status:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,240, 55,22);
    assert( obj17 != NULL );

    mStatus =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,238, 315,22);
    assert( mStatus != NULL );
    mStatus->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj18 =
        new JXStaticText("UID Validity:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,265, 80,18);
    assert( obj18 != NULL );

    mUIDValidity =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,263, 315,20);
    assert( mUIDValidity != NULL );
    mUIDValidity->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj19 =
        new JXStaticText("Synchronised:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,288, 88,22);
    assert( obj19 != NULL );

    mLastSync =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,288, 315,20);
    assert( mLastSync != NULL );
    mLastSync->SetBorderWidth(kJXDefaultBorderWidth);

// end JXLayout1

	ListenTo(mCalculateBtn);
}

// Handle buttons
void CPropMailboxGeneral::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXTextButton::kPushed) && (sender == mCalculateBtn))
	{
		OnCalculateSize();
	}

	CMailboxPropPanel::Receive(sender, message);
}

// Set mbox list
void CPropMailboxGeneral::SetMboxList(CMboxList* mbox_list)
{
	// Save list
	mMboxList = mbox_list;

	// For now just use first item
	SetMbox((CMbox*) mMboxList->front());
}


// Set mbox list
void CPropMailboxGeneral::SetMbox(CMbox* mbox)
{
	// Do icon state
	if (mbox->GetProtocol()->CanDisconnect())
		mIconState->SetImage(CIconLoader::GetIcon(mbox->GetProtocol()->IsDisconnected() ?
												IDI_MAILBOX_DISCONNECTED : IDI_MAILBOX_REMOTE, mIconState, 32, 0x00CCCCCC), kFalse);
	else if (mbox->IsLocalMbox())
	{
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_LOCAL, mIconState, 32, 0x00CCCCCC),  kFalse);
		mLastSync->Deactivate();
	}
	else
	{
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_REMOTE, mIconState, 32, 0x00CCCCCC), kFalse);
		mLastSync->Deactivate();
	}

	cdstring copyStr;
	
	// Copy text to edit fields
	mName->SetText(mbox->GetName());

	mServer->SetText(mbox->GetProtocol()->GetDescriptor());

	mHierarchy->SetText(mbox->GetWD());

	if (mbox->GetDirDelim())
		copyStr = mbox->GetDirDelim();
	else
		copyStr = cdstring::null_str;
	mSeparator->SetText(copyStr);

	// Only set these if no error
	if (!mbox->Error() && !mbox->IsDirectory())
	{
		unsigned long total = mbox->GetNumberFound();
		copyStr = (long) total;
		mTotal->SetText(copyStr);

		copyStr = (long) mbox->GetNumberRecent();
		mRecent->SetText(copyStr);

		copyStr = (long) mbox->GetNumberUnseen();
		mUnseen->SetText(copyStr);

		// These are only available when open
		if (mbox->IsOpen())
		{
			mDeleted->Activate();
			copyStr = (long) mbox->GetNumberDeleted();
			mDeleted->SetText(copyStr);
		}
		else
		{
			mDeleted->Deactivate();
			mDeleted->SetText(cdstring::null_str);
		}

		// These are only available when open and sync'ing is available
		if (mbox->IsOpen() && mbox->GetProtocol()->CanDisconnect() && (mbox->GetLastSync() != 0))
		{
			unsigned long fullsync = mbox->CountFlags(NMessage::eFullLocal);
			copyStr = (long) fullsync;
			mFullSync->SetText(copyStr);

			unsigned long partialsync  = mbox->CountFlags(NMessage::ePartialLocal);
			copyStr = (long) partialsync;
			mPartialSync->SetText(copyStr);
			
			// If disconnected we cannot know how many are missing
			if (mbox->GetProtocol()->IsDisconnected())
				mMissingSync->SetText(cdstring::null_str);
			else
			{
				copyStr = (long) (total - fullsync - partialsync);
				mMissingSync->SetText(copyStr);
			}
		}
		else
		{
			mFullSync->SetText(cdstring::null_str);
			mPartialSync->SetText(cdstring::null_str);
			mMissingSync->SetText(cdstring::null_str);
		}

		{
			unsigned long size = mbox->GetSize();
			if ((size == ULONG_MAX) && mbox->IsLocalMbox() && mbox->IsCachedMbox())
			{
				mbox->CheckSize();
				size = mbox->GetSize();
			}
			if (size != ULONG_MAX)
			{
				mSize->SetText(::GetNumericFormat(size));
			}
			else
				mSize->SetText(cdstring::null_str);
		}

		copyStr = (long) mbox->GetUIDValidity();
		mUIDValidity->SetText(copyStr);

		{
			unsigned long utc_time = mbox->GetLastSync();
			
			if (utc_time)
			{
				// Determine timezone offset
				time_t t1 = ::time(NULL);
				time_t t2 = ::mktime(::gmtime(&t1));
				long offset = (long) ::difftime(t1, t2);
				long current_zone = (long)(::difftime(t1, t2) / 3600.0);	// Zone in hundreds e.g. -0500
				current_zone *= 100;
				utc_time += offset;

				mLastSync->SetText(CRFC822::GetTextDate(utc_time, current_zone, true));
			}
			else
				mLastSync->SetText(cdstring::null_str);
		}
	}

	// Disable size calculate if a directory
	if (mbox->IsDirectory())
		mCalculateBtn->Deactivate();

	copyStr = cdstring::null_str;
	cdstring s;
	if (mbox->IsOpenSomewhere())
	{
		s.FromResource("UI::MailboxProp::StateOpen");
		copyStr += s;
	}
	if (mbox->IsSubscribed())
	{
		if (!copyStr.empty())
			copyStr += ", ";
		s.FromResource("UI::MailboxProp::StateSubs");
		copyStr += s;
	}
	if (mbox->NoInferiors())
	{
		if (!copyStr.empty())
			copyStr += ", ";
		s.FromResource("UI::MailboxProp::StateNoInfs");
		copyStr += s;
	}
	if (mbox->NoSelect())
	{
		if (!copyStr.empty())
			copyStr += ", ";
		s.FromResource("UI::MailboxProp::StateNoSel");
		copyStr += s;
	}
	if (mbox->Marked())
	{
		if (!copyStr.empty())
			copyStr += ", ";
		s.FromResource("UI::MailboxProp::StateMark");
		copyStr += s;
	}
	mStatus->SetText(copyStr);
}

// Set protocol
void CPropMailboxGeneral::SetProtocol(CMboxProtocol* protocol)
{
	// Does nothing
}

// Force update of items
void CPropMailboxGeneral::ApplyChanges(void)
{
	// Nothing to update
}

// Check sizes of all mailboxes
void CPropMailboxGeneral::OnCalculateSize()
{
	// Iterate over all mailboxes
	for(CMboxList::iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);
		if (!mbox->IsDirectory())
			mbox->CheckSize();
	}

	{
		unsigned long size = static_cast<CMbox*>(mMboxList->front())->GetSize();
		if (size != ULONG_MAX)
		{
			mSize->SetText(::GetNumericFormat(size));
		}
		else
			mSize->SetText(cdstring::null_str);
	}
}
