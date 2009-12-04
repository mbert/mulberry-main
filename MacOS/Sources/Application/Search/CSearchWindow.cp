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


// Source for CSearchWindow class

#include "CSearchWindow.h"

#include "CBrowseMailboxDialog.h"
#include "CGetStringDialog.h"
#include "CIconTextTable.h"
#include "CMailAccountManager.h"
#include "CMailboxWindow.h"
#include "CMailCheckThread.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMboxList.h"
#include "CMboxRef.h"
#include "CMboxProtocol.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CSearchCriteria.h"
#include "CSearchEngine.h"
#include "CSearchItem.h"
#include "CStaticText.h"
#include "CTextListChoice.h"
#include "CVisualProgress.h"
#include "CWindowsMenu.h"
#include "CXStringResources.h"

#include <LDisclosureTriangle.h>
#include <LPopupButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S S E A R C H W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

CSearchWindow* CSearchWindow::sSearchWindow = nil;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

enum
{
	eStyle_SaveAs = 1,
	eStyle_Delete,
	eStyle_Separator1,
	eStyle_First
};

// Default constructor
CSearchWindow::CSearchWindow() :
	CSearchBase(false)
{
	sSearchWindow = this;
	CWindowsMenu::AddWindow(this);
}

// Constructor from stream
CSearchWindow::CSearchWindow(LStream *inStream) :
	LWindow(inStream),
	CSearchBase(false)
{
	sSearchWindow = this;
	CWindowsMenu::AddWindow(this);
}

