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


// Source for CMailboxInfoView class

#include "CMailboxInfoView.h"

#include "CCommands.h"
#include "CErrorHandler.h"
#include "CDivider.h"
#include "CFocusBorder.h"
#include "CIconLoader.h"
#include "CLetterWindow.h"
#include "CMailAccountManager.h"
#include "CMailboxTitleTable.h"
#include "CMailboxInfoTable.h"
#include "CMailboxInfoToolbar.h"
#include "CMailboxInfoWindow.h"
#include "CMboxProtocol.h"
#include "CMbox.h"
#include "CMessageWindow.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CProgressBar.h"
#include "CQuickSearchSavedPopup.h"
#include "CQuickSearchText.h"
#include "CSearchWindow.h"
#include "CSplitterView.h"
#include "CTableScrollbarSet.h"
#include "CToolbarView.h"
#include "CWindowsMenu.h"
#include "C3PaneMailboxToolbar.h"
#include "C3PaneWindow.h"

#include "HResourceMap.h"

#include <JXFlatRect.h>
#include <JXImageWidget.h>
#include "JXMultiImageButton.h"
#include "JXMultiImageCheckbox.h"
#include <JXStaticText.h>
#include <JXUpRect.h>

#include <algorithm>
#include <cassert>
#include <string.h>
#include <numeric>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxInfoView::CMailboxInfoView(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
	: CMailboxView(enclosure, hSizing, vSizing, x, y, w, h),
		JXIdleTask(100)
{
	mNoSearchReset = false;
	mShowQuickSearch = true;
	mQuickSearchTimerPending = false;
}

// Default destructor
CMailboxInfoView::~CMailboxInfoView()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cTitleHeight = 16;

// Setup help balloons
void CMailboxInfoView::OnCreate()
{
	// Do inherited
	CMailboxView::OnCreate();

	// Create scrollbars
// begin JXLayout1

    CFocusBorder* focus_border =
        new CFocusBorder(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 500,200);
    assert( focus_border != NULL );

    mQuickSearchArea =
        new JXFlatRect(focus_border,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 494,30);
    assert( mQuickSearchArea != NULL );

    mQuickSearchIndicator =
        new JXImageWidget(mQuickSearchArea,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 4,7, 16,16);
    assert( mQuickSearchIndicator != NULL );

    mQuickSearchCriteria =
        new HPopupMenu("",mQuickSearchArea,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 25,5, 130,20);
    assert( mQuickSearchCriteria != NULL );

    mQuickSearchText =
        new CQuickSearchText(mQuickSearchArea,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 160,5, 305,20);
    assert( mQuickSearchText != NULL );

    mQuickSearchSaved =
        new CQuickSearchSavedPopup("",mQuickSearchArea,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 158,5, 190,20);
    assert( mQuickSearchSaved != NULL );

    mQuickSearchProgress =
        new CProgressBar(mQuickSearchArea,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 159,5, 290,20);
    assert( mQuickSearchProgress != NULL );

    mQuickSearchCancel =
        new JXMultiImageButton(mQuickSearchArea,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 470,5, 24,22);
    assert( mQuickSearchCancel != NULL );

    mScroller =
        new CTableScrollbarSet(focus_border,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,30, 494,144);
    assert( mScroller != NULL );

    mSortDirectionBtn =
        new JXMultiImageCheckbox(mScroller,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 478,0, 16,16);
    assert( mSortDirectionBtn != NULL );

    JXUpRect* obj1 =
        new JXUpRect(focus_border,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 0,174, 494,20);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Total:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,2, 40,16);
    assert( obj2 != NULL );

    mTotal =
        new JXStaticText("000000", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 45,2, 50,16);
    assert( mTotal != NULL );

    CDivider* obj3 =
        new CDivider(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 95,0, 2,16);
    assert( obj3 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Unseen:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 100,2, 50,16);
    assert( obj4 != NULL );

    mUnseen =
        new JXStaticText("000000", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 150,2, 50,16);
    assert( mUnseen != NULL );

    CDivider* obj5 =
        new CDivider(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 200,0, 2,16);
    assert( obj5 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Deleted:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 205,2, 50,16);
    assert( obj6 != NULL );

    mDeleted =
        new JXStaticText("000000", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 255,2, 50,16);
    assert( mDeleted != NULL );

    CDivider* obj7 =
        new CDivider(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 305,0, 2,16);
    assert( obj7 != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Found:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 310,2, 45,16);
    assert( obj8 != NULL );

    mMatched =
        new JXStaticText("000000", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 355,2, 50,16);
    assert( mMatched != NULL );

    CDivider* obj9 =
        new CDivider(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 405,0, 2,16);
    assert( obj9 != NULL );

