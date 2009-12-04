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

#include "CBetterScrollbarSet.h"
#include "CDrawUtils.h"
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
#include "CMulberryWMClass.h"
#include "CPreferences.h"
#include "CResizeNotifier.h"
#include "CSearchCriteria.h"
#include "CSearchEngine.h"
#include "CSearchListPanel.h"
#include "CSearchItem.h"
#include "CTextListChoice.h"
#include "CTwister.h"
#include "CWindowsMenu.h"

#include "TPopupMenu.h"
#include "HResourceMap.h"

#include <JXColormap.h>
#include <JXDisplay.h>
#include <JXProgressIndicator.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>
#include <JXWindowPainter.h>

#include <cassert>
#include <typeinfo>

void CBlankScrollable::Init()
{
	// Always show scrollbars
	AlwaysShowScrollbars();

	// Fit to enclosure
	FitToEnclosure();

	// Adjust bounds to size of aperture
	AdjustBounds(-2, 0);

	SetFocusColor((GetColormap())->GetDefaultBackColor());
}

void CBlankScrollable::Draw(JXWindowPainter& p, const JRect& rect)
{
	// Draw grey background
	CDrawUtils::DrawBackground(p, rect, false, false);
}

void CBlankScrollable::AdjustBounds(const JCoordinate dw, const JCoordinate dh)
{
	// Adjust bounds based on input
	JRect boundsG = GetBoundsGlobal();
	SetBounds(boundsG.width() + dw, boundsG.height() + dh);
}

// Always tie bounds width to aperture
void CBlankScrollable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	if (dw != 0)
		AdjustBounds(dw, 0);
	JXScrollableWidget::ApertureResized(dw, dh);
}

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S S E A R C H W I N D O W
// __________________________________________________________________________________________________

// Static 

// Static members

CSearchWindow* CSearchWindow::sSearchWindow = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

enum
{
	eStyle_SaveAs = 1,
	eStyle_Delete,
	eStyle_First
};

// Default constructor
CSearchWindow::CSearchWindow(JXDirector* supervisor) : CWindow(supervisor), CSearchBase(false)
{
	sSearchWindow = this;
	CWindowsMenu::AddWindow(this);

	mPendingResize = false;
}

// Default destructor
CSearchWindow::~CSearchWindow()
{
	sSearchWindow = NULL;
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
		CSearchWindow* search = new CSearchWindow(CMulberryApp::sApp);
		search->OnCreate();
		search->ResetState();
		if (spec)
			search->SetStyle(spec);
		//search->GetWindow()->LockCurrentSize();
	 	search->Activate();
	}
}
void CSearchWindow::DestroySearchWindow()
{
	if (sSearchWindow)
		FRAMEWORK_DELETE_WINDOW(sSearchWindow)
	sSearchWindow = NULL;
}

