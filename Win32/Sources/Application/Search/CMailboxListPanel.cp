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


// CMailboxListPanel.cp : implementation of the CMailboxListPanel class
//

#include "CMailboxListPanel.h"

#include "CBrowseMailboxDialog.h"
#include "CIconTextTable.h"
#include "CLog.h"
#include "CMailAccountManager.h"
#include "CMailCheckThread.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMboxList.h"
#include "CMboxRef.h"
#include "CMboxProtocol.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CStatusWindow.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

const int cCaptionHeight = 16;

const int cTwisterWidth = 16;
const int cTwisterHeight = 16;
const int cTwisterHOffset = 8;
const int cTwisterVOffset = 0;

const int cTwisterCaptionWidth = 160;
const int cTwisterCaptionHOffset = cTwisterHOffset + 24;
const int cTwisterCaptionVOffset = cTwisterVOffset + 2;

const int cMailboxBorderHOffset = cTwisterHOffset;
const int cMailboxBorderVOffset = cTwisterVOffset + cTwisterHeight + 4;
const int cMailboxBorderWidth = 2 * cMailboxBorderHOffset;
const int cMailboxBorderHeight = 2;

const int cMailboxAreaVOffset = cMailboxBorderVOffset + 4;

const int cCabinetsWidth = 24;
const int cCabinetsHeight = 16;
const int cCabinetsHOffset = 100;
const int cCabinetsVOffset = 10;
const int cCabinetsTitleWidth = 70;
const int cCabinetsTitleHOffset = cCabinetsHOffset - cCabinetsTitleWidth;
const int cCabinetsTitleVOffset = cCabinetsVOffset + 2;

const int cAddListBtnWidth = 50;
const int cAddListBtnHeight = 24;
const int cAddListBtnHOffset = cCabinetsHOffset + cCabinetsWidth + 10;
const int cAddListBtnVOffset = cCabinetsVOffset - 4;

const int cClearListBtnWidth = 50;
const int cClearListBtnHeight = 24;
const int cClearListBtnHOffset = cAddListBtnHOffset + cAddListBtnWidth + 8;
const int cClearListBtnVOffset = cCabinetsVOffset - 4;

const int cMailboxListHOffset = 8;
const int cMailboxListVOffset = cCabinetsVOffset + 22;
const int cMailboxListWidth = cMailboxBorderWidth;

const int cProgressAreaHeight = cCaptionHeight;

const int cTotalTitleWidth = 30;
const int cTotalTitleHOffset = cMailboxListHOffset;
const int cTotalTitleVOffset = 2;

const int cTotalWidth = 32;
const int cTotalHOffset = cTotalTitleHOffset + cTotalTitleWidth;
const int cTotalVOffset = cTotalTitleVOffset;

const int cFoundTitleWidth = 36;
const int cFoundTitleHOffset = cTotalHOffset + cTotalWidth + 8;
const int cFoundTitleVOffset = cTotalTitleVOffset;

const int cFoundWidth = 32;
const int cFoundHOffset = cFoundTitleHOffset + cFoundTitleWidth;
const int cFoundVOffset = cTotalTitleVOffset;

const int cMessagesTitleWidth = 54;
const int cMessagesTitleHOffset = cFoundHOffset + cFoundWidth + 8;
const int cMessagesTitleVOffset = cTotalTitleVOffset;

const int cMessagesWidth = 32;
const int cMessagesHOffset = cMessagesTitleHOffset + cMessagesTitleWidth;
const int cMessagesVOffset = cTotalTitleVOffset;

const int cProgressHOffset = cMessagesHOffset + cMessagesWidth + 2;
const int cProgressVOffset = cTotalTitleVOffset;
const int cProgressWidth = cMailboxListWidth;
const int cProgressHeight = cCaptionHeight;

/////////////////////////////////////////////////////////////////////////////
// CMailboxListPanel

