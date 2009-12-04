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


// CLetterDoc.h : interface of the CLetterDoc class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CLETTERDOC__MULBERRY__
#define __CLETTERDOC__MULBERRY__

#include "CSDIDoc.h"

class CLetterWindow;

class CLetterDoc : public CSDIDoc
{
protected: // create from serialization only
	CLetterDoc();
	DECLARE_DYNCREATE(CLetterDoc)

// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual ~CLetterDoc();

	virtual void			SaveTemporary();
	static  unsigned long	SaveAllTemporary();
	virtual void			DeleteTemporary();
	static	void			ReadTemporary();
	static	void			MakeTemporary(const cdstring& path, unsigned long ctr);

	virtual void SetLetterWindow(CLetterWindow* aLtrWnd)
			{ mLtrWnd = aLtrWnd; }

	virtual BOOL IsModified();				// Override to check window IsDirty

	virtual BOOL SaveModified();			// Put up warning on close
	virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE);	// Handle illegal file name
	virtual void Serialize(CArchive& ar);	// Save data to file

	virtual BOOL		DoFileSave();
	virtual void		DoMailboxSave();

	// Common updaters
	afx_msg void	OnUpdateAdminLockDraftSave(CCmdUI* pCmdUI);

// Generated message map functions
protected:
	//{{AFX_MSG(CLetterDoc)
	afx_msg void OnFileSaveAs();
	//}}AFX_MSG

private:
	static unsigned long	sTemporaryCount;

	CLetterWindow*	mLtrWnd;
	bool			mSaveToChoiceDone;
	bool			mSaveToMailbox;
	cdstring		mMailboxSave;
	cdstring		mTemporary;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
