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


// CLetterTextEditView.cpp : implementation file
//


#include "CLetterTextEditView.h"

#include "CAddressBookManager.h"
#include "CAdminLock.h"
#include "CCaptionParser.h"
#include "CDrawUtils.h"
#include "CEnvelope.h"
#include "CFontCache.h"
#include "CLetterHeaderView.h"
#include "CLetterWindow.h"
#include "CMailControl.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSpellPlugin.h"
#include "CStringUtils.h"
#include "CURL.h"
	
/////////////////////////////////////////////////////////////////////////////
// CLetterTextEditView

const int cToolbarHeight = 30;

IMPLEMENT_DYNCREATE(CLetterTextEditView, CCmdEditView)

CLetterTextEditView::CLetterTextEditView()
{
	mLtrWindow = NULL;
	mAdjustedPrintMargins = false;
	SetMargins(CRect(1080, 1080, 1080, 1080));
	
	mFormatter = new CDisplayFormatter(this);
	
	mToolbarShowing = true;	// Initially visible, but will be hidden
}

CLetterTextEditView::~CLetterTextEditView()
{
	delete mFormatter;
}


BEGIN_MESSAGE_MAP(CLetterTextEditView, CCmdEditView)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateAlways)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateAlways)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)

	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_NOTIFY_REFLECT(EN_SELCHANGE, OnSelChange)

	ON_WM_DROPFILES()

	ON_UPDATE_COMMAND_UI(IDM_ADDR_CAPTURE, OnUpdateNeedSel)
	ON_COMMAND(IDM_ADDR_CAPTURE, OnCaptureAddress)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLetterTextEditView message handlers

CStyleToolbar* CLetterTextEditView::GetToolbar()
{
	return &mStyleToolbar;
}

void CLetterTextEditView::OnSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	ASSERT(pNMHDR->code == EN_SELCHANGE);
	mStyleToolbar.OnSelChange();
	
	CCmdEditView::OnSelChange(pNMHDR, pResult);
}

int CLetterTextEditView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCmdEditView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create header pane
	mStyleToolbar.CreateEx(0, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, 500, cToolbarHeight, GetParent()->GetSafeHwnd(), (HMENU)IDC_LETTERHEADER);
	mStyleToolbar.SetFormatter(mFormatter);
	HideToolbar();
	
	// Turn on spell checking if present (must do after creation so that IRichEditOLE is valid)
	if (CPluginManager::sPluginManager.HasSpelling())
		SpellAutoCheck(CPluginManager::sPluginManager.GetSpelling()->SpellAsYouType());
	
	AddDropFlavor(CMulberryApp::sFlavorMsgList);
	AddDropFlavor(CMulberryApp::sFlavorAtchList);
	AddDropFlavor(CMulberryApp::sFlavorMsgAtchList);

	// Only if not locked out
	if (!CAdminLock::sAdminLock.mNoAttachments)
		DragAcceptFiles();

	return 0;
}


// Allow background keystrokes
void CLetterTextEditView::OnSetFocus(CWnd* pOldWnd)
{
	// Do inherited
	CCmdEditView::OnSetFocus(pOldWnd);
	
	// Allow background keystrokes to filter through
	//CMailControl::AllowBusyKeys(true);				<- cannot do this because cmd-keys propagate through
}

// Disallow background keystrokes
void CLetterTextEditView::OnKillFocus(CWnd* pNewWnd )
{
	// Do inherited
	CCmdEditView::OnKillFocus(pNewWnd);
	
	// Allow background keystrokes to filter through
	//CMailControl::AllowBusyKeys(false);				<- see above
}

CDisplayFormatter *CLetterTextEditView::GetFormatter()
{
	return mFormatter;
}

void CLetterTextEditView::Reset(bool click, long scale)
{
	// Now reset the display formatter
	mFormatter->Reset(click, scale);
}

void CLetterTextEditView::SetToolbar(const CMIMEContent& mime)
{
	if(mime.GetContentType() == eContentText)
		SetToolbar(mime.GetContentSubtype());
}

void CLetterTextEditView::SetToolbar(EContentSubType type)
{
	switch(type)
	{
	case eContentSubEnriched:
	case eContentSubHTML:
		if (CAdminLock::sAdminLock.mAllowStyledComposition)
			ShowToolbar(type);
		else
			HideToolbar();
		break;
	case eContentSubPlain:		
	default:
		HideToolbar();
		break;
	}
}

