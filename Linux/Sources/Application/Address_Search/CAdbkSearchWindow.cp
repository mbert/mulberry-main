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


// Source for CAdbkSearchWindow class

#include "CAdbkSearchWindow.h"

#include "CAdbkSearchTitleTable.h"
#include "CAdbkServerPopup.h"
#include "CContextMenu.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulberryWMClass.h"
#include "CPreferences.h"
#include "CTableScrollbarSet.h"
#include "CTextField.h"
#include "CTitleTable.h"
#include "CWindowsMenu.h"

#include "TPopupMenu.h"
#include "HResourceMap.h"

#include "JXMultiImageButton.h"
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>
#include "HButtonText.h"

#include <cassert>

// Static 

// Static members

CAdbkSearchWindow*	CAdbkSearchWindow::sAdbkSearch = nil;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkSearchWindow::CAdbkSearchWindow(JXDirector* owner)
	: CTableWindow(owner)
{
	sAdbkSearch = this;

	// Add to list
	CWindowsMenu::AddWindow(this, false);
}

// Default destructor
CAdbkSearchWindow::~CAdbkSearchWindow()
{
	// Set status
	SetClosing();

	sAdbkSearch = nil;

	// Remove from list
	CWindowsMenu::RemoveWindow(this);
	
	// Save state
	SaveDefaultState();

	// Set status
	SetClosed();
}

// Manually create document

CAdbkSearchWindow* CAdbkSearchWindow::ManualCreate(void)
{
	CAdbkSearchWindow * pWnd = new CAdbkSearchWindow(CMulberryApp::sApp);
	if (pWnd)
	{
		pWnd->OnCreate();
		pWnd->ResetState();
		pWnd->Activate();
	}

	return pWnd;
}