// Add a mailbox
void CSearchWindow::AddMbox(const CMbox* mbox, bool reset)
{
	// Only add unique
	if (CSearchEngine::sSearchEngine.TargetsAddMbox(mbox))
	{
		// Reset visual display
		if (reset && sSearchWindow)
			sSearchWindow->mMailboxListPanel->ResetMailboxList(CSearchEngine::sSearchEngine.GetTargets(),
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
		sSearchWindow->mMailboxListPanel->ResetMailboxList(CSearchEngine::sSearchEngine.GetTargets(),
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
	mMailboxListPanel->mMailboxList->ScrollCellIntoFrame(aCell);
	mMailboxListPanel->mMailboxList->SetAllIcons(0);
	mMailboxListPanel->mFound->SetText("0");
	mMailboxListPanel->mMessages->SetText("0");
	mMailboxListPanel->mProgress->SetValue(0);
	mMailboxListPanel->mProgress->SetMaxValue(CSearchEngine::sSearchEngine.TargetSize() ? CSearchEngine::sSearchEngine.TargetSize() : 1);
	mMailboxListPanel->mProgress->Show();

	// Lock out UI
	SearchInProgress(true);
}

void CSearchWindow::NextSearch(unsigned long item)
{
	// Set pointer in list
	mMailboxListPanel->mMailboxList->SetIcon(item + 1, IDI_SEARCH_POINTER);
	STableCell aCell(item + 1, 1);
	mMailboxListPanel->mMailboxList->ScrollCellIntoFrame(aCell);
}

void CSearchWindow::EndSearch()
{
	// Set to top
	STableCell aCell(CSearchEngine::sSearchEngine.GetCurrentTarget() >= 0 ?
						CSearchEngine::sSearchEngine.GetCurrentTarget() + 1 : 1, 1);
	mMailboxListPanel->mMailboxList->ScrollCellIntoFrame(aCell);
		
	mMailboxListPanel->mProgress->Hide();

	// Enable UI
	SearchInProgress(false);
	
	// Do open first if required
	if (CPreferences::sPrefs->mOpenFirstSearchResult.GetValue())
		mMailboxListPanel->mMailboxList->SelectCell(aCell);
}

void CSearchWindow::SetProgress(unsigned long progress)
{
	mMailboxListPanel->SetProgress(progress);
}

void CSearchWindow::SetFound(unsigned long found)
{
	mMailboxListPanel->SetFound(found);
}

void CSearchWindow::SetMessages(unsigned long msgs)
{
	mMailboxListPanel->SetMessages(msgs);
}

void CSearchWindow::SetHitState(unsigned long item, bool hit, bool clear)
{
	mMailboxListPanel->SetHitState(item, hit, clear);
}

#pragma mark ____________________________Visual, Commands, Messages

// Do various bits
void CSearchWindow::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 550,385, "");
    assert( window != NULL );
    SetWindow(window);

    CResizeNotifier* container =
        new CResizeNotifier(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,30, 550,355);
    assert( container != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Search Set:", container,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 433,12, 75,20);
    assert( obj1 != NULL );

    mSearchStyles =
        new HPopupMenu("",container,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 510,10, 30,20);
    assert( mSearchStyles != NULL );

    mBottomArea =
        new JXWidgetSet(container,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 2,120, 546,233);
    assert( mBottomArea != NULL );

    mMoreBtn =
        new JXTextButton("More Choices", mBottomArea,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 8,5, 105,25);
    assert( mMoreBtn != NULL );

    mFewerBtn =
        new JXTextButton("Fewer Choices", mBottomArea,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 123,5, 105,25);
    assert( mFewerBtn != NULL );

    mClearBtn =
        new JXTextButton("Reset", mBottomArea,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 388,5, 70,25);
    assert( mClearBtn != NULL );

    mSearchBtn =
        new JXTextButton("Search", mBottomArea,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 468,5, 70,25);
    assert( mSearchBtn != NULL );
    mSearchBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", mBottomArea,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 468,5, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mMailboxListPanel =
        new CSearchListPanel(mBottomArea,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,33, 496,200);
    assert( mMailboxListPanel != NULL );

    mScroller =
        new CBetterScrollbarSet(container,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,35, 530,85);
    assert( mScroller != NULL );

    mScrollPane =
        new CBlankScrollable(mScroller, mScroller->GetScrollEnclosure(), // mScroller,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 520,25);
    assert( mScrollPane != NULL );

    mCriteria =
        new JXEngravedRect(mScrollPane,
                    JXWidget::kHElastic, JXWidget::kVElastic, 3,10, 515,10);
    assert( mCriteria != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Find Messages whose:", mScrollPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,1, 146,15);
    assert( obj2 != NULL );

// end JXLayout

	// Do scroll pane init (set background to gray)
	mScrollPane->Init();
	mScroller->SetAllowScroll(false, true);

	// Set window details
	window->SetWMClass(cSearchWMClass, cMulberryWMClass);
	window->SetIcon(iconFromResource(IDR_SEARCHDOCSDI, window, 16, 0x00CCCCCC));
	window->SetTitle("Message Search");

	// Create the menu bar
	CreateMainMenu(window, CMainMenu::fFile | 
								 CMainMenu::fEdit | 
								 CMainMenu::fMailboxes | 
								 CMainMenu::fAddresses | 
								 CMainMenu::fCalendar | 
								 CMainMenu::fWindows | 
								 CMainMenu::fHelp);

	// Create the mailbox list panel
	mMailboxListPanel->OnCreate(this, mBottomArea, mScroller);
	mMailboxListPanel->ResetMailboxList(CSearchEngine::sSearchEngine.GetTargets(),
										CSearchEngine::sSearchEngine.GetTargetHits());

	// Adjust size to wider window (fdsign size if based on width of 500)
	mMailboxListPanel->AdjustSize(GetWindow()->GetBoundsWidth() - 504, 0);

	// Force mailbox sets to dissappear
	mMailboxListPanel->OnTwist();

	// Set current width and height as minimum
	window->SetMinSize(550, 230);

	// Set button state here as criteria set may change this
	mCancelBtn->Hide();
	mFewerBtn->Hide();

	// Initialise styles and criteria
	InitStyles();
	InitCriteria(CSearchEngine::sSearchEngine.GetSearchItem());

	// Listen to UI items
	ListenTo(container);
	ListenTo(mSearchStyles);
	ListenTo(mMoreBtn);
	ListenTo(mFewerBtn);
	ListenTo(mClearBtn);
	ListenTo(mSearchBtn);
	ListenTo(mCancelBtn);
	ListenTo(mMailboxListPanel->mTwister);
	ListenTo(mMailboxListPanel->mCabinets);
	ListenTo(mMailboxListPanel->mAddListBtn);
	ListenTo(mMailboxListPanel->mClearListBtn);
	ListenTo(mMailboxListPanel->mOptionsBtn);
	ListenTo(mMailboxListPanel->mMailboxList);

	// Make sure changes to cabinets are notified
	CMailAccountManager::sMailAccountManager->Add_Listener(this);

} // CSearchWindow::FinishCreateSelf

void CSearchWindow::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	if (cmdui->mMenu)
	{
		if (cmdui->mMenu == mSearchStyles)
		{
			cmdui->mMenu->EnableAll();
			return;
		}
		else if (cmdui->mMenu == mMailboxListPanel->mCabinets)
		{
			cmdui->mMenu->EnableAll();
			return;
		}
	}

	CWindow::UpdateCommand(cmd, cmdui);
}

bool CSearchWindow::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	if (menu)
	{
		if (menu->mMenu == mSearchStyles)
		{
			OnStyles(menu->mIndex);
			return true;
		}
		else if (menu->mMenu == mMailboxListPanel->mCabinets)
		{
			mMailboxListPanel->OnCabinet(menu->mIndex);
			return true;
		}
	}

	return CWindow::ObeyCommand(cmd, menu);
}

void CSearchWindow::Activate()
{
	// Do inherited
	CWindow::Activate();

	// Make it the commander target
	SetTarget(this);
}

void CSearchWindow::WindowFocussed(JBoolean focussed)
{
	// Do inherited
	CWindow::WindowFocussed(focussed);

	// Make it the commander target
	if (focussed)
		SetTarget(this);
}

// Respond to clicks in the icon buttons
void CSearchWindow::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(CResizeNotifier::kResized))
	{
		const CResizeNotifier::Resized* info =
			dynamic_cast<const CResizeNotifier::Resized*>(&message);
		BoundsResized(info->dw(), info->dh());
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mMoreBtn)
		{
			OnMore();
			return;
		}
		else if (sender == mFewerBtn)
		{
			OnFewer();
			return;
		}
		else if (sender == mClearBtn)
		{
			OnClear();
			return;
		}
		else if (sender == mSearchBtn)
		{
			OnSearch();
			return;
		}
		else if (sender == mCancelBtn)
		{
			OnCancel();
			return;
		}
		else if (sender == mMailboxListPanel->mAddListBtn)
		{
			mMailboxListPanel->OnAddMailboxList();
			return;
		}
		else if (sender == mMailboxListPanel->mClearListBtn)
		{
			mMailboxListPanel->OnClearMailboxList();
			return;
		}
		else if (sender == mMailboxListPanel->mOptionsBtn)
		{
			mMailboxListPanel->OnOptions();
			return;
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mMailboxListPanel->mTwister)
		{
			// Allow resize of fixed-size dialog window
			mMailboxListPanel->OnTwist();
			mPendingResize = true;
			return;
		}
	}
	else if (sender == mMailboxListPanel->mMailboxList)
	{
		if (message.Is(CTextTable::kLDblClickCell))
		{
			mMailboxListPanel->OnOpenMailboxList();
			return;
		}
	}

	CWindow::Receive(sender, message);
}