// end JXLayout1

	// Only use focus if 3pane
	if (!Is3Pane())
	{
		focus_border->HasFocus(false);
		focus_border->SetBorderWidth(0);
	}

	mQuickSearchIndicator->SetVisible(kFalse);
	mQuickSearchIndicator->SetImage(CIconLoader::GetIcon(IDI_MAILBOXSEARCH, mQuickSearchIndicator, 16, 0x00CCCCCC), kFalse);

	mQuickSearchCriteria->SetMenuItems("From %r | To %r | Recipient %r | Correspondent %r | Subject %r | Body %r %l | Saved Search %r");
	mQuickSearchCriteria->SetUpdateAction(JXMenu::kDisableNone);
	mQuickSearchCriteria->SetToPopupChoice(kTrue, eQuickSearchFrom);

	mQuickSearchText->Add_Listener(this);
	mQuickSearchSaved->OnCreate();
	
	mQuickSearchCancel->SetImage(IDI_QUICKSEARCHCANCEL);
	mQuickSearchCancel->SetActive(kFalse);

	// Create sort button positioned inside scrollbars
	mSortBtn = mSortDirectionBtn;
	mSortDirectionBtn->SetImages(IDI_SORTASCENDING, 0, IDI_SORTDESCENDING, 0);
	mSortDirectionBtn->SetHint(stringFromResource(IDC_MAILBOXSORTBTN));
	ListenTo(mSortDirectionBtn);

	mTitles = new CMailboxTitleTable(mScroller, mScroller->GetScrollEnclosure(),
										JXWidget::kHElastic, JXWidget::kFixedTop,
											 0, 0, 500, cTitleHeight);

	mTable = new CMailboxInfoTable(mScroller, mScroller->GetScrollEnclosure(),
								 JXWidget::kHElastic, JXWidget::kVElastic,
								 0, cTitleHeight, 500, mScroller->GetApertureHeight());
	
	mTable->OnCreate();
	mTitles->OnCreate();

	PostCreate(mTable, mTitles);
	
	// Always start disabled until mailbox is set
	Deactivate();
	
	// Init quick search area
	mQuickSearchProgress->SetVisible(kFalse);
	DoQuickSearchPopup(CPreferences::sPrefs->mQuickSearch.GetValue());

	// Hide quick search if user does not want it
	if (!CPreferences::sPrefs->mQuickSearchVisible.GetValue())
		ShowQuickSearch(false);
	
	ListenTo(mQuickSearchCriteria);
	ListenTo(mQuickSearchSaved);
	ListenTo(mQuickSearchCancel);

	// Need to force focus to table first
	GetWindow()->UnregisterFocusWidget(mQuickSearchText);
	GetWindow()->RegisterFocusWidget(mQuickSearchText);

	// Set status
	SetOpen();	
}

CMailboxToolbarPopup* CMailboxInfoView::GetCopyBtn() const

{
	if (Is3Pane())
	{
		// Copy btn from toolbar
		return C3PaneWindow::s3PaneWindow->GetMailboxToolbar()->GetCopyBtn();
	}
	else
	{
		// Match popup from toolbar
		return static_cast<CMailboxInfoToolbar*>(mToolbar)->GetCopyBtn();
	}
}

void CMailboxInfoView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CQuickSearchText::eBroadcast_Return:
		CMulberryApp::sApp->RemovePermanentTask(this);
		DoQuickSearch(true);
		break;
	case CQuickSearchText::eBroadcast_Tab:
		if (mQuickSearchTimerPending)
		{
			CMulberryApp::sApp->RemovePermanentTask(this);
			DoQuickSearch(true);
		}
		else
			GetInfoTable()->Focus();
		break;
		break;
	case CQuickSearchText::eBroadcast_Key:
		StartQuickSearchTimer();
		break;
	default:
		CMailboxView::ListenTo_Message(msg, param);
		break;
	}
}

