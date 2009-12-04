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


// Source for C3PaneMessage class

#include "C3PaneMessage.h"

#include "CMessage.h"
#include "CMessageView.h"

BEGIN_MESSAGE_MAP(C3PaneMessage, C3PanePanel)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneMessage::C3PaneMessage()
{
}

// Default destructor
C3PaneMessage::~C3PaneMessage()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int C3PaneMessage::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (C3PaneViewPanel::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Create server view
	CRect rect = CRect(0, 0, width, height);
	mMessageView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	AddAlignment(new CWndAlignment(&mMessageView, CWndAlignment::eAlign_WidthHeight));
	
	// Hide it until a mailbox is set
	ShowWindow(SW_HIDE);

	return 0;
}

CBaseView* C3PaneMessage::GetBaseView() const
{
	return const_cast<CMessageView*>(&mMessageView);
}

bool C3PaneMessage::TestClose()
{
	// Can always close
	return true;
}

bool C3PaneMessage::IsSpecified() const
{
	return mMessageView.GetMessage() != NULL;
}

void C3PaneMessage::SetMessage(CMessage* msg)
{
	// Give it to message view to display it
	mMessageView.SetMessage(msg);
}

cdstring C3PaneMessage::GetTitle() const
{
	CMessage* msg = mMessageView.GetMessage();
	return (msg && msg->GetEnvelope()) ? msg->GetEnvelope()->GetSubject() : cdstring::null_str;
}

UINT C3PaneMessage::GetIconID() const
{
	return IDI_3PANEPREVIEWMESSAGE;
}
