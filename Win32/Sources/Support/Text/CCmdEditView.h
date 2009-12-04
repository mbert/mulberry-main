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


// CCmdEditView.h : header file
//

#ifndef __CCMDEDITVIEW__MULBERRY__
#define __CCMDEDITVIEW__MULBERRY__

#include "CCommander.h"
#include "CBroadcaster.h"
#include "CTabber.h"

#include "cdstring.h"
#include "cdustring.h"

#include <TOM.h>

/////////////////////////////////////////////////////////////////////////////
// CCmdEditView view

class CCmdEditView : public CRichEditView, public CCommander, public CBroadcaster, public CTabber
{
	friend class CFindReplaceWindow;
	friend class CSpellPlugin;

public:
	enum
	{
		eBroadcast_Activate = 'TXac',
		eBroadcast_Deactivate = 'TXda',
		eBroadcast_SelectionChanged = 'TXsc'
	};

	class StPreserveSelection
	{
	public:
		StPreserveSelection(CCmdEditView* view)
			{ mView = view;
			  mView->GetRichEditCtrl().GetSel(selStart, selEnd); }
		~StPreserveSelection()
			{ mView->GetRichEditCtrl().SetSel(selStart, selEnd); }
		void Offset(long start, long delta)
			{ if (selStart > start) selStart += delta;
			  if (selEnd > start) selEnd += delta; }
	private:
		CCmdEditView* mView;
		long selStart;
		long selEnd;
	};

protected:
	CCmdEditView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCmdEditView)

// Attributes
public:
	virtual void	ResetFont(CFont* pFont, bool preserve_styles = false);		// Change font in whole control

	virtual BOOL	PreTranslateMessage(MSG* pMsg);
	virtual void	SetContextMenuID(UINT contextID)			// Set context menu ID
						{ mContextMenuID = contextID; }

	virtual void	SetSpacesPerTab(short numSpacesPerTab);				// Set spaces per tab
	virtual short	GetSpacesPerTab(void) const {return mSpacesPerTab;}	// Get spaces per tab

	virtual void	SetWrap(short wrap);								// Set wrap width
	virtual short	GetWrap(void) const
						{return mWrap;}									// Get wrap width

	virtual void	SetHardWrap(bool hard_wrap);						// Set wrapping mode
	virtual void	ResetHardWrap();									// Reset wrapping mode
	virtual bool	GetHardWrap(void) const
						{return mHardWrap;}								// Get wrapping mode

	virtual void	SetQuotation(const cdstring& quotation)				// Set quotation
		{ mQuotation = quotation; }
	virtual const cdstring&	GetQuotation(void) const					// Get quotation
		{ return mQuotation; }

	virtual void	SetFindAllowed(bool find_allowed)					// Set find allowed
						{ mFindAllowed = find_allowed; }
	virtual bool	GetFindAllowed(void) const							// Get find allowed
						{ return mFindAllowed; }

	virtual long	GetTextLengthEx() const;
	virtual void	GetSelectionRange(long& selStart, long& selEnd) const
		{ return GetRichEditCtrl().GetSel(selStart, selEnd); }
	virtual void	SetSelectionRange(long selStart, long selEnd)
		{ return GetRichEditCtrl().SetSel(selStart, selEnd); }
	virtual void	GetSelectedText(cdstring& selected) const;
	virtual void	GetSelectedText(cdustring& selected) const;

			void	SetText(const cdstring& all);						// UTF8 in
			void	SetText(const cdustring& all);						// UTF16 in

			void	InsertUTF8(const cdstring& txt);						// Insert unstyled utf8 at selection
			void	InsertUTF8(const char* txt, size_t len = -1);			// Insert unstyled utf8 at selection
			void	InsertText(const cdustring& txt);						// Insert unstyled utf16 at selection
			void	InsertText(const unichar_t* utxt, size_t len = -1);		// Insert unstyled utf16 at selection

			void	GetText(cdstring& all) const;						// UTF8 out
			void	GetText(cdustring& all) const;						// UTF16 out
			void	GetRawText(cdstring& all) const;					// UTF8 out
			void	GetRawText(cdustring& all) const;					// UTF16 out

	virtual void	UpdateMargins();
	virtual void	RestoreMargins();

	virtual bool	GetCtrlRedraw() const
		{ return mCtrlRedraw; }
	virtual void	SetCtrlRedraw(bool redraw);

			void	GetScrollPos(long& h, long& v) const;			// Get scrollbar pos
			void	SetScrollPos(long h, long v);					// Set scrollbar pos

	virtual	bool	HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags);			// Handle character

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCmdEditView)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	UINT				mContextMenuID;
	bool 				mShowStyled;
	CCommanderProtect	mCmdProtect;					// Protect commands

	virtual ~CCmdEditView();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void HandleContextMenu(CWnd*, CPoint point);
	virtual HRESULT QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR* lpcfFormat, DWORD dwReco, BOOL bReally, HGLOBAL hMetaFile);

	// Generated message map functions
