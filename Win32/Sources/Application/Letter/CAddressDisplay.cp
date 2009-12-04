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


// CAddressDisplay.cpp : implementation file
//


#include "CAddressDisplay.h"

#include "CAddressBookManager.h"
#include "CAddressList.h"
#include "CLetterWindow.h"
#include "CLog.h"
#include "CPreferences.h"
#include "CStringUtils.h"
#include "CTextListChoice.h"
#include "CTwister.h"

#include "StValueChanger.h"

#include <strstream>

/////////////////////////////////////////////////////////////////////////////
// CAddressDisplay
#pragma mark ____________________________________CAddressDisplay


bool CAddressDisplay::sResolve = true;

CAddressDisplay::CAddressDisplay()
{
	mResolving = false;
	mTwister = NULL;
	mMover = NULL;
	mWindow = NULL;
}

CAddressDisplay::~CAddressDisplay()
{
}


BEGIN_MESSAGE_MAP(CAddressDisplay, CCmdEdit)
	//{{AFX_MSG_MAP(CAddressDisplay)
	ON_WM_CREATE()
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
	ON_COMMAND(IDM_ADDR_EXPAND, OnExpandAddress)
	ON_COMMAND(IDM_ADDR_CAPTURE, OnCaptureAddress)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddressDisplay message handlers

int CAddressDisplay::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCmdEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
		
	// register drop target
	SetOLECallback(&m_xRichEditOleCallback);
	
	return 0;
}

void CAddressDisplay::OnKillFocus(CWnd* pNewWnd)
{
	// Only do if parent active and not already resolving
	if (mResolving || (pNewWnd == NULL) ||
		((GetParentFrame() != pNewWnd->GetParentFrame()) ||
		 (GetParentOwner() != pNewWnd->GetParentOwner())))
	{
		CCmdEdit::OnKillFocus(pNewWnd);
		return;
	}

	try
	{
		StValueChanger<bool> _change(mResolving, true);
		ResolveAddressList(pNewWnd);
	}
	catch(...)
	{
		// Do not allow this to interrupt the fosuc change
		CLOG_LOGCATCH(...);
	}

	CCmdEdit::OnKillFocus(pNewWnd);
}

CAddressList* CAddressDisplay::GetAddresses()
{
	// Always resolve addresses if still active
	if (!mResolving && (GetFocus() == this))
	{
		try
		{
			StValueChanger<bool> _change(mResolving, true);
			ResolveAddressList(NULL);
		}
		catch(...)
		{
			// Do not allow this to interrupt the fosuc change
			CLOG_LOGCATCH(...);
		}
	}

	// Now get text data
	cdstring txt;
	GetText(txt);
	return new CAddressList(txt, txt.length());
}

void CAddressDisplay::ResolveAddressList(CWnd* pNewWnd)
{
	// Resolve addresses
	cdstring str;
	GetText(str);
	short text_length = str.length();
	if (text_length && sResolve)
	{
		// See if resolution or expand required
		bool resolution = !CPreferences::sPrefs->mExpandNoNicknames.GetValue();
		bool qualify = !CPreferences::sPrefs->mExpandFailedNicknames.GetValue();
		
		// Generate an address list
		auto_ptr<CAddressList> list(new CAddressList(str, text_length, 0, resolution));
		
		if (!resolution || !qualify)
		{
			// Look at each address
			for(CAddressList::iterator iter = list->begin(); iter != list->end(); iter++)
			{
				// See if expand required
				if ((*iter)->GetHost().empty() && !(*iter)->GetMailbox().empty())
				{
					// Get current name (which was the text in the address field).
					// Make sure any quotes are removed so the search will work properly.
					cdstring expand = (*iter)->GetMailbox();
					expand.unquote();

					// Expand address usng search into set of results
					cdstrvect results;
					if (ExpandAddressText(expand, results))
					{
						// Replace existing with the first one in the list
						**iter = CAddress(results.front());
						results.erase(results.begin());

						// Insert the rest
						if (results.size())
						{
							// Use array index as iterators are invalidated by insert
							unsigned long pos = iter - list->begin();
							for(cdstrvect::const_iterator iter2 = results.begin(); iter2 != results.end(); iter2++)
								list->insert(++pos + list->begin(), new CAddress(*iter2));
							iter = list->begin() + pos;
						}
					}
				}
			}
			
			// Must reset focus back to window being moved to
			{
				StValueChanger<bool> _change(sResolve, false);
				if (pNewWnd)
					pNewWnd->SetFocus();
				else
					SetFocus();
			}
		}

		// Qualify remainder
		list->QualifyAddresses(CPreferences::sPrefs->mMailDomain.GetValue());
		bool twist = (list->size() > 1);

		{
			ostrstream new_txt;
			list->WriteToStream(new_txt);
			new_txt << ends;
			cdstring total;
			total.steal(new_txt.str());

			// Only change if different
			GetText(str);
			if (str != total)
				SetText(total);
			else
				// Turn off twist change if text is the same as before
				twist = false;
		}

		// If not already twisted (i.e. switching out of address field) force toggle
		if (twist && mTwister && !mTwister->IsPushed())
		{
			// Must save restore focus
			CWnd* old_focus = GetFocus();

			StValueChanger<bool> _change(sResolve, false);
			mWindow->DoTwist(mMover, this, mTwister);
			
			if (old_focus)
				old_focus->SetFocus();
		}
	}
}