// Default destructor
CSearchWindow::~CSearchWindow()
{
	sSearchWindow = nil;
	CWindowsMenu::RemoveWindow(this);

	SaveDefaultState();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Create it or bring it to the front
void CSearchWindow::CreateSearchWindow(CSearchItem* spec)
{
	CSearchEngine::sSearchEngine.ClearTargets();

	// Create find & replace window or bring to front
	if (sSearchWindow)
	{
		if (spec)
			sSearchWindow->SetStyle(spec);
		FRAMEWORK_WINDOW_TO_TOP(sSearchWindow)
	}
	else
	{
		CSearchWindow* search = (CSearchWindow*) LWindow::CreateWindow(paneid_SearchWindow, CMulberryApp::sApp);
		search->ResetState();
		if (spec)
			search->SetStyle(spec);
		search->Show();
	}
}

void CSearchWindow::DestroySearchWindow()
{
	FRAMEWORK_DELETE_WINDOW(sSearchWindow)
	sSearchWindow = nil;
}

// Add a mailbox
void CSearchWindow::AddMbox(const CMbox* mbox, bool reset)
{
	// Only add unique
	if (CSearchEngine::sSearchEngine.TargetsAddMbox(mbox))
	{
		// Reset visual display
		if (reset && sSearchWindow)
			sSearchWindow->mMailboxListPanel.ResetMailboxList(CSearchEngine::sSearchEngine.GetTargets(),
																CSearchEngine::sSearchEngine.GetTargetHits());
	}
}

// Add a list of mailboxes
void CSearchWindow::AddMboxList(const CMboxList* list)
{
	for(CMboxList::const_iterator iter = list->begin(); iter != list->end(); iter++)
		AddMbox(static_cast<const CMbox*>(*iter), false);

	// Reset visual display
	if (sSearchWindow)
		sSearchWindow->mMailboxListPanel.ResetMailboxList(CSearchEngine::sSearchEngine.GetTargets(),
																CSearchEngine::sSearchEngine.GetTargetHits());
}

void CSearchWindow::SearchAgain(CMbox* mbox)
{
	CSearchEngine::sSearchEngine.ClearTargets();
	CSearchWindow::AddMbox(mbox);

	// Fire off the search engine thread!
	CSearchEngine::sSearchEngine.StartSearch();
}

void CSearchWindow::SearchAgain(CMboxList* list)
{
	CSearchEngine::sSearchEngine.ClearTargets();
	CSearchWindow::AddMboxList(list);
}

void CSearchWindow::StartSearch()
{
	// Clear icons in list and set to top
	STableCell aCell(1, 1);
	mMailboxListPanel.mMailboxList->ScrollCellIntoFrame(aCell);
	mMailboxListPanel.mMailboxList->SetAllIcons(0);
	mMailboxListPanel.mFound->SetValue(0);
	mMailboxListPanel.mMessages->SetValue(0);
	mMailboxListPanel.mProgress->SetTotal(CSearchEngine::sSearchEngine.TargetSize());
	mMailboxListPanel.mProgress->Show();

	// Lock out UI
	SearchInProgress(true);
}

void CSearchWindow::NextSearch(unsigned long item)
{
	// Set pointer in list
	mMailboxListPanel.mMailboxList->SetIcon(item + 1, icnx_SearchPointer);
	STableCell aCell(item + 1, 1);
	mMailboxListPanel.mMailboxList->ScrollCellIntoFrame(aCell);
}

void CSearchWindow::EndSearch()
{
	// Set to top
	STableCell aCell(CSearchEngine::sSearchEngine.GetCurrentTarget() >= 0 ?
						CSearchEngine::sSearchEngine.GetCurrentTarget() + 1 : 1, 1);
	mMailboxListPanel.mMailboxList->ScrollCellIntoFrame(aCell);

	mMailboxListPanel.mProgress->Hide();

	// Enable UI
	SearchInProgress(false);

	// Do open first if required
	if (CPreferences::sPrefs->mOpenFirstSearchResult.GetValue())
		mMailboxListPanel.mMailboxList->SelectCell(aCell);
}

void CSearchWindow::SetProgress(unsigned long progress)
{
	mMailboxListPanel.SetProgress(progress);
}

void CSearchWindow::SetFound(unsigned long found)
{
	mMailboxListPanel.SetFound(found);
}

void CSearchWindow::SetMessages(unsigned long msgs)
{
	mMailboxListPanel.SetMessages(msgs);
}

void CSearchWindow::SetHitState(unsigned long item, bool hit, bool clear)
{
	mMailboxListPanel.SetHitState(item, hit, clear);
}

#pragma mark ____________________________Visual, Commands, Messages

// Do various bits
void CSearchWindow::FinishCreateSelf()
{
	// Do inherited
	LWindow::FinishCreateSelf();

	// Get all controls
	mSearchStyles = (LPopupButton*) FindPaneByID(paneid_SearchSearchSets);
	mScroller = (LView*) FindPaneByID(paneid_SearchScroller);
	mCriteria = (LView*) FindPaneByID(paneid_SearchCriteriaArea);
	mCriteriaMove = (LView*) FindPaneByID(paneid_SearchCriteriaMove);
	mMoreBtn = (LPushButton*) FindPaneByID(paneid_SearchMore);
	mFewerBtn = (LPushButton*) FindPaneByID(paneid_SearchFewer);
	mClearBtn = (LPushButton*) FindPaneByID(paneid_SearchClear);
	mSearchBtn = (LPushButton*) FindPaneByID(paneid_SearchSearch);
	mSearchBtn->SetDefaultButton(true);
	mCancelBtn = (LPushButton*) FindPaneByID(paneid_SearchCancel);
	mBottomArea = (LView*) FindPaneByID(paneid_SearchBottomArea);
	
	mMailboxListPanel.FinishCreateSelf(this, mBottomArea);
	mMailboxListPanel.mMailboxList->AddListener(this);

	mMailboxListPanel.ResetMailboxList(CSearchEngine::sSearchEngine.GetTargets(),
										CSearchEngine::sSearchEngine.GetTargetHits());

	// Force mailbox sets to dissappear
	SBooleanRect binding;
	mCriteriaMove->GetFrameBinding(binding);
	binding.bottom = false;
	mCriteriaMove->SetFrameBinding(binding);
	mMailboxListPanel.OnTwist();
	binding.bottom = true;
	mCriteriaMove->SetFrameBinding(binding);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CSearchWindowBtns);

	// Initialise items
	InitStyles();
	InitCriteria();

	// Make sure changes to cabinets are notified
	CMailAccountManager::sMailAccountManager->Add_Listener(this);
}

// Activate search item
void CSearchWindow::ActivateSelf(void)
{
	// Active first criteria that wants to acticate
	DoActivate();

	LWindow::ActivateSelf();
}

Boolean CSearchWindow::AttemptQuitSelf(SInt32 inSaveOption)
{
	// Cancel and wait for search termination
	WaitForCancel();

	// Do inherited
	return LWindow::AttemptQuitSelf(inSaveOption);
}

void CSearchWindow::AttemptClose(void)
{
	// Cancel and wait for search termination
	WaitForCancel();

	// Do inherited
	LWindow::AttemptClose();
}

