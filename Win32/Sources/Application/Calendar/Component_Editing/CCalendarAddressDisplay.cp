/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CCalendarAddressDisplay.cpp : implementation file
//


#include "CCalendarAddressDisplay.h"

#include "CAddressBookManager.h"
#include "CLog.h"
#include "CPreferences.h"
#include "CStringUtils.h"
#include "CTextListChoice.h"

#include "StValueChanger.h"

#include <strstream>

/////////////////////////////////////////////////////////////////////////////
// CCalendarAddressDisplay
#pragma mark ____________________________________CCalendarAddressDisplay


bool CCalendarAddressDisplay::sResolve = true;

CCalendarAddressDisplay::CCalendarAddressDisplay()
{
	mResolving = false;
}

CCalendarAddressDisplay::~CCalendarAddressDisplay()
{
}


BEGIN_MESSAGE_MAP(CCalendarAddressDisplay, CCmdEdit)
	//{{AFX_MSG_MAP(CCalendarAddressDisplay)
	ON_WM_CREATE()
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCalendarAddressDisplay message handlers

int CCalendarAddressDisplay::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCmdEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
		
	// register drop target
	SetOLECallback(&m_xRichEditOleCallback);
	
	return 0;
}

void CCalendarAddressDisplay::OnKillFocus(CWnd* pNewWnd)
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
		// Do not allow this to interrupt the focus change
		CLOG_LOGCATCH(...);
	}

	CCmdEdit::OnKillFocus(pNewWnd);
}

CCalendarAddressList* CCalendarAddressDisplay::GetAddresses()
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
			// Do not allow this to interrupt the focus change
			CLOG_LOGCATCH(...);
		}
	}

	// Now get text data
	cdstring txt;
	GetText(txt);
	CCalendarAddressList* list = new CCalendarAddressList;
	cdstrvect tokens;
	txt.split("\r\n", tokens);
	for(cdstrvect::const_iterator iter = tokens.begin(); iter != tokens.end(); iter++)
	{
		list->push_back(new CCalendarAddress(*iter));
	}
	return list;
}

void CCalendarAddressDisplay::ResolveAddressList(CWnd* pNewWnd)
{
	// Resolve addresses
	cdstring str;
	GetText(str);
	short text_length = str.length();
	if (text_length && sResolve)
	{
		// See if resolution or expand required
		bool resolution = !CPreferences::sPrefs->mExpandNoNicknames.GetValue();
		bool expand = !CPreferences::sPrefs->mExpandFailedNicknames.GetValue();
		
		// Tokenize
		cdstrvect tokens;
		str.split("\n", tokens);
		cdstrvect results;
		for(cdstrvect::const_iterator iter = tokens.begin(); iter != tokens.end(); iter++)
		{
			ResolveExpandAddress(*iter, results, resolution, expand);
		}

		cdstring result;
		result.join(results, "\r\n");

		// Only change if different
		bool twist = (results.size() > 1);
		if (str != result)
			SetText(result);
		else
			// Turn off twist change if text is the same as before
			twist = false;
	}
}

void CCalendarAddressDisplay::ResolveExpandAddress(const cdstring& text, cdstrvect& results, bool resolve, bool expand)
{
	// Check for valid address first
	CCalendarAddress addr(text);
	if (addr.IsValid())
	{
		results.push_back(text);
		return;
	}

	// See whether we need to resolve
	if (resolve)
	{
		CAddressList list;
		if (CAddressBookManager::sAddressBookManager->ResolveNickName(text, &list) ||
			CAddressBookManager::sAddressBookManager->ResolveGroupName(text, &list))
		{
			list.AddCalendarAddressToList(results, true);
			return;
		}
	}

	if (expand)
	{
		ExpandAddressText(text, results);
		return;
	}
}

// Expand address
bool CCalendarAddressDisplay::ExpandAddressText(const cdstring& expand, cdstrvect& results)
{
	// Do expansion
	cdstrvect addrs;
	CAddressBookManager::sAddressBookManager->ExpandCalendarAddresses(expand, addrs);
	
	// Check for multiple
	cdstring temp;
	if (addrs.size() == 1)
		results.push_back(addrs.front());
	else if (addrs.size() > 1)
	{
		// Display list to user
		ulvector selection;
		if (CTextListChoice::PoseDialog("Alerts::Letter::AddressDisplay_Title", "Alerts::Letter::AddressDisplay_Desc", NULL, false, false, false, true,
											addrs, temp, selection, NULL))
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

void CCalendarAddressDisplay::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
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
	}
	else
		CCmdEdit::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CCalendarAddressDisplay::IsRichEditFormat(CLIPFORMAT cf)
{
	return ((cf == _oleData.cfRichTextFormat) ||
		(cf == _oleData.cfRichTextAndObjects) || (cf == CF_TEXT));
}

HRESULT CCalendarAddressDisplay::QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR* lpcfFormat, DWORD dwReco, BOOL bReally, HGLOBAL hMetaFile)
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

