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


// CSpacebarEditView.cpp : implementation file
//


#include "CSpacebarEditView.h"

#include "CCaptionParser.h"
#include "CDrawUtils.h"
#include "CEnvelope.h"
#include "CFontCache.h"
#include "CMessage.h"
#include "CMessageView.h"
#include "CMessageWindow.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CRFC822.h"
	
/////////////////////////////////////////////////////////////////////////////
// CSpacebarEditView

IMPLEMENT_DYNCREATE(CSpacebarEditView, CCmdEditView)

CSpacebarEditView::CSpacebarEditView()
{
	mMsgWindow = NULL;
	mMsgView = NULL;
	mAdjustedPrintMargins = false;
	SetMargins(CRect(1080, 722, 1080, 842));
}

CSpacebarEditView::~CSpacebarEditView()
{
}


BEGIN_MESSAGE_MAP(CSpacebarEditView, CCmdEditView)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateAlways)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateAlways)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpacebarEditView message handlers

bool CSpacebarEditView::HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	switch(nChar)
	{
	// Scroll down then to next message
	case VK_SPACE:
		{
			// Look for maximum pos of scroll bar
			SCROLLINFO info;
			bool has_scroll = (GetRichEditCtrl().GetStyle() & WS_VSCROLL) && GetRichEditCtrl().GetScrollInfo(SB_VERT, &info);
			if (!has_scroll || (info.nPos + info.nPage >= info.nMax) || (info.nPage == 0))
			{
				// Control key => delete and read next
				if (::GetKeyState(VK_CONTROL) < 0)
				{
					if (mMsgWindow)
						mMsgWindow->OnMessageDeleteRead();
					else if (mMsgView)
						mMsgView->OnMessageDeleteRead();
				}
				else if (::GetKeyState(VK_SHIFT) < 0)
				{

					// Delete and go to next message
					if (mMsgWindow)
						mMsgWindow->OnMessageReadPrev();
					else if (mMsgView)
						mMsgView->OnMessageReadPrev();
				}
				else
				{
					if (mMsgWindow)
						mMsgWindow->OnMessageReadNext();
					else if (mMsgView)
						mMsgView->OnMessageReadNext();
				}
			}
			else
			{
				// Do page down
				GetRichEditCtrl().SendMessage(WM_VSCROLL, SB_PAGEDOWN, 0L);
			}
		}
		return true;

	// Select next new in preview
	case VK_TAB:
		// Only do in a preview pane
		if (mMsgView)
		{
			mMsgView->OnMessageReadNextNew();
			return true;
		}
		break;

	// Delete and next
	case VK_BACK:
	case VK_DELETE:
		if (mMsgWindow)
			mMsgWindow->OnMessageDeleteRead();
		else if (mMsgView)
			mMsgView->OnMessageDeleteRead();
		return true;

	default:;
	}

	return CCmdEditView::HandleChar(nChar, nRepCnt, nFlags);
}

#pragma mark ____________________________________Command Updaters

// Common updaters
void CSpacebarEditView::OnUpdateAlways(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);	// Always
}

#pragma mark ____________________________________Printing

BOOL CSpacebarEditView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}

void CSpacebarEditView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Reset main text font to printer font
	bool use_part;
	if (mMsgWindow)
		use_part = (mMsgWindow->GetCurrentPart() != NULL) && (mMsgWindow->GetViewAs() != eViewAsRaw);
	else if (mMsgView)
		use_part = (mMsgView->GetCurrentPart() != NULL) && (mMsgView->GetViewAs() != eViewAsRaw);

	bool change_font;
	if (mMsgWindow)
		change_font = use_part && (mMsgWindow->GetCurrentPart()->GetContent().GetContentSubtype() == eContentSubPlain);
	else if (mMsgView)
		change_font = use_part && (mMsgView->GetCurrentPart()->GetContent().GetContentSubtype() == eContentSubPlain);

	if (change_font)
	{
		SetCtrlRedraw(false);
		ResetFont(CFontCache::GetPrintFont(), true);
	}

	// Check for summary headers
	if (mMsgWindow)
		mMsgWindow->AddPrintSummary();
	else
		mMsgView->AddPrintSummary();

	OnPrinterChanged(*pDC);
	mAdjustedPrintMargins = false;

	CCmdEditView::OnBeginPrinting(pDC, pInfo);
}

