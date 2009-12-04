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


// Source for CTextListChoice class

#include "CTextListChoice.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CStaticText.h"
#include "CTextFieldX.h"
#include "CTextTable.h"
#include "CXStringResources.h"

#include <LPushButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C T E X T L I S T D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTextListChoice::CTextListChoice()
{
}

// Constructor from stream
CTextListChoice::CTextListChoice(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CTextListChoice::~CTextListChoice()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CTextListChoice::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mList = (CTextTable*) FindPaneByID(paneid_TextListList);
	mList->AddListener(this);
	mList->AddListener((CBalloonDialog*) mSuperCommander);
	mName = (CTextFieldX*) FindPaneByID(paneid_TextListItem);
}

// Get details from dialog
cdstring CTextListChoice::GetItem(void)
{
	return mName->GetText();
}

// Get details from dialog
ulvector CTextListChoice::GetSelection(void)
{
	ulvector selection;
	mList->GetSelection(selection);

	return selection;
}

// Called during idle
void CTextListChoice::SetUpDetails(const char* title, const char* description, const char* item,
									bool use_entry, bool single_selection, bool no_selection, bool select_first,
									cdstrvect& items, cdstring& text, const char* btn)
{
	LStr255 txt;

	// Give text to pane and window title
	txt = rsrc::GetString(title);
	SetDescriptor(txt);

	CStaticText* theCaption = (CStaticText*) FindPaneByID(paneid_TextListDescription);
	txt = rsrc::GetString(description);
	theCaption->SetDescriptor(txt);

	if (use_entry)
	{
		theCaption = (CStaticText*) FindPaneByID(paneid_TextListItemTitle);
		txt = rsrc::GetString(item);
		theCaption->SetDescriptor(txt);
		mName->SetText(text);
		mName->SelectAll();
	}
	else
	{
		// Hide unwanted items
		LView* entry_area = (LView*) FindPaneByID(paneid_TextListItems);
		entry_area->Hide();
		ResizeWindowBy(0, -48);
	}

	// Set table details
	if (no_selection)
		mList->SetNoSelection();
	else if (single_selection)
		mList->SetSingleSelection();
	mList->SetContents(items);
	if (!use_entry)
	{
		SetLatentSub(mList);
		if (!no_selection && select_first)
		{
			mList->SelectCell(STableCell(1, 1));
			
			// Make sure double-click does OK
			mList->SetDoubleClickMsg(msg_OK);
		}
	}

	// Set button
	if (btn)
	{
		txt = rsrc::GetString(btn);
		LPushButton* theButton = (LPushButton*) FindPaneByID(paneid_TextListButton);
		theButton->SetDescriptor(txt);
	}
}

bool CTextListChoice::PoseDialog(const char* title, const char* description, const char* item,
									bool use_entry, bool single_selection, bool no_selection, bool select_first,
									cdstrvect& items, cdstring& text, ulvector& selection, const char* btn)
{
	bool result = false;

	// Create the dialog
	{
		CBalloonDialog	theHandler(paneid_TextListDialog, CMulberryApp::sApp);
		CTextListChoice* dlog = (CTextListChoice*) theHandler.GetDialog();

		// Set dlog info
		dlog->SetUpDetails(title, description, item, use_entry, single_selection, no_selection, select_first, items, text, btn);

		theHandler.StartDialog();

		// Let DialogHandler process events
		MessageT hitMessage;
		while (true)
		{
			hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				if (use_entry)
					text = dlog->GetItem();
				if (!no_selection)
					selection = dlog->GetSelection();
				result = true;
				break;
			}
			else if (!no_selection && (hitMessage == msg_TextListDblClick))
			{
				if (use_entry)
				{
					cdstrvect texts;
					dlog->mList->GetSelection(texts);
					if (texts.size() != 0)
						text = texts.front();
				}
				if (!no_selection)
					selection = dlog->GetSelection();
				result = true;
				break;
			}
			else if (hitMessage == msg_Cancel)
				break;
		}
	}

	return result;
}