void CMailboxInfoView::Receive (JBroadcaster* sender, const Message& message) 
{
	if (message.Is(JXMenu::kItemSelected))
	{
		JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
		if (sender == mQuickSearchCriteria)
		{
			DoQuickSearchPopup(index);
			
			// Always force focus to the quick search text field
			if (index != eQuickSearchSaved)
				mQuickSearchText->Focus();
			return;
		}
		else if (sender == mQuickSearchSaved)
		{
			DoQuickSearchSavedPopup(index);
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mQuickSearchCancel)
		{
			DoQuickSearchCancel();
			return;
		}
	} 

	CMailboxView::Receive(sender, message);
}

// Make a toolbar appropriate for this view
void CMailboxInfoView::MakeToolbars(CToolbarView* parent)
{
	if (Is3Pane())
	{
		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(C3PaneWindow::s3PaneWindow->GetMailboxToolbar());
		C3PaneWindow::s3PaneWindow->GetMailboxToolbar()->AddCommander(GetTable());
	}
	else
	{
		// Create a suitable toolbar
		CMailboxInfoToolbar* tb = new CMailboxInfoToolbar(parent, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 128, 45);
		tb->OnCreate();
		mToolbar = tb;

		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(tb);
		
		// Now give toolbar to its view as standard buttons
		parent->AddToolbar(tb, GetTable(), CToolbarView::eStdButtonsGroup);

		// Force toolbar to be active
		Broadcast_Message(eBroadcast_ViewActivate, this);
	}
}

void CMailboxInfoView::OnUpdateWindowsShowQuickSearch(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	cdstring txt;
	txt.FromResource(mShowQuickSearch ? IDS_HIDEQUICKSEARCH : IDS_SHOWQUICKSEARCH);
	
	pCmdUI->SetText(txt);
}

void CMailboxInfoView::OnWindowsShowQuickSearch()
{
	ShowQuickSearch(!mShowQuickSearch);
	CPreferences::sPrefs->mQuickSearchVisible.SetValue(mShowQuickSearch);
}

// Enable quick search items
void CMailboxInfoView::EnableQuickSearch(bool enable)
{
	mQuickSearchCriteria->SetActive(enable ? kTrue : kFalse);
	mQuickSearchText->SetActive(enable ? kTrue : kFalse);
	mQuickSearchProgress->SetActive(enable ? kTrue : kFalse);
	mQuickSearchCancel->SetActive(kFalse);
	mQuickSearchIndicator->SetVisible(kFalse);
}

// Show quick search area
void CMailboxInfoView::ShowQuickSearch(bool show)
{
	if (!(mShowQuickSearch ^ show))
		return;

	int moveby = mQuickSearchArea->GetFrameHeight();

	if (show)
	{
		// Shrink/move table pane
		mScroller->AdjustSize(0, -moveby);
		mScroller->Move(0, moveby);

		// Show parts after all other changes
		mQuickSearchArea->SetVisible(kTrue);
	}
	else
	{
		// Hide parts before other changes
		mQuickSearchArea->SetVisible(kFalse);

		// Expand/move splitter
		mScroller->AdjustSize(0, moveby);
		mScroller->Move(0, -moveby);
	}

	mShowQuickSearch = show;
}

// Do quick search
void CMailboxInfoView::DoQuickSearchPopup(JIndex value)
{
	// Store current value in prefs
	CPreferences::sPrefs->mQuickSearch.SetValue(value - eQuickSearchFrom);
	
	// Hide/show saved popup
	if (value == eQuickSearchSaved)
	{
		mQuickSearchText->SetVisible(kFalse);
		mQuickSearchSaved->SetVisible(kTrue);
		mQuickSearchSaved->SetValue(CQuickSearchSavedPopup::eChoose);
	}
	else
	{
		mQuickSearchSaved->SetVisible(kFalse);
		mQuickSearchText->SetVisible(kTrue);
	}
}