void CLetterTextEditView::ShowToolbar(EContentSubType type, bool override)
{
	if (!mToolbarShowing || override)
	{
		// Turn on selection notification for toolbar
		long old_mask = GetRichEditCtrl().GetEventMask();
		GetRichEditCtrl().SetEventMask(old_mask | ENM_SELCHANGE);

		mToolbarShowing = true;
		
		// Show after redoing layout to avoid update errors
		mStyleToolbar.ShowWindow(SW_SHOW);
		mShowStyled = true;

		Rect r;
		GetParent()->GetClientRect(&r);
		r.top += cToolbarHeight;
		GetRichEditCtrl().MoveWindow(&r);
	}
	
	// Always reset
	if (type == eContentSubEnriched)
		mStyleToolbar.OnEnrichedToolbar();
	else
		mStyleToolbar.OnHTMLToolbar();
	mStyleToolbar.RedrawWindow();
}

void CLetterTextEditView::HideToolbar(bool override)
{
	if (mToolbarShowing || override)
	{
		// Turn off selection notification for toolbar
		//long old_mask = GetRichEditCtrl().GetEventMask();
		//GetRichEditCtrl().SetEventMask(old_mask & ~ENM_SELCHANGE);

		mStyleToolbar.ShowWindow(SW_HIDE);
		mStyleToolbar.OnPlainToolbar();
		mToolbarShowing = false;
		mShowStyled = false;

		Rect r;
		GetParent()->GetClientRect(&r);
		GetRichEditCtrl().MoveWindow(&r);
	}
}

#pragma mark ____________________________________Command Updaters

// Common updaters
void CLetterTextEditView::OnUpdateAlways(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);	// Always
}

BOOL CLetterTextEditView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}

void CLetterTextEditView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// reset main text font to printer font
	if (mLtrWindow->mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain)
	{
		SetCtrlRedraw(false);
		ResetFont(CFontCache::GetPrintFont(), true);
	}

	OnPrinterChanged(*pDC);
	mAdjustedPrintMargins = false;

	CCmdEditView::OnBeginPrinting(pDC, pInfo);
}

void CLetterTextEditView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CCmdEditView::OnEndPrinting(pDC, pInfo);

	// reset main text font to printer font
	if (mLtrWindow->mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain)
	{
		ResetFont(CFontCache::GetDisplayFont(), true);
		SetCtrlRedraw(true);
		RedrawWindow();
	}
}

void CLetterTextEditView::AdjustPrintMargins(CDC* pDC, CPrintInfo* pInfo, const CMessage* msg)
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
		btm_offset = area.bottom - (margins.bottom * printer_dpi_y) / 1000;;
		
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
		btm_offset = area.bottom - (margins.bottom * printer_dpi_y) / 1440;;
	}

	area.left = left_offset;
	area.right = right_offset;
	area.top = top_offset;

	// Do header

	// Create header text
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

void CLetterTextEditView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	// Get message/envelope
	CMessage* msg = mLtrWindow->CreateMessage(false);

	// Adjust margins first time through
	if (!mAdjustedPrintMargins)
		AdjustPrintMargins(pDC, pInfo, msg);

	// Check for summary headers
	mLtrWindow->AddPrintSummary(msg);

	// If empty clip do not draw header/footer
	CRect clip;
	if (pDC->GetClipBox(clip) != NULLREGION)
	{
		// offset by printing offset
		pDC->SetViewportOrg(-pDC->GetDeviceCaps(PHYSICALOFFSETX),
			-pDC->GetDeviceCaps(PHYSICALOFFSETY));

		// Determine screen and printer dpis
		CDC* sDC = GetDC();
		int screen_dpi = ::GetDeviceCaps(sDC->m_hDC, LOGPIXELSY);
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
		cdstring headTxt = CCaptionParser::ParseCaption(CPreferences::sPrefs->mLtrHeaderCaption.GetValue(), msg, false, pInfo->m_nCurPage);
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

		cdstring footTxt = CCaptionParser::ParseCaption(CPreferences::sPrefs->mLtrFooterCaption.GetValue(), msg, false, pInfo->m_nCurPage);
		int footer_height = pDC->DrawText(footTxt.win_str(), -1, area, DT_CALCRECT | DT_LEFT | DT_WORDBREAK);

		pDC->DrawText(footTxt.win_str(), -1, area, DT_LEFT | DT_WORDBREAK);
		if (CPreferences::sPrefs->mFooterBox.GetValue())
			pDC->FrameRect(area, &CDrawUtils::sBlackBrush);

		// Switch DC to header/footet font
		pDC->SelectObject(old_font);
	}

	// Print page
	CCmdEditView::OnPrint(pDC, pInfo);

	// Check for summary headers
	mLtrWindow->RemovePrintSummary(msg);

	delete msg;
}