// Respond to list changes
void CSearchWindow::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CMboxProtocol::eBroadcast_NewList:
		// Force reset
		mMailboxListPanel->InitCabinets();
		break;

	case CMboxProtocol::eBroadcast_RemoveList:
		if (typeid(*static_cast<CTreeNodeList*>(param)) == typeid(CMboxRefList))
		{
			unsigned long index = CMailAccountManager::sMailAccountManager->GetFavouriteType(static_cast<CMboxRefList*>(param));
			mMailboxListPanel->mCabinets->RemoveItem(index + 1);
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
		mSearchStyles->Deactivate();
		mCriteria->Deactivate();
		mMoreBtn->Deactivate();
		mFewerBtn->Deactivate();
		mClearBtn->Deactivate();
		mSearchBtn->Hide();
		mSearchBtn->Deactivate();
		mCancelBtn->Activate();
		mCancelBtn->Show();
	}
	else
	{
		// Enable controls
		mSearchStyles->Activate();
		mCriteria->Activate();
		mMoreBtn->Activate();
		mFewerBtn->Activate();
		mClearBtn->Activate();
		mCancelBtn->Hide();
		mCancelBtn->Deactivate();
		mSearchBtn->Activate();
		mSearchBtn->Show();
	}

	mMailboxListPanel->SetInProgress(searching);
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
	// Must have items in the list
	if (!mMailboxListPanel->mMailboxList->GetItemCount())
		return;

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
	mSearchStyles->RemoveAllItems();
	mSearchStyles->SetMenuItems("Save As... | Delete... %l");
	
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++)
		mSearchStyles->AppendItem((*iter)->GetName(), kFalse, kFalse);
}

