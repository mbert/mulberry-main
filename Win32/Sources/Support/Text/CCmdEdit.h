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


// CCmdEdit.h : header file
//

#ifndef __CCMDEDIT__MULBERRY__
#define __CCMDEDIT__MULBERRY__

#include "CCommander.h"
#include "CTabber.h"

#include "cdstring.h"
#include "cdustring.h"

/////////////////////////////////////////////////////////////////////////////
// CCmdEdit view

class CCmdEdit : public CRichEditCtrl, public CCommander, public CTabber
{
public:
	class StPreserveSelection
	{
	public:
		StPreserveSelection(CCmdEdit* edit)
			{ mEdit = edit;
			  mEdit->GetSel(selStart, selEnd); }
		~StPreserveSelection()
			{ mEdit->SetSel(selStart, selEnd); }
	private:
		CCmdEdit* mEdit;
		long selStart;
		long selEnd;
	};

public:
	CCmdEdit();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCmdEdit)

	virtual ~CCmdEdit();

	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Attributes
public:
	virtual void	ResetFont(CFont* pFont);					// Change font in whole control
	virtual BOOL	PreTranslateMessage(MSG* pMsg);
	virtual void	SetContextMenuID(UINT contextID)			// Set context menu ID
						{ mContextMenuID = contextID; }
	virtual void	SetContextView(CView* aView)				// Set context view
						{ mContextView = aView; }

	virtual void	SetSpacesPerTab(short numSpacesPerTab)
						{mSpacesPerTab = numSpacesPerTab;}				// Set spaces per tab
	virtual short	GetSpacesPerTab(void) const {return mSpacesPerTab;}	// Get spaces per tab

	virtual void	SetReturnCmd(bool return_cmd)					// Set return key command
						{ mReturnCmd = return_cmd; }
	virtual void	SetAnyCmd(bool any_cmd)							// Set any key command
						{ mAnyCmd = any_cmd; }

	virtual long	GetTextLengthEx() const;
	virtual void	GetSelectedText(cdstring& selected) const;
	virtual void	GetSelectedText(cdustring& selected) const;

			void	SetText(const cdstring& all);					// UTF8 in
			void	SetText(const cdustring& all);					// UTF16 in

			void	InsertUTF8(const cdstring& txt);						// Insert unstyled utf8 at selection
			void	InsertUTF8(const char* txt, size_t len = -1);			// Insert unstyled utf8 at selection
			void	InsertText(const cdustring& txt);						// Insert unstyled utf16 at selection
			void	InsertText(const unichar_t* utxt, size_t len = -1);		// Insert unstyled utf16 at selection

			void	GetText(cdstring& all) const;					// UTF8 out
		cdstring	GetText() const;							// UTF8 out
			void	GetText(cdustring& all) const;					// UTF16 out

			void	GetRawText(cdstring& all) const;				// UTF8 out
			void	GetRawText(cdustring& all) const;				// UTF16 out

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCmdEdit)
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	UINT				mContextMenuID;
	CView*				mContextView;
	bool 				mShowStyled;
	CCommanderProtect	mCmdProtect;					// Protect commands

	virtual void HandleContextMenu(CWnd*, CPoint point);

	// Generated message map functions
protected:
	//{{AFX_MSG(CCmdEdit)
	afx_msg void OnUpdateAlways(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedSel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedClip(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditReadWrite(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPasteReadWrite(CCmdUI* pCmdUI);

	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);

	afx_msg void OnContextMenu(CWnd*, CPoint point);
	afx_msg	void OnRButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere
	afx_msg void OnEditUndo(void);
	afx_msg void OnEditRedo(void);
	afx_msg void OnEditSelectAll(void);
	afx_msg void OnPaste();
	afx_msg void OnCut();
	afx_msg void OnClear();

	afx_msg int	 OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

private:
	short	mSpacesPerTab;								// Number of spaces to use in place of TAB
	bool 	mReturnCmd;									// Return key sends command
	bool 	mAnyCmd;									// Any key sends command

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