#pragma mark ____________________________________Commands

void CLetterTextEditView::OnEditPaste()
{
	if (!::OpenClipboard(*this))
		return;
	HANDLE txt = ::GetClipboardData(CF_UNICODETEXT);
	cdustring old_txt;
	if (txt && CPreferences::sPrefs->mSmartURLPaste.GetValue())
	{
		unichar_t* lptstr = (unichar_t*) ::GlobalLock(txt);
		cdustring utf16(lptstr);
		cdstring utf8 = utf16.ToUTF8();

		const char* p = utf8;
		unsigned long hlen = utf8.length();
		unsigned long plen = hlen;

		// Look for URL scheme at start
		unsigned long scheme_len = 0;
		for(cdstrvect::const_iterator iter = CPreferences::sPrefs->mRecognizeURLs.GetValue().begin();
			(scheme_len == 0) && (iter != CPreferences::sPrefs->mRecognizeURLs.GetValue().end()); iter++)
		{
			size_t iter_len = (*iter).length();

			// Look for URL scheme prefix of URL:scheme prefix
			if ((plen > iter_len) &&
				::strncmpnocase(p, (*iter).c_str(), iter_len) == 0)
				scheme_len = iter_len;
			else if ((plen > iter_len + cURLMainSchemeLength) &&
						(::strncmpnocase(p, cURLMainScheme, cURLMainSchemeLength) == 0) &&
					 	(::strncmpnocase(p + cURLMainSchemeLength, (*iter).c_str(), iter_len) == 0))
				scheme_len = cURLMainSchemeLength + iter_len;
		}

		// Check whether a scheme was found
		if (scheme_len != 0)
		{
			// Look for all text contain valid URL characters
			plen -= scheme_len;
			p += scheme_len;
			while(plen-- && (scheme_len != 0))
			{
				// Look for valid URL character
				if (cURLXCharacter[*(unsigned char*)p++] == 0)
					// Set scheme_len to zero to indicate failure
					scheme_len = 0;
			}
			
			// Now create a new string with delimiters
			if (scheme_len != 0)
			{
				// Create new delimited URL
				cdstring new_url = "<";
				new_url += utf8;
				new_url += ">";
				
				utf16 = cdustring(new_url);

				// Create new handle from URL and save to scrap
				HANDLE new_txt = ::GlobalAlloc(GMEM_DDESHARE, (utf16.length() + 1) * sizeof(unichar_t));
				if (new_txt)
				{
					// Store the old one so we can restore the clipboard once the paste is complete
					old_txt = lptstr;
					
					// Now add new text to clipboard
					unichar_t* nlptstr = (unichar_t*) ::GlobalLock(new_txt);
					::unistrcpy(nlptstr, utf16);
					::GlobalUnlock(new_txt);
					::EmptyClipboard();
					::SetClipboardData(CF_UNICODETEXT, new_txt);
				}
			}
		}

		::GlobalUnlock(txt);
	}
	::CloseClipboard();

	// Now do default action
	CCmdEditView::OnEditPaste();
	
	// Restore old text if if was changed
	if (!old_txt.empty())
	{
		// Create new handle from URL and save to scrap
		HANDLE restore_txt = ::GlobalAlloc(GMEM_DDESHARE, (old_txt.length() + 1) * sizeof(unichar_t));
		if (restore_txt)
		{
			// Now add new text to clipboard
			unichar_t* rlptstr = (unichar_t*) ::GlobalLock(restore_txt);
			::unistrcpy(rlptstr, old_txt);
			::GlobalUnlock(restore_txt);
			if (::OpenClipboard(*this))
			{
				::EmptyClipboard();
				::SetClipboardData(CF_UNICODETEXT, restore_txt);
				::CloseClipboard();
			}
		}
	}
}