void CSearchWindow::SaveStyleAs()
{
	// Get a new name for the mailbox (use old name as starter)
	cdstring style_name;
	if (CGetStringDialog::PoseDialog("Alerts::Search::SaveStyleAsTitle", "Alerts::Search::SaveStyleAs", style_name))
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

	if (CTextListChoice::PoseDialog("Alerts::Search::DeleteStyleTitle", "Alerts::Search::DeleteStyleDesc", "Alerts::Search::DeleteStyleDesc",
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

void CSearchWindow::Resized(int dy)
{
	// Adjust scroll pane and scroll to bottom if scroller active
	mScrollPane->AdjustBounds(0, dy);
	if (dy > 0)
		mScrollPane->Scroll(0, -dy);
}

void CSearchWindow::BoundsResized(const JCoordinate dw, const JCoordinate dh)
{
	// Check for pending update on expand/collapse
	if (mPendingResize)
	{
		mPendingResize = false;
	}
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
	mMailboxListPanel->mTwister->SetState(JBoolean(state->GetExpanded()));

	// Do not set if empty
	JRect set_rect = state->GetBestRect(CPreferences::sPrefs->mSearchWindowDefault.GetValue());
	if (set_rect.left && set_rect.top)
	{
		// Clip to screen
		::RectOnScreen(set_rect);

		// Reset position
		GetWindow()->Place(set_rect.left, set_rect.top);
		GetWindow()->SetSize(set_rect.width(), set_rect.height());
	}

	// If forced reset, save it
	if (force)
		SaveDefaultState();
}

// Save current state in prefs
void CSearchWindow::SaveDefaultState(void)
{
	// Get bounds - convert to position only
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);

	// Add info to prefs
	CSearchWindowState state(NULL, &bounds, eWindowStateNormal, mMailboxListPanel->mTwister->IsChecked());
	if (CPreferences::sPrefs->mSearchWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mSearchWindowDefault.SetDirty();
}
