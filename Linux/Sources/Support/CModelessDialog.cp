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

#include "cdstring.h"

#include <JString.h>
#include <JXButton.h>
#include <JXWindow.h>

CModelessDialog::CModelessDialog(JXDirector* supervisor)
	: JXDialogDirector(supervisor, kFalse)
{
}

void CModelessDialog::OnCreate()
{
	// Bump title counter
	TitleCounter()++;
	
	// Alter title if needed
	if (TitleCounter() > 1)
	{
		cdstring title = GetWindow()->GetTitle().GetCString();
		title += " ";
		title += cdstring(TitleCounter());
		GetWindow()->SetTitle(title);
	}
}

void CModelessDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == GetOKButton() && message.Is(JXButton::kPushed))
	{
		OnOK();
	}
	else if (sender == GetCancelButton() && message.Is(JXButton::kPushed))
	{
		OnCancel();
	}
	else
	{
		JXDialogDirector::Receive(sender, message);
	}
}

void CModelessDialog::OnOK()
{
	EndDialog(kTrue);
}

void CModelessDialog::OnCancel()
{
	EndDialog(kFalse);
}
