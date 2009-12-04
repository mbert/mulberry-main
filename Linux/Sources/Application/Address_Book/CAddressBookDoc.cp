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


// CAddressBookDoc.cp : implementation of the CAddressBookDoc class
//


#include "CAddressBookDoc.h"

#include "CAddressBook.h"
#include "CAddressBookWindow.h"
#include "CCommands.h"
#include "CErrorHandler.h"
#include "CLocalAddressBook.h"
#include "CLog.h"
#include "CMulberryApp.h"

#include "cdfstream.h"

#include <JXChooseSaveFile.h>
#include <jFileUtil.h>
#include <jXGlobals.h>

#include <memory>

#define COMMA_SPACE			", "

const char cAnon[]				= "Anonymous";
const char cGroupHeader[]		= "Group:";
const char cNewGroupHeader[]	= "Grp:";

/////////////////////////////////////////////////////////////////////////////
// CAddressBookDoc

/////////////////////////////////////////////////////////////////////////////
// CAddressBookDoc construction/destruction

CAddressBookDoc::CAddressBookDoc(JXDirector* owner)
	: CFileDocument(owner, "", kFalse, kFalse, ".mba")
{
	mAddressBookWnd = NULL;
}

CAddressBookDoc::~CAddressBookDoc()
{
}

void CAddressBookDoc::InitDoc(CAddressBook* adbk)
{
}

CAddressBook* CAddressBookDoc::GetAddressBook() const
{
	return mAddressBookWnd->GetAddressBook();
}

// Is closing allowed
JBoolean CAddressBookDoc::OKToClose()
{
	// Look for dangling messages then process close actions
	if (!GetAddressBookWindow()->GetAddressBookView()->TestClose())
		return kFalse;

	// Close the view  - this will close the actual window at idle time
	GetAddressBookWindow()->GetAddressBookView()->DoClose();
	
	return kTrue;
}

bool CAddressBookDoc::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch(cmd)
	{
	case CCommand::eFileClose:
		OnFileClose();
		return true;
	case CCommand::eFileSave:
		OnFileSave();
		return true;
	case CCommand::eFileSaveAs:
		OnFileSaveAs();
		return true;
	case CCommand::eFileRevert:
		OnFileRevert();
		return true;
	default:;
	}

	return CFileDocument::ObeyCommand(cmd, menu);
}

void CAddressBookDoc::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eFileSave:
		OnUpdateFileSave(cmdui);
		return;
	case CCommand::eFileSaveAs:
		OnUpdateAlways(cmdui);
		return;
	case CCommand::eFileRevert:
		OnUpdateFileRevert(cmdui);
		return;
	default:;
	}

	CFileDocument::UpdateCommand(cmd, cmdui);
}

void CAddressBookDoc::OnUpdateFileSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(false);
}

void CAddressBookDoc::OnUpdateFileRevert(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(false);
}

void CAddressBookDoc::OnOpenDocument(const JString& fname)
{
	// Must exist
	if (JFileReadable(fname))
	{
		cdifstream input(fname);
		ReadTextFile(input);
		FileChanged(fname, kTrue);
	}
	else
	{
		CLOG_LOGTHROW(CGeneralException, -1L);
		throw CGeneralException(-1L);
	}
}

void CAddressBookDoc::OnFileClose()
{
	Close();
}

void CAddressBookDoc::OnFileSave()
{
	// Do inherited
	SaveInCurrentFile();
}

void CAddressBookDoc::OnFileSaveAs()
{
	// Do inherited
	SaveInNewFile();
	
	// Check for name change
	if (GetFileName() != GetAddressBook()->GetName())
		GetAddressBook()->SetName(GetFileName().GetCString());
}

void CAddressBookDoc::OnFileRevert()
{
	if (!OKToRevert())
		return;

	// Delete items in existing lists
	GetAddressBook()->GetAddressList()->clear();
	GetAddressBook()->GetGroupList()->clear();
	
	try
	{
		// Reopen and read data
		DataReverted();
		OnOpenDocument(GetFilePath());
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Inform user
		CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::RevertError");
	}
}

// Read data from file
void CAddressBookDoc::ReadTextFile(std::istream& input)
{
	// Read in string to arbitrary line end
	cdstring s;
	::getline(input, s, 0);

	while (!s.empty())
	{
		GetAddressBook()->ImportAddress(s.c_str_mod(), true, NULL, NULL);

		// Read in string to arbitrary line end
		::getline(input, s, 0);
	}
}

void CAddressBookDoc::WriteTextFile(std::ostream& output, const JBoolean safetySave) const
{
	ExportTabbedAddresses(output);
}

void CAddressBookDoc::ExportTabbedAddresses(std::ostream& output) const
{
	for(CAddressList::const_iterator iter = GetAddressBook()->GetAddressList()->begin(); iter != GetAddressBook()->GetAddressList()->end(); iter++)
	{
		std::auto_ptr<const char> out_addr(GetAddressBook()->ExportAddress(static_cast<const CAdbkAddress*>(*iter)));
		output.write(out_addr.get(), ::strlen(out_addr.get()));
	}

	for(CGroupList::iterator iter = GetAddressBook()->GetGroupList()->begin(); iter != GetAddressBook()->GetGroupList()->end(); iter++)
	{
		std::auto_ptr<const char> out_grp(GetAddressBook()->ExportGroup(*iter));
		output.write(out_grp.get(), ::strlen(out_grp.get()));
	}
}