void CAddressDisplay::OnExpandAddress()
{
	// Must not resolve during this process
	StValueChanger<bool> change(sResolve, false);

	cdstring expand;

	// Look for selection
	long selStart;
	long selEnd;
	GetSel(selStart, selEnd);
	if (selStart == selEnd)
	{
		cdstring str;
		GetRawText(str);
		expand.assign(str, selStart);
		const char* p1 = ::strrchr(expand.c_str(), ',');
		const char* p2 = ::strrchr(expand.c_str(), '\r');
		if (p1 || p2)
		{
			const char* p = (p1 > p2) ? p1 : p2;
			p++;
			while(*p == ' ') p++;
			SetSel(p - expand.c_str(), selStart);
		}
		else
			SetSel(0, selStart);
	}

	// Only bother of something present
	GetSel(selStart, selEnd);
	unsigned long length = selEnd - selStart;
	if (!length)
		return;

	// Now copy selection
	GetSelectedText(expand);
	expand.trimspace();
	
	// Now insert new item
	cdstrvect results;
	if (ExpandAddressText(expand, results))
	{
		cdstring replace;
		for(cdstrvect::const_iterator iter = results.begin(); iter != results.end(); iter++)
		{
			if (iter != results.begin())
				replace += os_endl;
			replace += *iter;
		}
		InsertUTF8(replace);
	}
		
	// Reset focus back to field
	SetFocus();
}

// Expand address
bool CAddressDisplay::ExpandAddressText(cdstring& expand, cdstrvect& results)
{
	// Do expansion
	cdstrvect addrs;
	CAddressBookManager::sAddressBookManager->ExpandAddresses(expand, addrs);
	
	// Check for multiple
	expand = cdstring::null_str;
	if (addrs.size() == 1)
		results.push_back(addrs.front());
	else if (addrs.size() > 1)
	{
		// Display list to user
		ulvector selection;
		if (CTextListChoice::PoseDialog("Alerts::Letter::AddressDisplay_Title", "Alerts::Letter::AddressDisplay_Desc", NULL, false, false, false, true,
											addrs, expand, selection, NULL))
		{
			for(ulvector::const_iterator iter = selection.begin(); iter != selection.end(); iter++)
				results.push_back(addrs.at(*iter));
		}
	}
	else
		::MessageBeep(-1);
	
	// Now return result
	return results.size();
}

// Capture selected text as an address
void CAddressDisplay::OnCaptureAddress()
{
	// Get Selection
	cdstring selection;
	GetSelectedText(selection);
	
	// If empty, use all text
	if (selection.empty())
	{
		GetText(selection);
	}

	// Do capture if address capability available
	if (CAddressBookManager::sAddressBookManager)
		CAddressBookManager::sAddressBookManager->CaptureAddress(selection);
}

void CAddressDisplay::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Do twist and force resolution
	if (!mResolving && (nChar == VK_RETURN))
	{
		// Force resolution
		try
		{
			StValueChanger<bool> _change(mResolving, true);
			ResolveAddressList(NULL);
		}
		catch(...)
		{
			// Do not allow this to interrupt the fosuc change
			CLOG_LOGCATCH(...);
		}

		SetSel(GetTextLengthEx(), GetTextLengthEx());
		InsertUTF8("\r");
		
		if (mTwister && !mTwister->IsPushed())
		{
			// Must save restore focus
			CWnd* old_focus = GetFocus();

			StValueChanger<bool> _change(sResolve, false);
			mWindow->DoTwist(mMover, this, mTwister);
			
			if (old_focus)
				old_focus->SetFocus();
			SetSel(GetTextLengthEx(), GetTextLengthEx());
		}
	}
	else
		CCmdEdit::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CAddressDisplay::IsRichEditFormat(CLIPFORMAT cf)
{
	return ((cf == _oleData.cfRichTextFormat) ||
		(cf == _oleData.cfRichTextAndObjects) || (cf == CF_TEXT));
}

