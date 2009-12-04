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

#include "CFileDocument.h"

// Classes
class CAddressBook;
class CAddressBookWindow;

class CAddressBookDoc : public CFileDocument
{
public:
			CAddressBookDoc(JXDirector* owner);
	virtual ~CAddressBookDoc();

// Operations
public:
	virtual void SetAddressBookWindow(CAddressBookWindow* aAddressBookWnd)
			{ mAddressBookWnd = aAddressBookWnd; }
	virtual CAddressBookWindow* GetAddressBookWindow()
			{ return mAddressBookWnd; }
	virtual CAddressBook* GetAddressBook() const;

	virtual void InitDoc(CAddressBook* adbk);
	virtual JBoolean OKToClose();						// Put up warning on close

// Implementation
public:

// Generated message map functions
protected:
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	virtual void		ReadTextFile(std::istream& input);
	virtual void		WriteTextFile(std::ostream& output, const JBoolean safetySave) const;
	virtual void		ExportTabbedAddresses(std::ostream& output) const;

	// Common updaters
	void	OnUpdateFileSave(CCmdUI* pCmdUI);
	void	OnUpdateFileRevert(CCmdUI* pCmdUI);

	// Command handlers
	void	OnOpenDocument(const JString& fname);
	void	OnFileClose();
	void	OnFileSave();
	void	OnFileSaveAs();
	void	OnFileRevert();

private:
	CAddressBookWindow* mAddressBookWnd;
};

/////////////////////////////////////////////////////////////////////////////

#endif