// Do quick search
void CMailboxInfoView::DoQuickSearch(bool change_focus)
{
	CMulberryApp::sApp->RemovePermanentTask(this);
	mQuickSearchTimerPending = false;

	mQuickSearchText->SetVisible(kFalse);
	mQuickSearchProgress->SetVisible(kTrue);
	mQuickSearchProgress->SetIndeterminate(true);

	cdstring txt = mQuickSearchText->GetText();

	std::auto_ptr<CSearchItem> spec;
	switch(mQuickSearchCriteria->GetValue())
	{
	case eQuickSearchFrom:
		spec.reset(new CSearchItem(CSearchItem::eFrom, txt));
		break;
	case eQuickSearchTo:
		spec.reset(new CSearchItem(CSearchItem::eTo, txt));
		break;
	case eQuickSearchRecipient:
		spec.reset(new CSearchItem(CSearchItem::eRecipient, txt));
		break;
	case eQuickSearchCorrespondent:
		spec.reset(new CSearchItem(CSearchItem::eCorrespondent, txt));
		break;
	case eQuickSearchSubject:
		spec.reset(new CSearchItem(CSearchItem::eSubject, txt));
		break;
	case eQuickSearchBody:
		spec.reset(new CSearchItem(CSearchItem::eBody, txt));
		break;
	}
	
	// Make sure search failure does not prevent clean-up
	try
	{
		if (txt.length() != 0)
			GetInfoTable()->SetMatch(NMbox::eViewMode_ShowMatch, spec.get(), false);
		else
			GetInfoTable()->SetMatch(NMbox::eViewMode_All, NULL, false);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}

	mQuickSearchCancel->SetActive((txt.length() != 0) ? kTrue : kFalse);
	mQuickSearchIndicator->SetVisible((txt.length() != 0) ? kTrue : kFalse);
	if (change_focus)
		GetInfoTable()->Focus();

	mQuickSearchProgress->SetVisible(kFalse);
	mQuickSearchProgress->SetIndeterminate(false);
	mQuickSearchText->SetVisible(kTrue);
	if (!change_focus)
		mQuickSearchText->Focus();
}

// Do quick search saved
void CMailboxInfoView::DoQuickSearchSavedPopup(long value)
{
	if (value == CQuickSearchSavedPopup::eChoose)
	{
		GetInfoTable()->SetMatch(NMbox::eViewMode_All, NULL, false);
		mQuickSearchCancel->SetActive(kFalse);
		mQuickSearchIndicator->SetVisible(kFalse);
	}
	else
	{
		mQuickSearchSaved->SetVisible(kFalse);
		mQuickSearchProgress->SetVisible(kTrue);

		// Get search set
		std::auto_ptr<CSearchItem> spec(new CSearchItem(CSearchItem::eNamedStyle, CPreferences::sPrefs->mSearchStyles.GetValue().at(value - IDM_SEARCH_STYLES_Start)->GetName()));

		// Make sure search failure does not prevent clean-up
		try
		{
			GetInfoTable()->SetMatch(NMbox::eViewMode_ShowMatch, spec.get(), false);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}

		mQuickSearchCancel->SetActive(kTrue);
		mQuickSearchIndicator->SetVisible(kTrue);
		GetInfoTable()->Focus();

		mQuickSearchProgress->SetVisible(kFalse);
		mQuickSearchSaved->SetVisible(kTrue);
	}
}

// Cancel quick search
void CMailboxInfoView::DoQuickSearchCancel()
{
	GetInfoTable()->SetMatch(NMbox::eViewMode_All, NULL, false);
	mQuickSearchCancel->SetActive(kFalse);
	mQuickSearchIndicator->SetVisible(kFalse);
	
	if (mQuickSearchSaved->IsVisible())
	{
		StStopListening _no_listen(this);
		mQuickSearchSaved->SetValue(CQuickSearchSavedPopup::eChoose);
	}
}

