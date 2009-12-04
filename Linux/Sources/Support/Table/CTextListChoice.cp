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


// Source for CNewSearchDialog class

#include "CTextListChoice.h"

#include "CTableScrollbarSet.h"
#include "CTextField.h"
#include "CTextTable.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>

#include <jXGlobals.h>

#include <cassert>

CTextListChoice::CTextListChoice(JXDirector* supervisor)
	: CDialogDirector(supervisor), mButtonTitle("OK")
{
	mUseEntry = false;
	mSingleSelection = false;
	mNoSelection = false;
	mSelectFirst = false;
}

void CTextListChoice::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 340,220, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 340,220);
    assert( obj1 != NULL );

    mListTitle =
        new JXStaticText("Choose Item:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 320,20);
    assert( mListTitle != NULL );

    mEnterTitle =
        new JXStaticText("Input Title", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,130, 320,20);
    assert( mEnterTitle != NULL );

    mEnterCtrl =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,150, 320,20);
    assert( mEnterCtrl != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 250,185, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 160,185, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    CScrollbarSet* sbs =
        new CScrollbarSet(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 320,90);
    assert( sbs != NULL );

// end JXLayout

	mList = new CTextTable(sbs,sbs->GetScrollEnclosure(),
															JXWidget::kHElastic,
															JXWidget::kVElastic,
															0,0, 10, 10);
	mList->OnCreate();
	mList->SetSelectionMsg(true);
	mList->SetDoubleClickMsg(true);
	ListenTo(mList);

	SetButtons(mOKBtn, mCancelBtn);

	// Init data

	// Titles
	GetWindow()->SetTitle(mUseEntry ? mTextListEnterDescription : mTextListDescription);
	mOKBtn->SetLabel(mButtonTitle);
	mListTitle->SetText(mTextListDescription);
	if (mUseEntry)
	{
		mEnterTitle->SetText(mTextListEnterDescription);
		mEnterCtrl->SetText(mTextListEnter);
	}

	// Hide entry bits
	if (!mUseEntry)
	{
		mEnterCtrl->Hide();
		mEnterTitle->Hide();

		// Increase list item height
		sbs->AdjustSize(0, 50);
	}

	// Determine list selection type
	if (mNoSelection)
		mList->SetNoSelection();
	else if (mSingleSelection)
		mList->SetSingleSelection();

	// Add items to list
	for(cdstrvect::const_iterator iter = mItems.begin(); iter != mItems.end(); iter++)
		mList->AddItem(*iter);

	// Select first item
	if (mSelectFirst && !mNoSelection)
		mList->SelectRow(1);

	// Set the focus
	if (mUseEntry)
		mEnterCtrl->Focus();
	else
		mList->Focus();
}

void CTextListChoice::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == mList)
	{
		if (message.Is(CTextTable::kSelectionChanged))
		{
			// OK button only available when selection and not entry
			if (!mUseEntry)
			{
				if (mList->IsSelectionValid())
					mOKBtn->Activate();
				else
					mOKBtn->Deactivate();
			}
			return;
		}
		else if (message.Is(CTextTable::kLDblClickCell))
		{
			// Only if selections allowed
			if (!mNoSelection)
				mOKBtn->Push();
			return;
		}
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}

bool CTextListChoice::PoseDialog(const char* title, const char* description, const char* item, bool use_entry,
							bool single_selection, bool no_selection, bool select_first,
							cdstrvect& items, cdstring& text, ulvector& selection, const char* btn)
{
	bool result = false;

	CTextListChoice* dlog = new CTextListChoice(JXGetApplication());
	dlog->mUseEntry = use_entry;
	dlog->mSingleSelection = single_selection;
	dlog->mNoSelection = no_selection;
	dlog->mSelectFirst = select_first;
	if (description)
		dlog->mTextListDescription.FromResource(description);
	if (item)
		dlog->mTextListEnterDescription.FromResource(item);
	if (btn)
		dlog->mButtonTitle.FromResource(btn);
	dlog->mTextListEnter = text;

	// Add the rest
	for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++)
		dlog->mItems.push_back(*iter);

	if (dlog->DoModal() == kDialogClosed_OK)
	{
		if (use_entry)
			text = dlog->mEnterCtrl->GetText();
		else
			dlog->mList->GetSelection(selection);
		result = true;
		dlog->Close();
	}

	return result;
}