HRESULT CAddressDisplay::QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR* lpcfFormat, DWORD dwReco, BOOL bReally, HGLOBAL hMetaFile)
{
	COleDataObject dataobj;
	dataobj.Attach(lpdataobj, FALSE);

	// Handle the actual drop
	if (bReally)
	{
		OnDrop(&dataobj);
		return S_FALSE;
	}

	if (!bReally) // not actually pasting
		return S_OK;
	// if direct pasting a particular native format allow it
	if (IsRichEditFormat(*lpcfFormat))
		return S_OK;

	// if format is 0, then force particular formats if available
	if (*lpcfFormat == 0)
	{
		if (dataobj.IsDataAvailable((CLIPFORMAT)_oleData.cfRichTextAndObjects)) // native avail, let richedit do as it wants
			return S_OK;
		else if (dataobj.IsDataAvailable((CLIPFORMAT)_oleData.cfRichTextFormat))
		{
			*lpcfFormat = (CLIPFORMAT)_oleData.cfRichTextFormat;
			return S_OK;
		}
		else if (dataobj.IsDataAvailable(CF_TEXT))
		{
			*lpcfFormat = CF_TEXT;
			return S_OK;
		}
	}
	// paste OLE formats

	//DoPaste(dataobj, *lpcfFormat, hMetaFile);
	//return S_FALSE;
	return S_OK;
}

BOOL CAddressDisplay::OnDrop(COleDataObject* pDataObject)
{
	cdstring txt;
	STGMEDIUM stgm;
	bool done = false;
	HGLOBAL hglb = NULL;
	if ((hglb = pDataObject->GetGlobalData(CF_UNICODETEXT)) != NULL)
	{
		unichar_t* lptstr = (unichar_t*) ::GlobalLock(hglb);
		DWORD gsize = ::GlobalSize(hglb);			// This value is meaningless as it might be bigger than actual data
		cdustring utf16(lptstr, gsize / sizeof(unichar_t));
		txt = utf16.ToUTF8();

		::GlobalUnlock(hglb);
	}
	else if (pDataObject->GetData(CF_UNICODETEXT, &stgm))
	{
		switch (stgm.tymed)
		{
		case TYMED_HGLOBAL:
			if (!stgm.pUnkForRelease)
				::GlobalFree(stgm.hGlobal);
			break;
		case TYMED_ISTREAM:
		{
			// Read all data into buffer
			ostrstream out;
			const unsigned long buff_size = 1024;
			char buff[buff_size];
			unsigned long read_size = 0;
			do
			{
				stgm.pstm->Read(buff, buff_size, &read_size);
				out.write(buff, read_size);
			}
			while(read_size == buff_size);
			out << ends << ends;
			cdustring utf16;
			utf16.steal((unichar_t*)out.str());
			txt = utf16.ToUTF8();

			if (!stgm.pUnkForRelease)
				stgm.pstm->Release();
			break;
		}
		default:
			break;
		}
	}

	// Insert any data readin
	if (!txt.empty())
	{
		long sel = GetTextLengthEx();
		if (sel)
		{
			SetSel(sel - 1, sel);
			cdstring selection;
			GetSelectedText(selection);
			if (*selection.c_str() != '\r')
			{
				cdstring temp = "\r";
				temp += txt;
				txt = temp;
			}
		}
		SetSel(sel, sel);
		InsertUTF8(txt);
	}

	return done;
}

/////////////////////////////////////////////////////////////////////////////
// CAddressDisplay::XRichEditOleCallback

BEGIN_INTERFACE_MAP(CAddressDisplay, CCmdEdit)
	// we use IID_IUnknown because richedit doesn't define an IID
	INTERFACE_PART(CAddressDisplay, IID_IUnknown, RichEditOleCallback)
END_INTERFACE_MAP()

STDMETHODIMP_(ULONG) CAddressDisplay::XRichEditOleCallback::AddRef()
{
	METHOD_PROLOGUE_EX_(CAddressDisplay, RichEditOleCallback)
	return (ULONG)pThis->InternalAddRef();
}

STDMETHODIMP_(ULONG) CAddressDisplay::XRichEditOleCallback::Release()
{
	METHOD_PROLOGUE_EX_(CAddressDisplay, RichEditOleCallback)
	return (ULONG)pThis->InternalRelease();
}