// Sync quick search with current mailbox Match state
void CMailboxInfoView::SyncQuickSearch()
{
	// Ignore this if a search is already in progress as we are already sync'd
	if (mQuickSearchProgress->IsVisible())
		return;

	if (GetMbox() && (GetMbox()->GetViewMode() == NMbox::eViewMode_ShowMatch) && (GetMbox()->GetViewSearch() != NULL))
	{
		EnableQuickSearch(true);
		const CSearchItem* spec = GetMbox()->GetViewSearch();
		
		// Some types we can handle
		long menu_item = 0;
		cdstring txt;
		switch(spec->GetType())
		{
		case CSearchItem::eFrom:
		case CSearchItem::eTo:
		case CSearchItem::eRecipient:
		case CSearchItem::eCorrespondent:
		case CSearchItem::eSubject:
		case CSearchItem::eBody:
		{
			switch(spec->GetType())
			{
			case CSearchItem::eFrom:
				menu_item = eQuickSearchFrom;
				break;
			case CSearchItem::eTo:
				menu_item = eQuickSearchTo;
				break;
			case CSearchItem::eRecipient:
				menu_item = eQuickSearchRecipient;
				break;
			case CSearchItem::eCorrespondent:
				menu_item = eQuickSearchCorrespondent;
				break;
			case CSearchItem::eSubject:
				menu_item = eQuickSearchSubject;
				break;
			case CSearchItem::eBody:
				menu_item = eQuickSearchBody;
				break;
			default:;
			}
			if (spec->GetData() != NULL)
				txt = *reinterpret_cast<const cdstring*>(spec->GetData());
			
			// Now set controls
			JBroadcaster::StStopListening _no_listen(this);
			mQuickSearchCriteria->SetValue(menu_item);
			mQuickSearchText->SetText(txt);
			DoQuickSearchPopup(menu_item);
			mQuickSearchCancel->SetActive(kTrue);
			mQuickSearchIndicator->SetVisible(kTrue);
			break;
		}
		case CSearchItem::eNamedStyle:
		{
			if (spec->GetData() != NULL)
				txt = *reinterpret_cast<const cdstring*>(spec->GetData());
			menu_item = CPreferences::sPrefs->mSearchStyles.GetValue().FindIndexOf(txt);
			
			// Now set controls
			JBroadcaster::StStopListening _no_listen(this);
			mQuickSearchCriteria->SetValue(eQuickSearchSaved);
			DoQuickSearchPopup(eQuickSearchSaved);
			if (menu_item != -1)
				mQuickSearchSaved->SetValue(CQuickSearchSavedPopup::eFirst + menu_item);
			else
				mQuickSearchSaved->SetValue(CQuickSearchSavedPopup::eChoose);
			mQuickSearchCancel->SetActive(kTrue);
			mQuickSearchIndicator->SetVisible(kTrue);
			break;
		}
		default:;
		}
	}
	else
		// Enable/disable but not in quick search mode
		EnableQuickSearch(GetMbox() != NULL);
}

// Start timer to trigger quick search
void CMailboxInfoView::StartQuickSearchTimer()
{
	// Cache the time at which we trigger
	timeval tv;
	::gettimeofday(&tv, NULL);
	mQuickSearchTriggerTime = (tv.tv_sec & 0x003FFFFF) * 1000 + tv.tv_usec / 1000;

	// Start idle time processing
	CMulberryApp::sApp->InstallPermanentTask(this);
	mQuickSearchTimerPending = true;
}

void CMailboxInfoView::Perform(const Time delta, Time* maxSleepTime)
{
	*maxSleepTime = 500;
	Time bogus;
	if (!TimeToPerform(delta, &bogus))
		return;

	// See if we are equal or greater than trigger
	timeval tv;
	::gettimeofday(&tv, NULL);
	unsigned long new_time = (tv.tv_sec & 0x003FFFFF) * 1000 + tv.tv_usec / 1000;
	if (new_time > mQuickSearchTriggerTime + 500)
	{
		CMulberryApp::sApp->RemovePermanentTask(this);
		DoQuickSearch(false);
	}
}

// Set window state
void CMailboxInfoView::ResetState(bool force)
{
	CMbox* mbox = GetMbox();
	if (!mbox)
		return;

	// Get new name of this window
	cdstring name = mbox->GetAccountName();

	// Get window state from prefs
	CMailboxWindowState* state = CPreferences::sPrefs->GetMailboxWindowInfo(name);

	// If no prefs try default
	if (!state || force)
		state = &CPreferences::sPrefs->mMailboxWindowDefault.Value();

	// Do not set if empty
	JRect set_rect = state->GetBestRect(CPreferences::sPrefs->mMailboxWindowDefault.GetValue());
	if (!set_rect.IsEmpty())
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect);

			// Reset bounds
			GetMailboxWindow()->GetWindow()->Place(set_rect.left, set_rect.top);
			GetMailboxWindow()->GetWindow()->SetSize(set_rect.width(), set_rect.height());
		}
			
	}

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mMailboxWindowDefault.GetValue()));

	// Do match BEFORE sorting so that sort is only done on the matched set rather than being done twice

	// Only do search state if required
	if (!mNoSearchReset)
	{
		// Match bits and force mbox update
		GetInfoTable()->SetCurrentMatch(state->GetMatchItem());
		GetInfoTable()->SetMatch(state->GetViewMode(), GetInfoTable()->GetCurrentMatch().ConstructSearch(NULL), false);
	}
	
	// Adjust sorting
	SetSortBy(state->GetSortBy());

	// Set sorting button
	mSortDirectionBtn->SetState(JBoolean(state->GetShowBy() == cShowMessageDescending));
	SetShowBy(state->GetShowBy());

	// Init the preview state once if we're in a window
	if (!Is3Pane() && !mPreviewInit)
	{
		mMessageView->ResetState();
		mPreviewInit = true;
	}

	// Init splitter pos
	if (!Is3Pane() && (state->GetSplitterSize() != 0))
		GetMailboxWindow()->GetSplitter()->SetRelativeSplitPos(state->GetSplitterSize());

	// If forced reset, save it
	if (force)
		SaveState();

	// Do zoom
	//if (state->GetState() == eWindowStateMax)
	//	GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	// Activate only if not already visible
	if (!Is3Pane() && !GetWindow()->IsVisible())
		GetMailboxWindow()->Activate();

} // CMailboxInfoView::SetState