// Destroy it
void CAdbkSearchWindow::DestroyWindow()
{
	// Update from window
	if (sAdbkSearch)
		FRAMEWORK_DELETE_WINDOW(sAdbkSearch);
	sAdbkSearch = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cTitleHeight = 16;

// Do various bits
void CAdbkSearchWindow::OnCreate()
{

	SetWindowInfo(eAddrColumnName, 80);

// begin JXLayout

    JXWindow* window = new JXWindow(this, 420,270, "");
    assert( window != NULL );
    SetWindow(window);

    mHeader =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,30, 420,105);
    assert( mHeader != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Find Items", mHeader,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,5, 65,20);
    assert( obj1 != NULL );

    mSourcePopup =
        new HPopupMenu("",mHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 75,2, 155,20);
    assert( mSourcePopup != NULL );

    mFieldTitle1 =
        new JXStaticText("whose", mHeader,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,5, 56,20);
    assert( mFieldTitle1 != NULL );

    mServersTitle =
        new JXStaticText("Server", mHeader,
                    JXWidget::kHElastic, JXWidget::kVElastic, 240,5, 56,20);
    assert( mServersTitle != NULL );

    mServerPopup =
        new CAdbkServerPopup("",mHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 295,7, 30,16);
    assert( mServerPopup != NULL );

    mFieldTitle2 =
        new JXStaticText("whose", mHeader,
                    JXWidget::kHElastic, JXWidget::kVElastic, 330,4, 56,20);
    assert( mFieldTitle2 != NULL );

    mFieldPopup =
        new HPopupMenu("",mHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,30, 130,20);
    assert( mFieldPopup != NULL );

    mMethodPopup =
        new HPopupMenu("",mHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 145,30, 130,20);
    assert( mMethodPopup != NULL );

    mSearchText =
        new CTextInputField(mHeader,
                    JXWidget::kHElastic, JXWidget::kVElastic, 282,31, 130,20);
    assert( mSearchText != NULL );

    mSearchBtn =
        new HButtonText<JXMultiImageButton>(mHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,54, 32,32);
    assert( mSearchBtn != NULL );

    mClearBtn =
        new HButtonText<JXMultiImageButton>(mHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 52,54, 32,32);
    assert( mClearBtn != NULL );

    mNewMessageBtn =
        new HButtonText<JXMultiImageButton>(mHeader,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 380,54, 32,32);
    assert( mNewMessageBtn != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,135, 420,135);
    assert( sbs != NULL );

// end JXLayout

	window->SetTitle("Address Search");

	// Init items
	mServersTitle->Hide();
	mServerPopup->Hide();
	mFieldTitle2->Hide();

	mSourcePopup->SetMenuItems("in Address Books %r | using LDAP %r");
	mServerPopup->SyncMenu();
	mFieldPopup->SetMenuItems("Full Name %r | Nick-name %r | Email Address %r | Company %r | "
							  "Address %r | Phone Work %r | Phone Home %r | Fax %r | URLs %r | Notes %r");
	mMethodPopup->SetMenuItems("Is %r | Starts with %r | Ends with %r | Contains %r");

	cdstring s;
	s.FromResource(IDS_SEARCH);
	mSearchBtn->SetText(s);
	mSearchBtn->SetImage(IDI_ADDRESS_SEARCH);
	mSearchBtn->SetShortcuts("^M");

	s.FromResource(IDS_CLEAR);
	mClearBtn->SetText(s);
	mClearBtn->SetImage(IDI_ADDRESS_DELETE);

	s.FromResource(IDS_ADDRESS_NEWMSG);
	mNewMessageBtn->SetText(s);
	mNewMessageBtn->SetImage(IDI_ADDRESS_NEWMSG);

	//Create table and it's titles inside scrollbar set
	mTable = new CAdbkSearchTable(sbs,sbs->GetScrollEnclosure(),
														JXWidget::kHElastic,
														JXWidget::kVElastic,
														0,cTitleHeight, 105, 100);
	mTitles = new CAdbkSearchTitleTable(sbs, sbs->GetScrollEnclosure(),
																	JXWidget::kHElastic,
																	JXWidget::kFixedTop,
																	0, 0, 105, cTitleHeight);
	GetTable()->OnCreate();
	GetTitles()->OnCreate();

	PostCreate(mTable, mTitles);

	// Do inherited
	CTableWindow::OnCreate();

	CreateMainMenu(window, CMainMenu::fFile | 
								 CMainMenu::fEdit | 
								 CMainMenu::fAddresses | 
								 CMainMenu::fCalendar | 
								 CMainMenu::fWindows | 
								 CMainMenu::fHelp);

	// Tooltips
	mSourcePopup->SetHint(stringFromResource(IDC_SEARCH_SOURCEPOPUP));
	mServerPopup->SetHint(stringFromResource(IDC_SEARCH_SERVERPOPUP));
	mFieldPopup->SetHint(stringFromResource(IDC_SEARCH_FIELDPOPUP));
	mMethodPopup->SetHint(stringFromResource(IDC_SEARCH_METHODPOPUP));
	mSearchText->SetHint(stringFromResource(IDC_SEARCHTEXT));
	mSearchBtn->SetHint(stringFromResource(IDC_SEARCHBTN));
	mClearBtn->SetHint(stringFromResource(IDC_SEARCHCLEARBTN));
	mNewMessageBtn->SetHint(stringFromResource(IDC_SEARCHNEWMSGBTN));

	// Set current width and height as minimum
	GetWindow()->SetMinSize(GetWindow()->GetBoundsGlobal().width(),
							GetWindow()->GetBoundsGlobal().height());
	GetWindow()->SetWMClass(cAdbkSearchWMClass, cMulberryWMClass);
	GetWindow()->SetIcon(iconFromResource(IDR_ADDRESSSEARCHDOCSDI, GetWindow(), 32, 0x00CCCCCC));

	// Set status
	SetOpen();
	
	ListenTo(mSourcePopup);
	ListenTo(mSearchBtn);
	ListenTo(mClearBtn);
	ListenTo(mNewMessageBtn);

} // CAdbkSearchWindow::FinishCreateSelf

// Respond to clicks in the icon buttons
void CAdbkSearchWindow::Receive(JBroadcaster* sender, const Message& message)
{
	//Button Pushed
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mSearchBtn)
		{
			GetTable()->OnSearch();
			return;
		}
		else if (sender == mClearBtn)
		{
			GetTable()->OnClear();
			return;
		}
		else if (sender == mNewMessageBtn)
		{
			GetTable()->OnNewLetter();
			return;
		}
	}	
	else if (message.Is(JXMenu::kItemSelected))
	{
		JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
    	if (sender == mSourcePopup)
    	{
			OnChangeSource(index);
			return;
		}
	}

	CTableWindow::Receive(sender, message);
}

