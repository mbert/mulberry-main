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


// Source for CModelessDialog class

#include "CModelessDialog.h"

#include "CWindowsMenu.h"

#include "MyCFString.h"

//	Constructor

CModelessDialog::CModelessDialog(LStream *inStream) :
	LDialogBox(inStream)
{
	// Add to list
	CWindowsMenu::AddWindow(this);
}


//	Destructor

CModelessDialog::~CModelessDialog()
{
	// Remove from list
	CWindowsMenu::RemoveWindow(this);
}

#pragma mark -

void CModelessDialog::FinishCreateSelf()
{
	LDialogBox::FinishCreateSelf();

	// Bump title counter
	TitleCounter()++;
	
	// Alter title if needed
	if (TitleCounter() > 1)
	{
		MyCFString cf = CopyCFDescriptor();
		cdstring title = cf.GetString();
		title += " ";
		title += cdstring((int)TitleCounter());
		MyCFString cftitle(title, kCFStringEncodingUTF8);
		SetCFDescriptor(cftitle);
		CWindowsMenu::RenamedWindow();
	}
}

// Handle controls
void CModelessDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_OK:
		OnOK();
		break;
	
	case msg_Cancel:
		OnCancel();
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

void CModelessDialog::AttemptClose()
{
	// Do cancel operation
	OnCancel();
}

void CModelessDialog::DoClose()
{
	// Bump down counter
	if (TitleCounter() > 0)
		TitleCounter()--;
	
	// Do inherited
	LDialogBox::DoClose();
}

void CModelessDialog::OnOK()
{
	// Now close the dialog
	DoClose();
}

void CModelessDialog::OnCancel()
{
	// Now close the dialog
	DoClose();
}