// Save current state in prefs
void CMailboxInfoView::SaveState(void)
{
	CMbox* mbox = GetMbox();
	if (!mbox)
		return;

	// Can only do if mbox still open
	if (!mbox->IsOpen())
		return;

	// Get name as cstr
	cdstring name = mbox->GetAccountName();

	// Get bounds
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);
	bool zoomed = false;

	// Get unzoomed bounds
	JRect user_bounds(0, 0, 0, 0);
	if (!Is3Pane())
		user_bounds = bounds;

	// Get current match item
	CMatchItem match;
	if (!mbox->GetAutoViewMode())
		match = GetInfoTable()->GetCurrentMatch();
	NMbox::EViewMode mode = mbox->GetAutoViewMode() ? NMbox::eViewMode_All : mbox->GetViewMode();
	if (!std::accumulate(match.GetBitsSet().begin(), match.GetBitsSet().end(), false, std::logical_or<bool>()) && !match.GetSearchSet().size())
		mode = NMbox::eViewMode_All;

	// Sync column state
	SyncColumns();

	// Add info to prefs
	CMailboxWindowState* info = new CMailboxWindowState(name,
														&user_bounds,
														zoomed ? eWindowStateMax : eWindowStateNormal,
														&mColumnInfo,
														(ESortMessageBy) mbox->GetSortBy(),
														(EShowMessageBy) mbox->GetShowBy(),
														mode,
														&match,
														Is3Pane() ? 0 : GetMailboxWindow()->GetSplitter()->GetRelativeSplitPos());

	if (info)
		CPreferences::sPrefs->AddMailboxWindowInfo(info);
}

// Save current state as default
void CMailboxInfoView::SaveDefaultState(void)
{
	CMbox* mbox = GetMbox();
	if (!mbox)
		return;

	// Get bounds
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);
	bool zoomed = false;

	// Get unzoomed bounds
	JRect user_bounds(0, 0, 0, 0);
	if (!Is3Pane())
		user_bounds = bounds;

	// Get current match item
	CMatchItem match;
	if (!mbox->GetAutoViewMode())
		match = GetInfoTable()->GetCurrentMatch();

	// Sync column state
	SyncColumns();

	// Add info to prefs
	CMailboxWindowState state(NULL,
								&user_bounds,
								zoomed ? eWindowStateMax : eWindowStateNormal,
								&mColumnInfo,
								(ESortMessageBy) mbox->GetSortBy(),
								(EShowMessageBy) mbox->GetShowBy(),
								mbox->GetAutoViewMode() ? NMbox::eViewMode_All : mbox->GetViewMode(),
								&match,
								Is3Pane() ? 0 : GetMailboxWindow()->GetSplitter()->GetRelativeSplitPos());
	if (CPreferences::sPrefs->mMailboxWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mMailboxWindowDefault.SetDirty();

}
//	Respond to commands
bool CMailboxInfoView::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eWindowsShowQuickSearch:
		OnWindowsShowQuickSearch();
		return true;

	default:;
	};

	return CMailboxView::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CMailboxInfoView::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eWindowsShowQuickSearch:
	{
		// Always enabled but text may change
		OnUpdateWindowsShowQuickSearch(cmdui);
		return;
	}

	default:;
	}

	CMailboxView::UpdateCommand(cmd, cmdui);
}

