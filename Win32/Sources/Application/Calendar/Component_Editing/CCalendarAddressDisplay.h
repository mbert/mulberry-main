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


// CCalendarAddressDisplay.h : header file
//

#ifndef __CCALENDARADDRESSDISPLAY__MULBERRY__
#define __CCALENDARADDRESSDISPLAY__MULBERRY__

#include "CCmdEdit.h"

#include "CCalendarAddress.h"

/////////////////////////////////////////////////////////////////////////////
// CAddressDisplay window

class CCalendarAddressDisplay : public CCmdEdit
{
// Construction
public:
	CCalendarAddressDisplay();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCalendarAddressDisplay)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCalendarAddressDisplay();

	virtual CCalendarAddressList*	GetAddresses();

	static void		AllowResolve(bool allow)
						{ sResolve = allow; }

	// Generated message map functions
protected:
	//{{AFX_MSG(CCalendarAddressDisplay)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

			void	ResolveAddressList(CWnd* pNewWnd);
			void	ResolveExpandAddress(const cdstring& text, cdstrvect& results, bool resolve, bool expand);
			bool	ExpandAddressText(const cdstring& expand, cdstrvect& results);
	virtual HRESULT QueryAcceptData(LPDATAOBJECT, CLIPFORMAT*, DWORD, BOOL, HGLOBAL);
			BOOL	OnDrop(COleDataObject* pDataObject);

private:
	bool				mResolving;
	static bool			sResolve;

	BOOL IsRichEditFormat(CLIPFORMAT cf);

	DECLARE_MESSAGE_MAP()

// Interface Map
public:
	BEGIN_INTERFACE_PART(RichEditOleCallback, IRichEditOleCallback)
		INIT_INTERFACE_PART(CRichEditView, RichEditOleCallback)
		STDMETHOD(GetNewStorage) (LPSTORAGE*);
		STDMETHOD(GetInPlaceContext) (LPOLEINPLACEFRAME*,
									  LPOLEINPLACEUIWINDOW*,
									  LPOLEINPLACEFRAMEINFO);
		STDMETHOD(ShowContainerUI) (BOOL);
		STDMETHOD(QueryInsertObject) (LPCLSID, LPSTORAGE, LONG);
		STDMETHOD(DeleteObject) (LPOLEOBJECT);
		STDMETHOD(QueryAcceptData) (LPDATAOBJECT, CLIPFORMAT*, DWORD,BOOL, HGLOBAL);
		STDMETHOD(ContextSensitiveHelp) (BOOL);
		STDMETHOD(GetClipboardData) (CHARRANGE*, DWORD, LPDATAOBJECT*);
		STDMETHOD(GetDragDropEffect) (BOOL, DWORD, LPDWORD);
		STDMETHOD(GetContextMenu) (WORD, LPOLEOBJECT, CHARRANGE*, HMENU*);
	END_INTERFACE_PART(RichEditOleCallback)

	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