BEGIN_MESSAGE_MAP(CMailboxListPanel, CGrayBackground)
	ON_COMMAND(IDC_SEARCH_TWIST, OnTwist)
	ON_COMMAND_RANGE(IDM_SEARCH_CABINETS_First, IDM_SEARCH_CABINETS_End, OnCabinet)
	ON_COMMAND(IDC_SEARCH_ADDLIST, OnAddMailboxList)
	ON_COMMAND(IDC_SEARCH_CLEARLIST, OnClearMailboxList)
	ON_COMMAND(IDC_SEARCH_MAILBOXLIST, OnOpenMailboxList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMailboxListPanel construction/destruction

CMailboxListPanel::CMailboxListPanel()
{
	mTwisted = true;
	mInProgress = false;
	mParentFrame = NULL;
	mMoveParent1 = NULL;
	mMoveParent2 = NULL;
}

CMailboxListPanel::~CMailboxListPanel()
{
}

void CMailboxListPanel::CreateSelf(CWnd* parent_frame, CWnd* move_parent1, CWnd* move_parent2, int width, int height)
{
	mParentFrame = parent_frame;
	mMoveParent1 = move_parent1;
	mMoveParent2 = move_parent2;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	//const int cPopupHeight1 = cPopupHeight + small_offset;
	//const int cPopupWidthExtra = 4*small_offset;

	CRect r = CRect(cTwisterHOffset, cTwisterVOffset, cTwisterHOffset + cTwisterWidth, cTwisterVOffset + cTwisterHeight);
	mTwister.Create(r, this, IDC_SEARCH_TWIST);
	mTwister.SetPushed(true);
	AddAlignment(new CWndAlignment(&mTwister, CWndAlignment::eAlign_TopLeft));

	CString s;
	s.LoadString(IDS_SEARCH_SHOWCAPTION);
	r = CRect(cTwisterCaptionHOffset, cTwisterCaptionVOffset, cTwisterCaptionHOffset + cTwisterCaptionWidth + large_offset, cTwisterCaptionVOffset + cCaptionHeight);
	mShowCaption.Create(s, WS_CHILD | WS_VISIBLE, r, this, IDC_STATIC);
	mShowCaption.SetFont(CMulberryApp::sAppFontBold);
	AddAlignment(new CWndAlignment(&mShowCaption, CWndAlignment::eAlign_TopLeft));

	s.LoadString(IDS_SEARCH_HIDECAPTION);
	r = CRect(cTwisterCaptionHOffset, cTwisterCaptionVOffset, cTwisterCaptionHOffset + cTwisterCaptionWidth + large_offset, cTwisterCaptionVOffset + cCaptionHeight);
	mHideCaption.Create(s, WS_CHILD | WS_VISIBLE, r, this, IDC_STATIC);
	mHideCaption.SetFont(CMulberryApp::sAppFontBold);
	AddAlignment(new CWndAlignment(&mHideCaption, CWndAlignment::eAlign_TopLeft));

	//s.LoadString(IDS_SEARCH_MAILBOXLIST);
	r = CRect(cMailboxBorderHOffset, cMailboxBorderVOffset, cMailboxBorderHOffset + width - cMailboxBorderWidth, cMailboxBorderVOffset + cMailboxBorderHeight);
	mMailboxBorder.Create(_T(""), WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, r, this, IDC_STATIC);
	mMailboxBorder.SetFont(CMulberryApp::sAppFont);
	AddAlignment(new CWndAlignment(&mMailboxBorder, CWndAlignment::eAlign_TopWidth));

	int mailboxAreaBottom = height - cProgressHeight - large_offset - 6;
	r = CRect(0, cMailboxAreaVOffset, width, mailboxAreaBottom);
	mMailboxArea.Create(_T(""), WS_CHILD | WS_VISIBLE, r, this, IDC_STATIC);
	AddAlignment(new CWndAlignment(&mMailboxArea, CWndAlignment::eAlign_TopWidth));

	s.LoadString(IDS_SEARCH_CABINETSTITLE);
	r = CRect(cCabinetsTitleHOffset - 2*large_offset, cCabinetsTitleVOffset - small_offset, cCabinetsTitleHOffset + cCabinetsTitleWidth + 2*large_offset, cCabinetsTitleVOffset + cCaptionHeight);
	mCabinetsTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mMailboxArea, IDC_STATIC);
	mCabinetsTitle.SetFont(CMulberryApp::sAppFont);
	mMailboxArea.AddAlignment(new CWndAlignment(&mCabinetsTitle, CWndAlignment::eAlign_TopLeft));

	r = CRect(cCabinetsHOffset, cCabinetsVOffset, cCabinetsHOffset + cCabinetsWidth, cCabinetsVOffset + cCabinetsHeight);
	mCabinets.Create(_T(""), r, &mMailboxArea, IDC_SEARCH_CABINETPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mCabinets.SetMenu(IDR_POPUP_SEARCH_CABINETS);
	mCabinets.SetButtonText(false);
	mCabinets.SetFont(CMulberryApp::sAppFont);
	mMailboxArea.AddAlignment(new CWndAlignment(&mCabinets, CWndAlignment::eAlign_TopLeft));

	s.LoadString(IDS_SEARCH_ADDLISTBTN);
	r = CRect(cAddListBtnHOffset, cAddListBtnVOffset, cAddListBtnHOffset + cAddListBtnWidth, cAddListBtnVOffset + cAddListBtnHeight);
	mAddListBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mMailboxArea, IDC_SEARCH_ADDLIST);
	mAddListBtn.SetFont(CMulberryApp::sAppFont);
	mMailboxArea.AddAlignment(new CWndAlignment(&mAddListBtn, CWndAlignment::eAlign_TopLeft));

	s.LoadString(IDS_SEARCH_CLEARLISTBTN);
	r = CRect(cClearListBtnHOffset, cClearListBtnVOffset, cClearListBtnHOffset + cClearListBtnWidth, cClearListBtnVOffset + cClearListBtnHeight);
	mClearListBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, r, &mMailboxArea, IDC_SEARCH_CLEARLIST);
	mClearListBtn.SetFont(CMulberryApp::sAppFont);
	mMailboxArea.AddAlignment(new CWndAlignment(&mClearListBtn, CWndAlignment::eAlign_TopLeft));

	r = CRect(cMailboxListHOffset, cMailboxListVOffset + small_offset, width - cMailboxListWidth, mailboxAreaBottom - cMailboxListVOffset);
	mMailboxList.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, r, &mMailboxArea, IDC_STATIC);
	mMailboxList.SetDoubleClickMsg(IDC_SEARCH_MAILBOXLIST);
	mMailboxList.SetMsgTarget(this);
	mMailboxList.SetFont(CMulberryApp::sAppFont);
	mMailboxArea.AddAlignment(new CWndAlignment(&mMailboxList, CWndAlignment::eAlign_WidthHeight));

	r = CRect(0, mailboxAreaBottom, width, mailboxAreaBottom + cProgressHeight + large_offset);
	mProgressArea.Create(_T(""), WS_CHILD | WS_VISIBLE, r, this, IDC_STATIC);
	AddAlignment(new CWndAlignment(&mProgressArea, CWndAlignment::eAlign_BottomWidth));

	s.LoadString(IDS_SEARCH_TOTALTITLE);
	r = CRect(cTotalTitleHOffset, cTotalTitleVOffset, cTotalTitleHOffset + cTotalTitleWidth + large_offset, cTotalTitleVOffset + cCaptionHeight);
	mTotalTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mProgressArea, IDC_STATIC);
	mTotalTitle.SetFont(CMulberryApp::sAppFont);
	mProgressArea.AddAlignment(new CWndAlignment(&mTotalTitle, CWndAlignment::eAlign_TopLeft));

	r = CRect(cTotalHOffset + large_offset, cTotalVOffset, cTotalHOffset + cTotalWidth + 2*large_offset, cTotalVOffset + cCaptionHeight);
	mTotal.Create(_T(""), WS_CHILD | WS_VISIBLE, r, &mProgressArea, IDC_STATIC);
	mTotal.SetFont(CMulberryApp::sAppFont);
	mProgressArea.AddAlignment(new CWndAlignment(&mTotal, CWndAlignment::eAlign_TopLeft));

	s.LoadString(IDS_SEARCH_FOUNDTITLE);
	r = CRect(cFoundTitleHOffset + 2*large_offset, cFoundTitleVOffset, cFoundTitleHOffset + cFoundTitleWidth + 3*large_offset, cFoundTitleVOffset + cCaptionHeight);
	mFoundTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mProgressArea, IDC_STATIC);
	mFoundTitle.SetFont(CMulberryApp::sAppFont);
	mProgressArea.AddAlignment(new CWndAlignment(&mFoundTitle, CWndAlignment::eAlign_TopLeft));

	r = CRect(cFoundHOffset + 3*large_offset, cFoundVOffset, cFoundHOffset + cFoundWidth + 4*large_offset, cFoundVOffset + cCaptionHeight);
	mFound.Create(_T(""), WS_CHILD | WS_VISIBLE, r, &mProgressArea, IDC_STATIC);
	mFound.SetFont(CMulberryApp::sAppFont);
	mProgressArea.AddAlignment(new CWndAlignment(&mFound, CWndAlignment::eAlign_TopLeft));

	s.LoadString(IDS_SEARCH_MESSAGESTITLE);
	r = CRect(cMessagesTitleHOffset + 4*large_offset, cMessagesTitleVOffset, cMessagesTitleHOffset + cMessagesTitleWidth + 6*large_offset, cMessagesTitleVOffset + cCaptionHeight);
	mMessagesTitle.Create(s, WS_CHILD | WS_VISIBLE, r, &mProgressArea, IDC_STATIC);
	mMessagesTitle.SetFont(CMulberryApp::sAppFont);
	mProgressArea.AddAlignment(new CWndAlignment(&mMessagesTitle, CWndAlignment::eAlign_TopLeft));

	r = CRect(cMessagesHOffset + 6*large_offset, cMessagesVOffset, cMessagesHOffset + cMessagesWidth + 7*large_offset, cMessagesVOffset + cCaptionHeight);
	mMessages.Create(_T(""), WS_CHILD | WS_VISIBLE, r, &mProgressArea, IDC_STATIC);
	mMessages.SetFont(CMulberryApp::sAppFont);
	mProgressArea.AddAlignment(new CWndAlignment(&mMessages, CWndAlignment::eAlign_TopLeft));

	r = CRect(cProgressHOffset + 7*large_offset, cProgressVOffset, width - cProgressWidth, cProgressVOffset + cProgressHeight - 2);
	mProgress.Create(PBS_SMOOTH | WS_CHILD, r, &mProgressArea, IDC_STATIC);
	mProgressArea.AddAlignment(new CWndAlignment(&mProgress, CWndAlignment::eAlign_TopWidth));
}