protected:
	//{{AFX_MSG(CCmdEditView)
	afx_msg int  OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);

	afx_msg void OnContextMenu(CWnd*, CPoint point);
	afx_msg	void OnLButtonUp(UINT nFlags, CPoint point);				// Clicked somewhere
	afx_msg	void OnRButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere
	afx_msg void OnUpdateReadWrite(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectionReadWrite(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPasteReadWrite(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();


	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnSelChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEditChange();

	// Text processing commands
	afx_msg void	OnWrapLines();
	afx_msg void	OnUnwrapLines();
	afx_msg void	OnQuoteLines();
	afx_msg void	OnUnquoteLines();
	afx_msg void	OnRequoteLines();
	afx_msg void	OnShiftLeft();
	afx_msg void	OnShiftRight();

	virtual bool	ExpandMacro();

	virtual void	PrepareTextProcessing(long& sel_start, long& sel_end, cdstring& selected);
	virtual void	EndTextProcessing(const char* insert_text, long& sel_start, long& sel_end);
	virtual void	SelectFullLines();

	// Find & Replace command handlers
	afx_msg void	OnUpdateFindText(CCmdUI* pCmdUI);
	afx_msg void	OnFindText();
	afx_msg void	OnUpdateFindNextText(CCmdUI* pCmdUI);
	afx_msg void	OnFindNextText();
	afx_msg void	OnFindNextBackText();
	afx_msg void	OnUpdateFindSelectionText(CCmdUI* pCmdUI);
	afx_msg void	OnFindSelectionText();
	afx_msg void	OnFindSelectionBackText();
	afx_msg void	OnUpdateReplaceText(CCmdUI* pCmdUI);
	afx_msg void	OnReplaceText();
	afx_msg void	OnUpdateReplaceFindText(CCmdUI* pCmdUI);
	afx_msg void	OnReplaceFindText();
	afx_msg void	OnReplaceFindBackText();
	afx_msg void	OnUpdateReplaceAllText(CCmdUI* pCmdUI);
	afx_msg void	OnReplaceAllText();

	//}}AFX_MSG

	// Find & Replace commands
	virtual bool	DoFindNextText(bool backwards, bool silent = false);
	virtual void	DoFindSelectionText(bool backwards);
	virtual void	DoReplaceText();
	virtual void	DoReplaceFindText(bool backwards);
	virtual void	DoReplaceAllText();


	// Spelling commands

	// Real-time spelling checker state information:
	struct RTSpell
	{
		// Current state
		enum ESpellState {eNotInWord, eInUnmarkedWord, eInMarkedWord};
		ESpellState state;
		long cursorPos;			// Last know cursor position
		long charCount;			// used to detect insertions
		long wordStartPos;		// current word starting position
		long contextWordPos;	// position of context menu word
		bool busy;				// TRUE if already processing events
		bool enabled;			// TRUE if real-time speller enabled
		bool pending;			// TRUE if spell check text is pending
		CComPtr<ITextDocument>	tomDoc;	// Text Object Model document object used to do style changes withut selections
	};

public:
			friend class StPauseSpelling;
			class StPauseSpelling
			{
			public:
				StPauseSpelling(CCmdEditView* text)
					{ mText = text;
					  mWasEnabled = mText->PauseSpelling(true); }
				~StPauseSpelling()
					{ if (mWasEnabled) mText->PauseSpelling(false); }
			private:
				CCmdEditView*	mText;
				bool			mWasEnabled;
			};

protected:
			void	SpellAutoCheck(bool auto_check);
			bool	IsMisspelled(long sel_start, long sel_end) const;
			long	GetWordAt(long pos, cdustring& word, long* startPos, bool word_start) const;
			unichar_t	GetTextChar(long pos) const;
			long	GetSpellTextRange(long start, long end, cdustring& text) const;
			bool	GetMisspelled(long pos, cdustring& word, long& word_start) const;
			void	MarkMisspelled(long sel_start, long sel_end, bool misspelled);
			bool	CheckWord(const cdustring& word, long start);
			void	RecheckAllWords(const cdustring& word);
			void	SpellInsertText(long start, long numchars);
			void	SpellTextRequired()
				{ mRTSpell.pending = true; }
			void	SpellTextRequiredCheck()
				{ if (mRTSpell.pending) SpellTextChange(); }
			void	SpellTextChange();
			bool	PauseSpelling(bool pause);

private:
	short		mSpacesPerTab;								// Number of spaces to use in place of TAB
	short		mWrap;										// Size to hard wrap to
	bool		mHardWrap;									// Whether to hard wrap
	int			mWrapWidth;
	cdstring	mQuotation;									// String to use/recognise as prefix
	bool		mFindAllowed;								// Find allowed?
	bool		mCtrlRedraw;								// Redraw turned on
	bool		mHasVScroll;								// Needed for margin setting problems

	RTSpell		mRTSpell;


	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

typedef CCmdEditView	CTextDisplay;

class StStopRedraw
{
public:
	StStopRedraw(CCmdEditView* view)
		{ mView = view;
		  mRedrawWasOn = mView->GetCtrlRedraw();
		  if (mRedrawWasOn) mView->SetCtrlRedraw(false); }
	~StStopRedraw()
		{ if (mRedrawWasOn) mView->SetCtrlRedraw(true); }
private:
	CCmdEditView* mView;
	bool mRedrawWasOn;
};

class StStopRedrawPreserveUpdate
{
public:
	StStopRedrawPreserveUpdate(CCmdEditView* view);
	~StStopRedrawPreserveUpdate();
private:
	CCmdEditView*	mView;
	CRgn*			mRgn;
	bool			mRedrawWasOn;
};

#endif
