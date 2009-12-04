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


// CLetterTextEditView.h : header file
//

#ifndef __CLETTERTEXTEDITVIEW__MULBERRY__
#define __CLETTERTEXTEDITVIEW__MULBERRY__

#include "CCmdEditView.h"

#include "CStyleToolbar.h"

/////////////////////////////////////////////////////////////////////////////
// CLetterTextEditView window

class CLetterWindow;
class CMessage;
class CStyleToolbar;

class CLetterTextEditView : public CCmdEditView
{
	friend class CLetterWindow;

	DECLARE_DYNCREATE(CLetterTextEditView)

// Construction
public:
	CLetterTextEditView();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLetterTextEditView)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLetterTextEditView();

	virtual void SetLetterWindow(CLetterWindow* ltrWindow)
					{ mLtrWindow = ltrWindow; }

	// Common updaters
	afx_msg void	OnUpdateAlways(CCmdUI* pCmdUI);

	afx_msg void	OnEditPaste(void);

	void SetToolbar(const CMIMEContent& mime);
	void SetToolbar(EContentSubType type);
	void ShowToolbar(EContentSubType type, bool override=false);
	void HideToolbar(bool override=false);
	CStyleToolbar* GetToolbar();
	CDisplayFormatter* GetFormatter();

	void	Reset(bool click = false, long scale = 0);
	
protected:
	// Printing
	virtual BOOL 	OnPreparePrinting(CPrintInfo* pInfo);
	virtual void 	OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void 	OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void 	OnPrint(CDC* pDC, CPrintInfo* pInfo);
	afx_msg int 	OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void 	OnSetFocus(CWnd* pOldWnd);
	afx_msg void 	OnKillFocus(CWnd* pNewWnd);
	afx_msg void 	OnSelChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void 	OnFont(UINT nID);
	afx_msg void 	OnFontSize(UINT nID);
	afx_msg void	OnUpdateFont(CCmdUI* pCmdUI);
	afx_msg void	OnCaptureAddress();

	virtual void	AddDropFlavor(unsigned int theFlavor)			// Set its drop flavor
						{ mDropFlavors.Add(theFlavor); }
	virtual HRESULT QueryAcceptData(LPDATAOBJECT, CLIPFORMAT*, DWORD, BOOL, HGLOBAL);
			BOOL	OnDrop(unsigned int theFlavor, COleDataObject* pDataObject);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	
private:
	CLetterWindow*		mLtrWindow;
	CDisplayFormatter*	mFormatter;
	CStyleToolbar 		mStyleToolbar;
	bool 				mToolbarShowing;
	bool				mAdjustedPrintMargins;
	CDWordArray			mDropFlavors;				// List of flavors to accept

			void	AdjustPrintMargins(CDC* pDC, CPrintInfo* pInfo, const CMessage* msg);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