void CMailboxListPanel::SetProgress(unsigned long progress)
{
	mProgress.SetCount(progress);
}

void CMailboxListPanel::SetFound(unsigned long found)
{
	cdstring num_txt = found;
	CUnicodeUtils::SetWindowTextUTF8(&mFound, num_txt);
}

void CMailboxListPanel::SetMessages(unsigned long msgs)
{
	cdstring num_txt = msgs;
	CUnicodeUtils::SetWindowTextUTF8(&mMessages, num_txt);
}

void CMailboxListPanel::SetHitState(unsigned long item, bool hit, bool clear)
{
	mMailboxList.SetIcon(item + 1, clear ? 0 : (hit ? IDI_SEARCH_HIT : IDI_SEARCH_MISS));
}

void CMailboxListPanel::SetInProgress(bool in_progress)
{
	mInProgress = in_progress;

	mCabinets.EnableWindow(!mInProgress);
	mAddListBtn.EnableWindow(!mInProgress);
	mClearListBtn.EnableWindow(!mInProgress);
}

#pragma mark ____________________________Commands

void CMailboxListPanel::OnTwist()
{
	if (mTwisted)
	{
		mHideCaption.ShowWindow(SW_HIDE);
		mShowCaption.ShowWindow(SW_SHOW);
		mMailboxArea.ShowWindow(SW_HIDE);
		CRect size;
		mMailboxArea.GetWindowRect(size);
		::ResizeWindowBy(this, 0, -size.Height(), false);

		if (mMoveParent1)
			::ResizeWindowBy(mMoveParent1, 0, size.Height(), mParentFrame == NULL);
		if (mMoveParent2)
		{
			::ResizeWindowBy(mMoveParent2, 0, -size.Height(), mParentFrame == NULL);
			::MoveWindowBy(mMoveParent2, 0, size.Height(), false);
		}
		if (mParentFrame)
			::ResizeWindowBy(mParentFrame, 0, -size.Height());
	}
	else
	{
		mShowCaption.ShowWindow(SW_HIDE);
		mHideCaption.ShowWindow(SW_SHOW);
		CRect size;
		mMailboxArea.GetWindowRect(size);
		::ResizeWindowBy(this, 0, size.Height(), false);
		mMailboxArea.ShowWindow(SW_SHOW);

		if (mMoveParent2)
		{
			::ResizeWindowBy(mMoveParent2, 0, size.Height(), mParentFrame == NULL);
			::MoveWindowBy(mMoveParent2, 0, -size.Height(), false);
		}
		if (mParentFrame)
			::ResizeWindowBy(mParentFrame, 0, size.Height());
		if (mMoveParent1)
			::ResizeWindowBy(mMoveParent1, 0, -size.Height(), mParentFrame == NULL);
	}
	
	mTwisted = !mTwisted;
	mTwister.SetPushed(mTwisted);
}