void CSpacebarEditView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CCmdEditView::OnEndPrinting(pDC, pInfo);

	// Check for summary headers
	if (mMsgWindow)
		mMsgWindow->RemovePrintSummary();
	else
		mMsgView->RemovePrintSummary();

	// reset main text font to printer font
	bool use_part;
	if (mMsgWindow)
		use_part = (mMsgWindow->GetCurrentPart() != NULL) && (mMsgWindow->GetViewAs() != eViewAsRaw);
	else if (mMsgView)
		use_part = (mMsgView->GetCurrentPart() != NULL) && (mMsgView->GetViewAs() != eViewAsRaw);

	bool change_font;
	if (mMsgWindow)
		change_font = use_part && (mMsgWindow->GetCurrentPart()->GetContent().GetContentSubtype() == eContentSubPlain);
	else if (mMsgView)
		change_font = use_part && (mMsgView->GetCurrentPart()->GetContent().GetContentSubtype() == eContentSubPlain);

	if (change_font)
	{
		ResetFont(CFontCache::GetDisplayFont(), true);
		SetCtrlRedraw(true);
		RedrawWindow();
	}

	SetMargins(CRect(1080, 722, 1080, 842));
}

void CSpacebarEditView::AdjustPrintMargins(CDC* pDC, CPrintInfo* pInfo)
{
	// Determine screen and printer dpis
	CDC* sDC = GetDC();
	int screen_dpi = sDC->GetDeviceCaps(LOGPIXELSY);
	int printer_dpi_x = pDC->GetDeviceCaps(LOGPIXELSX);
	int printer_dpi_y = pDC->GetDeviceCaps(LOGPIXELSY);
	ReleaseDC(sDC);

	// Create scaled font from point size
	LOGFONT lf = CPreferences::sPrefs->mCaptionTextFontInfo.GetValue().logfont;
	lf.lfHeight = 5 * ((2*-72*lf.lfHeight/screen_dpi) + 1);	// Round up to nearest int
	CFont headfoot;
	headfoot.CreatePointFontIndirect(&lf, pDC);

	// Switch DC to header/footet font
	CFont* old_font = pDC->SelectObject(&headfoot);

	CRect area = GetPageRect();
	area.left = (area.left * printer_dpi_x) / 1440;
	area.right = (area.right * printer_dpi_x) / 1440;
	area.top = (area.top * printer_dpi_y) / 1440;
	area.bottom = (area.bottom * printer_dpi_y) / 1440;

	int top_offset = 0;
	int left_offset = 0;
	int right_offset = 0;
	int btm_offset = 0;;

	CRect margins;
	if (CMulberryApp::sApp->GetPageMargins(margins))
	{
		// Page setup margins in 1000th inch
		top_offset = (margins.top * printer_dpi_y) / 1000;
		left_offset = (margins.left * printer_dpi_x) / 1000;
		right_offset = area.right - (margins.right * printer_dpi_x) / 1000;
		btm_offset = area.bottom - (margins.bottom * printer_dpi_y) / 1000;
		
		// Adjust print margins to RichEdit margins
		margins.top = (margins.top * 1440) / 1000;
		margins.left = (margins.left * 1440) / 1000;
		margins.bottom = (margins.bottom * 1440) / 1000;
		margins.right = (margins.right * 1440) / 1000;
	}
	else
	{
		// RichEdit margins in 1440th inch
		margins = GetMargins();
		top_offset = (margins.top * printer_dpi_y) / 1440;
		left_offset = (margins.left * printer_dpi_x) / 1440;
		right_offset = area.right - (margins.right * printer_dpi_x) / 1440;
		btm_offset = area.bottom - (margins.bottom * printer_dpi_y) / 1440;
	}

	area.left = left_offset;
	area.right = right_offset;
	area.top = top_offset;

	// Do header

	// Create header text
	CMessage* msg = (mMsgWindow ? mMsgWindow->GetMessage() : (mMsgView ? mMsgView->GetMessage() : NULL));
	cdstring headTxt = CCaptionParser::ParseCaption(CPreferences::sPrefs->mHeaderCaption.GetValue(), msg, false, pInfo->m_nCurPage);
	int header_height = pDC->DrawText(headTxt.win_str(), -1, area, DT_CALCRECT | DT_LEFT | DT_WORDBREAK);

	// Do footer
	
	// Create footer text
	area = pInfo->m_rectDraw;
	area.left = left_offset;
	area.right = right_offset;

	cdstring footTxt = CCaptionParser::ParseCaption(CPreferences::sPrefs->mFooterCaption.GetValue(), msg, false, pInfo->m_nCurPage);
	int footer_height = pDC->DrawText(footTxt.win_str(), -1, area, DT_CALCRECT | DT_LEFT | DT_WORDBREAK);
	area.bottom = btm_offset;
	area.top = area.bottom - footer_height;

	// Switch DC to header/footer font
	pDC->SelectObject(old_font);
	
	// Adjust for size of captions
	CRect new_margins = margins;
	new_margins.top += (header_height * 1440) / printer_dpi_y + 144;
	new_margins.bottom += (footer_height * 1440) / printer_dpi_y + 144;
	SetMargins(new_margins);

	mAdjustedPrintMargins = true;
}