void CSearchWindow::WaitForCancel()
{
	// Cancel and wait for search termination
	if (mMailboxListPanel.mInProgress)
	{
		// Begin a busy operation
		StMailBusy busy_lock(NULL, NULL);

		OnCancel();
		while(!CSearchEngine::sSearchEngine.AbortCompleted())
			CMailControl::ProcessBusy(NULL);
	}
}

// Handle key presses
Boolean CSearchWindow::HandleKeyPress(const EventRecord &inKeyEvent)
{
	switch (inKeyEvent.message & charCodeMask)
	{
		// Edit the address
		case char_Return:
		case char_Enter:
			if (mSearchBtn->IsEnabled())
				mSearchBtn->SimulateHotSpotClick(kControlButtonPart);
			break;

		default:
			return LWindow::HandleKeyPress(inKeyEvent);
			break;
	}

	return true;
}

// Respond to clicks in the icon buttons
void CSearchWindow::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
		case msg_SearchSearchSets:
			OnStyles(*(long*) ioParam);
			break;

		case msg_SearchMore:
			OnMore();
			break;

		case msg_SearchFewer:
			OnFewer();
			break;

		case msg_SearchClear:
			OnClear();
			break;

		case msg_SearchSearch:
			OnSearch();
			break;

		case msg_SearchCancel:
			OnCancel();
			break;

		case msg_MailboxListTwist:
		{
			SBooleanRect binding;
			mCriteriaMove->GetFrameBinding(binding);
			binding.bottom = false;
			mCriteriaMove->SetFrameBinding(binding);
			mMailboxListPanel.OnTwist();
			binding.bottom = true;
			mCriteriaMove->SetFrameBinding(binding);
			break;
		}

		case msg_MailboxListCabinets:
			mMailboxListPanel.OnCabinet(*(long*) ioParam);
			break;

		case msg_MailboxListMailboxAdd:
			mMailboxListPanel.OnAddMailboxList();
			break;

		case msg_MailboxListMailboxClear:
			mMailboxListPanel.OnClearMailboxList();
			break;

		case msg_SearchOptions:
			mMailboxListPanel.OnOptions();
			break;

		case msg_MailboxListMailboxList:
			mMailboxListPanel.OnOpenMailboxList();
			break;
	}
}

// Respond to list changes
void CSearchWindow::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CMboxProtocol::eBroadcast_NewList:
		// Force reset
		mMailboxListPanel.InitCabinets();
		break;

	case CMboxProtocol::eBroadcast_RemoveList:
		if (typeid(*static_cast<CTreeNodeList*>(param)) == typeid(CMboxRefList))
		{
			unsigned long index = CMailAccountManager::sMailAccountManager->GetFavouriteType(static_cast<CMboxRefList*>(param));
			::DeleteMenuItem(mMailboxListPanel.mCabinets->GetMacMenuH(), index + 1);
		}
		break;
	default:
		CSearchBase::ListenTo_Message(msg, param);
		break;
	}
}

#pragma mark ____________________________Commands

void CSearchWindow::SearchInProgress(bool searching)
{
	if (searching)
	{
		// Disable controls
		mSearchStyles->Disable();
		mCriteria->Disable();
		mMoreBtn->Disable();
		mFewerBtn->Disable();
		mClearBtn->Disable();
		mSearchBtn->Hide();
		mSearchBtn->Disable();
		mCancelBtn->Enable();
		mCancelBtn->Show();
	}
	else
	{
		// Enable controls
		mSearchStyles->Enable();
		mCriteria->Enable();
		mMoreBtn->Enable();
		mFewerBtn->Enable();
		mClearBtn->Enable();
		mCancelBtn->Hide();
		mCancelBtn->Disable();
		mSearchBtn->Enable();
		mSearchBtn->Show();
	}

	mMailboxListPanel.SetInProgress(searching);
}

void CSearchWindow::OnStyles(long item)
{
	switch(item)
	{
	case eStyle_SaveAs:
		SaveStyleAs();
		break;
	case eStyle_Delete:
		DeleteStyle();
		break;
	default:
		SetStyle(CPreferences::sPrefs->mSearchStyles.GetValue().at(item - eStyle_First)->GetSearchItem());
	}
}