// Capture selected text as an address
void CLetterTextEditView::OnCaptureAddress()
{
	// Get Selection
	cdstring selection;
	GetSelectedText(selection);
	
	// Do capture if address capability available
	if (CAddressBookManager::sAddressBookManager)
		CAddressBookManager::sAddressBookManager->CaptureAddress(selection);
}

#pragma mark ____________________________________Drag and drop

void CLetterTextEditView::OnDropFiles(HDROP hDropInfo)
{
	mLtrWindow->mPartsTable.OnDropFiles(hDropInfo);
	mLtrWindow->mPartsTable.ExposePartsList();
}

HRESULT CLetterTextEditView::QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR* lpcfFormat, DWORD dwReco, BOOL bReally, HGLOBAL hMetaFile)
{
	COleDataObject dataobj;
	dataobj.Attach(lpdataobj, FALSE);

	// Check for mDropFlavors
	if (!bReally || !(::GetKeyState(VK_MENU) < 0))
	{
		for(int i = 0; i < mDropFlavors.GetSize(); i++)
		{
			unsigned int theFlavor = mDropFlavors[i];
		
			if (dataobj.IsDataAvailable(theFlavor))
			{
				// Handle the actual drop
				if (bReally)
				{
					OnDrop(theFlavor, &dataobj);
					return S_FALSE;
				}
				else
					return S_OK;
			}
		}
	}
	
	return CCmdEditView::QueryAcceptData(lpdataobj, lpcfFormat, dwReco, bReally, hMetaFile);
}

BOOL CLetterTextEditView::OnDrop(unsigned int theFlavor, COleDataObject* pDataObject)
{
	bool done = false;
	bool free_hglb = false;
	HGLOBAL hglb = pDataObject->GetGlobalData(theFlavor);
	
	if (!hglb)
	{
		STGMEDIUM stg;
		if (pDataObject->GetData(theFlavor, &stg))
		{
			switch(stg.tymed)
			{
			case TYMED_HGLOBAL:
				hglb = ::GlobalAlloc(GMEM_DDESHARE, ::GlobalSize(stg.hGlobal));
				if (hglb)
				{
					unsigned char* dest = static_cast<unsigned char*>(::GlobalLock(hglb));
					unsigned char* src = static_cast<unsigned char*>(::GlobalLock(stg.hGlobal));
					::memcpy(dest, src, ::GlobalSize(stg.hGlobal));
					::GlobalUnlock(stg.hGlobal);
					::GlobalUnlock(hglb);
					free_hglb = true;
				}
				break;
			case TYMED_ISTREAM:
				// Count bytes in stream
				unsigned long ctr = 0;
				char buf[1024];
				unsigned long actual = 0;
				while(true)
				{
					stg.pstm->Read(buf, 1024, &actual);
					ctr += actual;
					if (actual != 1024)
						break;
				}
				_LARGE_INTEGER lint = {0, 0};
				stg.pstm->Seek(lint, STREAM_SEEK_SET, NULL);
				hglb = ::GlobalAlloc(GMEM_DDESHARE, ctr);
				if (hglb)
				{
					unsigned char* dest = static_cast<unsigned char*>(::GlobalLock(hglb));
					while(true)
					{
						stg.pstm->Read(dest, 1024, &actual);
						dest += actual;
						if (actual != 1024)
							break;
					}
					::GlobalUnlock(hglb);
					free_hglb = true;
				}
			}
			::ReleaseStgMedium(&stg);
		}
	}

	if (hglb)
	{
		char* lptstr = (char*) ::GlobalLock(hglb);
		DWORD gsize = ::GlobalSize(hglb);			// This value is meaningless as it might be bigger than actual data
		try
		{
			done = mLtrWindow->mPartsTable.DropData(theFlavor, lptstr, gsize);
			mLtrWindow->mPartsTable.ExposePartsList();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

		}
		
		::GlobalUnlock(hglb);
		if (free_hglb)
			::GlobalFree(hglb);
	}

	return done;
}