void CSpacebarEditView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	// Adjust margins first time through
	if (!mAdjustedPrintMargins)
		AdjustPrintMargins(pDC, pInfo);

	// If empty clip do not draw header/footer
	CRect clip;
	if (pDC->GetClipBox(clip) != NULLREGION)
	{
		// offset by printing offset
		pDC->SetViewportOrg(-pDC->GetDeviceCaps(PHYSICALOFFSETX),
			-pDC->GetDeviceCaps(PHYSICALOFFSETY));

		// Determine screen and printer dpis
		CDC* sDC = GetDC();
		int screen_dpi = sDC->GetDeviceCaps(LOGPIXELSY);
		int printer_dpi_x = pDC->GetDeviceCaps(LOGPIXELSX);
		int printer_dpi_y = pDC->GetDeviceCaps(LOGPIXELSY);
		ReleaseDC(sDC);

		// Create scaled font from point size
		LOGFONT lf = CPreferences::sPrefs->mCaptionTextFontInfo.GetValue().logfont;
		lf.lfHeight = 5 * ((2*-72*lf.lfHeight/screen_dpi) + 1);	// Round up to nearest int
		CFont headfoot;
		headfoot.CreatePointFontIndirect(&lf, pDC);

		// Switch DC to header/footet font
		CFont* old_font = pDC->SelectObject(&headfoot);

		CRect area = GetPageRect();
		area.left = (area.left * printer_dpi_x) / 1440;
		area.right = (area.right * printer_dpi_x) / 1440;
		area.top = (area.top * printer_dpi_y) / 1440;
		area.bottom = (area.bottom * printer_dpi_y) / 1440;

		CRect margins = GetMargins();
		int top_offset = ((margins.top - 144) * printer_dpi_y) / 1440;
		int left_offset = (margins.left * printer_dpi_x) / 1440;
		int right_offset = area.right - (margins.right * printer_dpi_x) / 1440;
		int btm_offset = area.bottom - ((margins.bottom - 144) * printer_dpi_y) / 1440;

		area.left = left_offset;
		area.right = right_offset;
		area.top = top_offset;

		// Do header

		// Create header text
		CMessage* msg = (mMsgWindow ? mMsgWindow->GetMessage() : (mMsgView ? mMsgView->GetMessage() : NULL));
		cdstring headTxt = CCaptionParser::ParseCaption(CPreferences::sPrefs->mHeaderCaption.GetValue(), msg, false, pInfo->m_nCurPage);
		int header_height = pDC->DrawText(headTxt.win_str(), -1, area, DT_CALCRECT | DT_LEFT | DT_WORDBREAK);
		area.OffsetRect(0, -header_height);
		pDC->DrawText(headTxt.win_str(), -1, area, DT_LEFT | DT_WORDBREAK);
		if (CPreferences::sPrefs->mHeaderBox.GetValue())
			pDC->FrameRect(area, &CDrawUtils::sBlackBrush);

		// Do footer
		
		// Create footer text
		area = pInfo->m_rectDraw;
		area.left = left_offset;
		area.right = right_offset;
		area.top = btm_offset;

		cdstring footTxt = CCaptionParser::ParseCaption(CPreferences::sPrefs->mFooterCaption.GetValue(), msg, false, pInfo->m_nCurPage);
		int footer_height = pDC->DrawText(footTxt.win_str(), -1, area, DT_CALCRECT | DT_LEFT | DT_WORDBREAK);

		pDC->DrawText(footTxt.win_str(), -1, area, DT_LEFT | DT_WORDBREAK);
		if (CPreferences::sPrefs->mFooterBox.GetValue())
			pDC->FrameRect(area, &CDrawUtils::sBlackBrush);

		// Switch DC to header/footer font
		pDC->SelectObject(old_font);
	}

	// Print page
	CCmdEditView::OnPrint(pDC, pInfo);
}