STDMETHODIMP CAddressDisplay::XRichEditOleCallback::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(CAddressDisplay, RichEditOleCallback)
	return (HRESULT)pThis->InternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CAddressDisplay::XRichEditOleCallback::GetNewStorage(LPSTORAGE* ppstg)
{
	METHOD_PROLOGUE_EX_(CAddressDisplay, RichEditOleCallback)

	// Create a flat storage and steal it from the client item
	// the client item is only used for creating the storage
	COleClientItem item;
	item.GetItemStorageFlat();
	*ppstg = item.m_lpStorage;
	HRESULT hRes = E_OUTOFMEMORY;
	if (item.m_lpStorage != NULL)
	{
		item.m_lpStorage = NULL;
		hRes = S_OK;
	}
	//pThis->GetDocument()->InvalidateObjectCache();
	return hRes;
}

STDMETHODIMP CAddressDisplay::XRichEditOleCallback::GetInPlaceContext(
	LPOLEINPLACEFRAME* lplpFrame, LPOLEINPLACEUIWINDOW* lplpDoc,
	LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	METHOD_PROLOGUE_EX(CAddressDisplay, RichEditOleCallback)
	return E_FAIL;
}

STDMETHODIMP CAddressDisplay::XRichEditOleCallback::ShowContainerUI(BOOL fShow)
{
	METHOD_PROLOGUE_EX(CAddressDisplay, RichEditOleCallback)
	return E_FAIL;
}

STDMETHODIMP CAddressDisplay::XRichEditOleCallback::QueryInsertObject(
	LPCLSID /*lpclsid*/, LPSTORAGE /*pstg*/, LONG /*cp*/)
{
	METHOD_PROLOGUE_EX(CAddressDisplay, RichEditOleCallback)
	//pThis->GetDocument()->InvalidateObjectCache();
	return S_OK;
}

STDMETHODIMP CAddressDisplay::XRichEditOleCallback::DeleteObject(LPOLEOBJECT /*lpoleobj*/)
{
	METHOD_PROLOGUE_EX_(CAddressDisplay, RichEditOleCallback)
	//pThis->GetDocument()->InvalidateObjectCache();
	return S_OK;
}

STDMETHODIMP CAddressDisplay::XRichEditOleCallback::QueryAcceptData(
	LPDATAOBJECT lpdataobj, CLIPFORMAT* lpcfFormat, DWORD reco,
	BOOL fReally, HGLOBAL hMetaPict)
{
	METHOD_PROLOGUE_EX(CAddressDisplay, RichEditOleCallback)
	return pThis->QueryAcceptData(lpdataobj, lpcfFormat, reco,
		fReally, hMetaPict);
}

STDMETHODIMP CAddressDisplay::XRichEditOleCallback::ContextSensitiveHelp(BOOL /*fEnterMode*/)
{
	return E_NOTIMPL;
}

STDMETHODIMP CAddressDisplay::XRichEditOleCallback::GetClipboardData(
	CHARRANGE* lpchrg, DWORD reco, LPDATAOBJECT* lplpdataobj)
{
	METHOD_PROLOGUE_EX(CAddressDisplay, RichEditOleCallback)
	LPDATAOBJECT lpOrigDataObject = NULL;

	// get richedit's data object
	if (FAILED(pThis->GetIRichEditOle()->GetClipboardData(lpchrg, reco,
		&lpOrigDataObject)))
	{
		return E_NOTIMPL;
	}

	// allow changes
	HRESULT hRes = E_NOTIMPL;

	// if changed then free original object
	if (SUCCEEDED(hRes))
	{
		if (lpOrigDataObject!=NULL)
			lpOrigDataObject->Release();
		return hRes;
	}
	else
	{
		// use richedit's data object
		*lplpdataobj = lpOrigDataObject;
		return S_OK;
	}
}

STDMETHODIMP CAddressDisplay::XRichEditOleCallback::GetDragDropEffect(
	BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
{
	if (!fDrag) // allowable dest effects
	{
		DWORD dwEffect;
		// check for force link
		if ((grfKeyState & (MK_CONTROL|MK_SHIFT)) == (MK_CONTROL|MK_SHIFT))
			dwEffect = DROPEFFECT_LINK;
		// check for force copy
		else if ((grfKeyState & MK_CONTROL) == MK_CONTROL)
			dwEffect = DROPEFFECT_COPY;
		// check for force move
		else if ((grfKeyState & MK_ALT) == MK_ALT)
			dwEffect = DROPEFFECT_MOVE;
		// default -- recommended action is move
		else
			dwEffect = DROPEFFECT_MOVE;
		if (dwEffect & *pdwEffect) // make sure allowed type
			*pdwEffect = dwEffect;
	}
	return S_OK;
}

STDMETHODIMP CAddressDisplay::XRichEditOleCallback::GetContextMenu(
	WORD seltype, LPOLEOBJECT lpoleobj, CHARRANGE* lpchrg,
	HMENU* lphmenu)
{
	METHOD_PROLOGUE_EX(CAddressDisplay, RichEditOleCallback)
	return E_NOTIMPL;
}