// Activate search item
void CAdbkSearchWindow::Activate()
{
	CTableWindow::Activate();

	mSearchText->Focus();
	mSearchText->SelectAll();
}

void CAdbkSearchWindow::OnChangeSource(JIndex nID)
{
	switch(nID)
	{
	case menu_AddrSearchMethodMulberry:
		// Enable all field items
		mServerPopup->EnableAll();
		break;
	case menu_AddrSearchMethodLDAP:
		mServerPopup->InitAccountMenu(CINETAccount::eLDAP);
		mServerPopup->SyncMenu();
		// Enable all field items
		mServerPopup->EnableAll();
		break;
	}

	switch(nID)
	{
	case menu_AddrSearchMethodMulberry:
		mFieldTitle1->Show();
		mServersTitle->Hide();
		mServerPopup->Hide();
		mFieldTitle2->Hide();
		break;
	case menu_AddrSearchMethodLDAP:
		mFieldTitle1->Hide();
		mServersTitle->Show();
		mServerPopup->Show();
		mFieldTitle2->Show();
		break;
	}
}

// Init columns and text
void CAdbkSearchWindow::InitColumns(void)
{
	// Remove any previous columns
	mColumnInfo.clear();
	
	// Flags, width = 240
	InsertColumn(1);
	SetColumnType(1, eAddrColumnName);
	SetColumnWidth(1, 240);
}

// Reset state from prefs
void CAdbkSearchWindow::ResetState(bool force)
{
	// Get default state
	CAdbkSearchWindowState* state = &CPreferences::sPrefs->mAdbkSearchWindowDefault.Value();

	// Get visible state
	bool visible = GetWindow()->IsVisible();

	// Do not set if empty
	JRect set_rect = state->GetBestRect(CPreferences::sPrefs->mAdbkSearchWindowDefault.GetValue());
	if (!set_rect.IsEmpty())
	{
		// Clip to screen
		::RectOnScreen(set_rect);

		GetWindow()->Place(set_rect.left, set_rect.top);
		GetWindow()->SetSize(set_rect.width(), set_rect.height());
	}

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mAdbkSearchWindowDefault.GetValue()));
	
	// Always show - the only time reset state is called is when the user asks for the search window

	// Show/hide window based on state
	//if (state->mHide)
	//	Hide();
	//else
	//	Show();
	
	// Set state of popups
	mSourcePopup->SetValue(state->GetMethod());
	OnChangeSource(state->GetMethod());
	mFieldPopup->SetValue(state->GetField());
	mMethodPopup->SetValue(state->GetCriteria());

	// If forced reset, save it
	if (force)
		SaveDefaultState();

	// Do zoom
	//if (state->GetState() == eWindowStateMax)
	//	GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);
}

// Save current state in prefs
void CAdbkSearchWindow::SaveDefaultState(void)
{
	// Get bounds
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);
	bool zoomed = false;

	// Sync column widths
	for(unsigned long i = 0; i < mColumnInfo.size(); i++)
		mColumnInfo[i].column_width = GetTable()->GetColWidth(i + 1);

	// Add info to prefs
	CAdbkSearchWindowState state(NULL, &bounds,
									zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo,
									(EAdbkSearchMethod) mSourcePopup->GetValue(),
									(EAddrColumn) mFieldPopup->GetValue(),
									(EAdbkSearchCriteria) mMethodPopup->GetValue(),
									!GetWindow()->IsVisible());
	if (CPreferences::sPrefs->mAdbkSearchWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mAdbkSearchWindowDefault.SetDirty();

} // CAdbkSearchWindow::SaveDefaultState
