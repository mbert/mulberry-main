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

#include "CConnectionManager.h"
//#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CRFC822.h"
#include "CTextFieldX.h"

#include <LIconControl.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S N E T W O R K
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPropMailboxGeneral::CPropMailboxGeneral()
{
}

// Constructor from stream
CPropMailboxGeneral::CPropMailboxGeneral(LStream *inStream)
		: CMailboxPropPanel(inStream)
{
}

// Default destructor
CPropMailboxGeneral::~CPropMailboxGeneral()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropMailboxGeneral::FinishCreateSelf(void)
{
	// Do inherited
	CMailboxPropPanel::FinishCreateSelf();

	// Icon state
	mIconState = (LIconControl*) FindPaneByID(paneid_MailboxGeneralState);

	// Get items
	mName = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralName);
	mServer = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralServer);
	mHierarchy = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralHierarchy);
	mSeparator = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralSeparator);
	mTotal = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralTotal);
	mRecent = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralRecent);
	mUnseen = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralUnseen);
	mDeleted = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralDeleted);
	mFullSync = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralFullSync);
	mPartialSync = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralPartialSync);
	mMissingSync = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralMissingSync);
	mSize = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralSize);
	mStatus = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralStatus);
	mUIDValidity = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralUIDValidity);
	mLastSync = (CTextFieldX*) FindPaneByID(paneid_MailboxGeneralLastSync);

	// Cannot calculate size when disconnected
	if (!CConnectionManager::sConnectionManager.IsConnected())
		FindPaneByID(paneid_MailboxGeneralCalculate)->Disable();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPropMailboxGeneralBtns);
}

// Handle buttons
void CPropMailboxGeneral::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_MailboxGeneralCalculate:
		CheckSize();
		break;
	}
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
		mIconState->SetResourceID(mbox->GetProtocol()->IsDisconnected() ? icnx_MailboxStateDisconnected : icnx_MailboxStateRemote);
	else if (mbox->IsLocalMbox())
	{
		mIconState->SetResourceID(icnx_MailboxStateLocal);
		mLastSync->Disable();
	}
	else
	{
		mIconState->SetResourceID(icnx_MailboxStateRemote);
		mLastSync->Disable();
	}

	// Copy text to edit fields
	mName->SetText(mbox->GetName());

	mServer->SetText(mbox->GetProtocol()->GetDescriptor());

	mHierarchy->SetText(mbox->GetWD());

	cdstring temp(mbox->GetDirDelim());
	mSeparator->SetText(temp);

	// Only set these if no error and not a directory
	if (!mbox->Error() && !mbox->IsDirectory())
	{
		temp = cdstring(mbox->GetUIDValidity());	// unsigned long -> c-string
		mUIDValidity->SetText(temp);

		{
			unsigned long utc_time = mbox->GetLastSync();
			
			if (utc_time)
			{
				// Determine timezone offset
				time_t t1 = ::time(NULL);
				time_t t2 = ::mktime(::gmtime(&t1));
				long offset = ::difftime(t1, t2);
				long current_zone = ::difftime(t1, t2) / 3600.0;	// Zone in hundreds e.g. -0500
				current_zone *= 100;
				utc_time += offset;

				temp = CRFC822::GetTextDate(utc_time, current_zone, true);
			}
			else
				temp = cdstring::null_str;
			mLastSync->SetText(temp);
		}

		mTotal->SetNumber(mbox->GetNumberFound());

		mRecent->SetNumber(mbox->GetNumberRecent());

		mUnseen->SetNumber(mbox->GetNumberUnseen());

		// These are only available when open
		if (mbox->IsOpen())
		{
			mDeleted->SetNumber(mbox->GetNumberDeleted());
		}
		else
			mDeleted->SetText(cdstring::null_str);

		// These are only available when open and sync'ing is available
		if (mbox->IsOpen() && mbox->GetProtocol()->CanDisconnect() && (mbox->GetLastSync() != 0))
		{
			unsigned long fullsync = mbox->CountFlags(NMessage::eFullLocal);
			mFullSync->SetNumber(fullsync);

			unsigned long partialsync  = mbox->CountFlags(NMessage::ePartialLocal);
			mPartialSync->SetNumber(partialsync);
			
			// If disconnected we cannot know how many are missing
			if (mbox->GetProtocol()->IsDisconnected())
				mMissingSync->SetText(cdstring::null_str);
			else
			{
				mMissingSync->SetNumber(mbox->GetNumberFound() - fullsync - partialsync);
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
	}

	// Disable size calculate if a directory
	if (mbox->IsDirectory())
		FindPaneByID(paneid_MailboxGeneralCalculate)->Disable();

	temp = cdstring::null_str;
	if (mbox->IsOpenSomewhere())
	{
		cdstring rsrc;
		rsrc.FromResource("UI::MailboxProp::StateOpen");
		temp += rsrc;
	}
	if (mbox->IsSubscribed())
	{
		cdstring rsrc;
		rsrc.FromResource("UI::MailboxProp::StateSubs");
		if (!temp.empty())
			temp += ", ";
		temp += rsrc;
	}
	if (mbox->NoInferiors())
	{
		cdstring rsrc;
		rsrc.FromResource("UI::MailboxProp::StateNoInfs");
		if (!temp.empty())
			temp += ", ";
		temp += rsrc;
	}
	if (mbox->NoSelect())
	{
		cdstring rsrc;
		rsrc.FromResource("UI::MailboxProp::StateNoSel");
		if (!temp.empty())
			temp += ", ";
		temp += rsrc;
	}
	if (mbox->Marked())
	{
		cdstring rsrc;
		rsrc.FromResource("UI::MailboxProp::StateMark");
		if (!temp.empty())
			temp += ", ";
		temp += rsrc;
	}
	mStatus->SetText(temp);
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
void CPropMailboxGeneral::CheckSize()
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
