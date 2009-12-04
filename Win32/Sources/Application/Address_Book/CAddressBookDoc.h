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


// CAddressBookDoc.h : interface of the CAddressBookDoc class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CADDRESSBOOKDOC__MULBERRY__
#define __CADDRESSBOOKDOC__MULBERRY__

#include "CSDIDoc.h"

#include "CAddressList.h"
#include "CGroupList.h"

// Classes
class CAddressBook;
class CAddressBookWindow;

class CAddressBookDoc : public CSDIDoc
{
protected: // create from serialization only
	CAddressBookDoc();
	DECLARE_DYNCREATE(CAddressBookDoc)

// Attributes
public:

// Operations
public:
	virtual void SetAddressBookWindow(CAddressBookWindow* aAddressBookWnd)
			{ mAddressBookWnd = aAddressBookWnd; }
	virtual CAddressBookWindow* GetAddressBookWindow()
			{ return mAddressBookWnd; }
	virtual CAddressBook* GetAddressBook();

	virtual void InitDoc(CAddressBook* adbk);
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);		// Put up warning on close

	virtual void Serialize(CArchive& ar);				// Read/Write data from/to file

// Implementation
public:
	virtual ~CAddressBookDoc();

	// Common updaters
	afx_msg void	OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateFileRevert(CCmdUI* pCmdUI);

	// Command handlers
	afx_msg void	OnFileClose(void);
	afx_msg void	OnFileSave(void);
	afx_msg void	OnFileSaveAs(void);
	afx_msg void	OnFileRevert(void);

// Generated message map functions
protected:
	//{{AFX_MSG(CAddressBookDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG

private:
	CAddressBookWindow* mAddressBookWnd;

	virtual void		ReadFile(CArchive& ar);
	virtual void		SaveFile(CArchive& ar);

	virtual void		ExportTabbedAddresses(CArchive& ar);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