void CSearchWindow::OnSearch()
{
	CSearchItem* spec = ConstructSearch();
	CSearchEngine::sSearchEngine.SetSearchItem(spec);
	delete spec;

	// Fire off the search engine thread!
	CSearchEngine::sSearchEngine.StartSearch();
}

void CSearchWindow::OnCancel()
{
	CSearchEngine::sSearchEngine.Abort();
}

#pragma mark ____________________________Styles

void CSearchWindow::InitStyles()
{
	// Remove any existing items from main menu
	short num_menu = ::CountMenuItems(mSearchStyles->GetMacMenuH());
	for(short i = eStyle_First; i <= num_menu; i++)
		::DeleteMenuItem(mSearchStyles->GetMacMenuH(), eStyle_First);

	short index = eStyle_First;
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++, index++)
		::AppendItemToMenu(mSearchStyles->GetMacMenuH(), index, (*iter)->GetName());

	// Force max/min update
	mSearchStyles->SetMenuMinMax();
	StopListening();
	mSearchStyles->SetValue(0);
	StartListening();
}

void CSearchWindow::SaveStyleAs()
{
	// Get a new name for the mailbox (use old name as starter)
	cdstring style_name;
	if (CGetStringDialog::PoseDialog("Alerts::Search::SaveStyleAs", style_name))
	{
		// Create new style
		CSearchStyle* style = new CSearchStyle(style_name, ConstructSearch());
		CPreferences::sPrefs->mSearchStyles.Value().push_back(style);
		CPreferences::sPrefs->mSearchStyles.SetDirty();

		// Reset menu
		InitStyles();
	}
}

void CSearchWindow::DeleteStyle()
{
	cdstrvect items;
	cdstring text;
	ulvector selection;

	// Add all styles to list
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++)
		items.push_back((*iter)->GetName());

	if (CTextListChoice::PoseDialog("Alerts::Search::DeleteStyleTitle", "Alerts::Search::DeleteStyleDesc", NULL,
									false, false, false, false, items, text, selection,
									"Alerts::Search::DeleteStyleButton") && selection.size())
	{
		// Delete styles in reverse
		CSearchStyleList& list = CPreferences::sPrefs->mSearchStyles.Value();
		for(ulvector::reverse_iterator riter = selection.rbegin(); riter != selection.rend(); riter++)
			list.erase(list.begin() + *riter);
		CPreferences::sPrefs->mSearchStyles.SetDirty();

		// Reset menu
		InitStyles();
	}
}

void CSearchWindow::SetStyle(const CSearchItem* spec)
{
	// Remove all
	RemoveAllCriteria();

	// Reset current item
	CSearchEngine::sSearchEngine.SetSearchItem(spec);

	InitCriteria(CSearchEngine::sSearchEngine.GetSearchItem());
}

#pragma mark ____________________________Criteria Panels

const long cMaxWindowHeight = 400L;

void CSearchWindow::Resized(int dy)
{
	mCriteria->ResizeFrameBy(0, dy, true);
	mScroller->ResizeImageBy(0, dy, true);
	
	// Always scroll to bottom if enlarging
//	if (dy > 0)
//		mScroller->ScrollPinnedImageBy(0, dy, true);

}

#pragma mark ____________________________Window State

// Reset state from prefs
void CSearchWindow::ResetState(bool force)
{
	// Get name as cstr
	char name = 0;

	// Get default state
	CSearchWindowState* state = &CPreferences::sPrefs->mSearchWindowDefault.Value();

	// Set button states before window resize
	mMailboxListPanel.mTwister->SetValue(state->GetExpanded());

	// Do not set if empty
	Rect set_rect = state->GetBestRect(CPreferences::sPrefs->mSearchWindowDefault.GetValue());
	if (set_rect.left && set_rect.top && set_rect.right && set_rect.bottom)
	{
		// Clip to screen
		::RectOnScreen(set_rect, this);

		// Reset bounds
		DoSetBounds(set_rect);
	}

	// If forced reset, save it
	if (force)
		SaveDefaultState();
}

// Save current state in prefs
void CSearchWindow::SaveDefaultState(void)
{
	// Get bounds - convert to position only
	Rect bounds = mUserBounds;

	// Add info to prefs
	CSearchWindowState state(nil, &bounds, eWindowStateNormal, mMailboxListPanel.mTwister->GetValue());
	if (CPreferences::sPrefs->mSearchWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mSearchWindowDefault.SetDirty();
}
