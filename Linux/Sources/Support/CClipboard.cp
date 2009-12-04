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


// Source for CHelpAttach class

#include "CClipboard.h"

#include "cdstring.h"

#include <JXDisplay.h>
#include <JXSelectionManager.h>
#include <JXTextSelection.h>
#include <jXGlobals.h>

#include <cassert>

#pragma mark ____________________________CContextMenu


// O T H E R  M E T H O D S ____________________________________________________________________________

void CClipboard::CopyToPrimaryClipboard(JXDisplay* display, const char* txt)
{
	CopyToClipboard(display, txt, XA_PRIMARY);
}

void CClipboard::CopyToSecondaryClipboard(JXDisplay* display, const char* txt)
{
	CopyToClipboard(display, txt, XInternAtom(*display, "CLIPBOARD", False));
}

void CClipboard::GetPrimaryClipboard(JXDisplay* display, cdstring& txt)
{
	GetClipboard(display, txt, XA_PRIMARY);
}

void CClipboard::GetSecondaryClipboard(JXDisplay* display, cdstring& txt)
{
	GetClipboard(display, txt, XInternAtom(*display, "CLIPBOARD", False));
}

bool CClipboard::HasPrimaryText(JXDisplay* display)
{
	// Check for text types
	Time time = CurrentTime;
	Atom textType;
	return TestText(display, XA_PRIMARY, time, textType);
}

bool CClipboard::HasSecondaryText(JXDisplay* display)
{
	// Check for text types
	Time time = CurrentTime;
	Atom textType;
	return TestText(display, XInternAtom(*display, "CLIPBOARD", False), time, textType);
}

void CClipboard::CopyToClipboard(JXDisplay* display, const char* txt, Atom clip)
{
	JXTextSelection* data = new JXTextSelection(display, txt, NULL, NULL);
	assert( data != NULL );

	if (!display->GetSelectionManager()->SetData(clip, data))
	{
		(JGetUserNotification())->ReportError("Unable to copy to the X Clipboard.");
	}
}

void CClipboard::GetClipboard(JXDisplay* display, cdstring& txt, Atom clip)
{
	// Get available types on clipboard
	JXSelectionManager* selMgr = display->GetSelectionManager();

	// Check for text types
	Time time = CurrentTime;
	Atom textType;
	if (!TestText(display, clip, time, textType))
		return;

	// Now get actual data
	JBoolean gotData = kFalse;

	Atom returnType, textReturnType;
	unsigned char* data = NULL;
	JSize dataLength;
	JXSelectionManager::DeleteMethod delMethod;

	if (selMgr->GetData(clip, time, textType,
						&textReturnType, &data, &dataLength, &delMethod))
	{
		if (textReturnType == XA_STRING ||
			textReturnType == selMgr->GetUTF8StringXAtom() ||
			textReturnType == selMgr->GetMimePlainTextXAtom())
		{
			gotData = kTrue;
			txt.assign(reinterpret_cast<char*>(data), dataLength);
		}
		selMgr->DeleteData(&data, delMethod);
	}
}

bool CClipboard::TestText(JXDisplay* display, Atom clip, Time time, Atom& textType)
{
	// Get available types on clipboard
	JXSelectionManager* selMgr = display->GetSelectionManager();
	JArray<Atom> typeList;
	if (!selMgr->GetAvailableTypes(clip, time, &typeList))
		return false;

	// Check for text types
	const JSize typeCount = typeList.GetElementCount();
	bool got_type = false;
	for (JIndex i=1; i<=typeCount; i++)
	{
		const Atom type = typeList.GetElement(i);
		if (type == XA_STRING ||
			type == selMgr->GetMimePlainTextXAtom() ||
			(!got_type && type == selMgr->GetTextXAtom()))
		{
			textType = type;
			got_type = true;
		}
	}

	return got_type;
}