BOOL CCalendarAddressDisplay::OnDrop(COleDataObject* pDataObject)
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
			std::ostrstream out;
			const unsigned long buff_size = 1024;
			char buff[buff_size];
			unsigned long read_size = 0;
			do
			{
				stgm.pstm->Read(buff, buff_size, &read_size);
				out.write(buff, read_size);
			}
			while(read_size == buff_size);
			out << std::ends << std::ends;
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
// CCalendarAddressDisplay::XRichEditOleCallback

BEGIN_INTERFACE_MAP(CCalendarAddressDisplay, CCmdEdit)
	// we use IID_IUnknown because richedit doesn't define an IID
	INTERFACE_PART(CCalendarAddressDisplay, IID_IUnknown, RichEditOleCallback)
END_INTERFACE_MAP()

STDMETHODIMP_(ULONG) CCalendarAddressDisplay::XRichEditOleCallback::AddRef()
{
	METHOD_PROLOGUE_EX_(CCalendarAddressDisplay, RichEditOleCallback)
	return (ULONG)pThis->InternalAddRef();
}

STDMETHODIMP_(ULONG) CCalendarAddressDisplay::XRichEditOleCallback::Release()
{
	METHOD_PROLOGUE_EX_(CCalendarAddressDisplay, RichEditOleCallback)
	return (ULONG)pThis->InternalRelease();
}

STDMETHODIMP CCalendarAddressDisplay::XRichEditOleCallback::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(CCalendarAddressDisplay, RichEditOleCallback)
	return (HRESULT)pThis->InternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP CCalendarAddressDisplay::XRichEditOleCallback::GetNewStorage(LPSTORAGE* ppstg)
{
	METHOD_PROLOGUE_EX_(CCalendarAddressDisplay, RichEditOleCallback)

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

STDMETHODIMP CCalendarAddressDisplay::XRichEditOleCallback::GetInPlaceContext(
	LPOLEINPLACEFRAME* lplpFrame, LPOLEINPLACEUIWINDOW* lplpDoc,
	LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	METHOD_PROLOGUE_EX(CCalendarAddressDisplay, RichEditOleCallback)
	return E_FAIL;
}

STDMETHODIMP CCalendarAddressDisplay::XRichEditOleCallback::ShowContainerUI(BOOL fShow)
{
	METHOD_PROLOGUE_EX(CCalendarAddressDisplay, RichEditOleCallback)
	return E_FAIL;
}

STDMETHODIMP CCalendarAddressDisplay::XRichEditOleCallback::QueryInsertObject(
	LPCLSID /*lpclsid*/, LPSTORAGE /*pstg*/, LONG /*cp*/)
{
	METHOD_PROLOGUE_EX(CCalendarAddressDisplay, RichEditOleCallback)
	//pThis->GetDocument()->InvalidateObjectCache();
	return S_OK;
}

STDMETHODIMP CCalendarAddressDisplay::XRichEditOleCallback::DeleteObject(LPOLEOBJECT /*lpoleobj*/)
{
	METHOD_PROLOGUE_EX_(CCalendarAddressDisplay, RichEditOleCallback)
	//pThis->GetDocument()->InvalidateObjectCache();
	return S_OK;
}

STDMETHODIMP CCalendarAddressDisplay::XRichEditOleCallback::QueryAcceptData(
	LPDATAOBJECT lpdataobj, CLIPFORMAT* lpcfFormat, DWORD reco,
	BOOL fReally, HGLOBAL hMetaPict)
{
	METHOD_PROLOGUE_EX(CCalendarAddressDisplay, RichEditOleCallback)
	return pThis->QueryAcceptData(lpdataobj, lpcfFormat, reco,
		fReally, hMetaPict);
}

STDMETHODIMP CCalendarAddressDisplay::XRichEditOleCallback::ContextSensitiveHelp(BOOL /*fEnterMode*/)
{
	return E_NOTIMPL;
}

STDMETHODIMP CCalendarAddressDisplay::XRichEditOleCallback::GetClipboardData(
	CHARRANGE* lpchrg, DWORD reco, LPDATAOBJECT* lplpdataobj)
{
	METHOD_PROLOGUE_EX(CCalendarAddressDisplay, RichEditOleCallback)
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

STDMETHODIMP CCalendarAddressDisplay::XRichEditOleCallback::GetDragDropEffect(
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

STDMETHODIMP CCalendarAddressDisplay::XRichEditOleCallback::GetContextMenu(
	WORD seltype, LPOLEOBJECT lpoleobj, CHARRANGE* lpchrg,
	HMENU* lphmenu)
{
	METHOD_PROLOGUE_EX(CCalendarAddressDisplay, RichEditOleCallback)
	return E_NOTIMPL;
}