#pragma mark ____________________________Mailbox List

void CMailboxListPanel::InitCabinets()
{
	// Remove any existing items from main menu
	short num_menu = mCabinets.GetPopupMenu()->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		mCabinets.GetPopupMenu()->RemoveMenu(0, MF_BYPOSITION);
	
	short menu_id = IDM_SEARCH_CABINETS_First;
	for(CFavourites::const_iterator iter = CMailAccountManager::sMailAccountManager->GetFavourites().begin();
			iter != CMailAccountManager::sMailAccountManager->GetFavourites().end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(mCabinets.GetPopupMenu(), MF_STRING, menu_id++, (*iter)->GetName());
}

void CMailboxListPanel::OnCabinet(UINT nID)
{
	// Get cabinet
	const CMboxRefList* cabinet = CMailAccountManager::sMailAccountManager->GetFavourites().at(nID - IDM_SEARCH_CABINETS_First);

	{
		// Begin a busy operation - force it to cancel background threads
		CBusyContext busy;
		busy.SetCancelOthers(true);
		cdstring desc = rsrc::GetString("Status::IMAP::Checking");
		StMailBusy busy_lock(&busy, &desc);

		// Must wait for and pause mail checking
		while(CMailCheckThread::IsRunning())
		{
			if (CMailControl::ProcessBusy(&busy))
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}

	}

	bool was_paused = CMailCheckThread::Pause(true);

	CMboxList match;
	for(CMboxRefList::const_iterator iter = cabinet->begin(); iter != cabinet->end(); iter++)
	{
		const CMboxRef* mbox_ref = static_cast<CMboxRef*>(*iter);

		// Don't bother with directories
		if (mbox_ref->IsDirectory())
			continue;

		// Resolve to mboxes - maybe wildcard so get list but do not get directories
		if (mbox_ref->IsWildcard())
			mbox_ref->ResolveMbox(match, true, true);
		else
		{
			CMbox* mbox = mbox_ref->ResolveMbox(true);
			if (mbox && !mbox->IsDirectory())
				match.push_back(mbox);
		}
	}

	// Add all to list (will do refresh)
	AddMboxList(&match);
	
	// Can continue mail checking
	CMailCheckThread::Pause(was_paused);
}

void CMailboxListPanel::ResetMailboxList(const CMboxRefList& targets, const ulvector& target_hits)
{
	cdstrvect items;
	ulvector item_states;

	// Do to each mailbox
	CMboxRefList::const_iterator iter1 = targets.begin();
	ulvector::const_iterator iter2 = target_hits.begin();
	for(; iter1 != targets.end() && iter2 != target_hits.end(); iter1++, iter2++)
	{
		items.push_back((*iter1)->GetName());
		switch(*iter2)
		{
		case eNotHit:
		default:
			item_states.push_back(0);
			break;
		case eHit:
			item_states.push_back(IDI_SEARCH_HIT);
			break;
		case eMiss:
			item_states.push_back(IDI_SEARCH_MISS);
			break;
		}
	}
	
	mMailboxList.SetContents(items, item_states);
	
	cdstring num_str;
	num_str = static_cast<unsigned long>(items.size());
	CUnicodeUtils::SetWindowTextUTF8(&mTotal, num_str);
	num_str = 0L;
	CUnicodeUtils::SetWindowTextUTF8(&mFound, num_str);
	CUnicodeUtils::SetWindowTextUTF8(&mMessages, num_str);
}

void CMailboxListPanel::OnClearMailboxList()
{
	CMboxRefList targets;
	ulvector target_hits;
	ResetMailboxList(targets, target_hits);
}

void CMailboxListPanel::OnAddMailboxList()
{
	// Use browse dialog with multi-selection of mailboxes
	CMboxList found;
	if (CBrowseMailboxDialog::PoseDialog(found))
		AddMboxList(&found);
}

// Add a list of mailboxes
void CMailboxListPanel::AddMboxList(const CMboxList* list)
{
}

void CMailboxListPanel::OnOpenMailboxList()
{
	// This implementation does nothing
}
