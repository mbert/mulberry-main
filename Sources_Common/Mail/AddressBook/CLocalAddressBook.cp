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


// CLocalAddressBook.cp

// Class to handle remote address books

#include "CLocalAddressBook.h"

#include "CAdbkManagerTable.h"
#include "CAdbkProtocol.h"
#include "CAddressBookDoc.h"
#include "CAddressBookManager.h"
#if __dest_os == __win32_os || __dest_os == __linux_os
#include "CAddressBookWindow.h"
#endif
#include "CErrorHandler.h"
#include "CLog.h"
#include "CURL.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "FullPath.h"
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x

CLocalAddressBook::CLocalAddressBook(PPx::FSObject* fspec)
{
	mSpecified = (fspec != NULL);
	if (mSpecified)
		mFileSpec = *fspec;
	SetFlags(eLocalAdbk);
	SetFlags(eCachedAdbk);
}

CLocalAddressBook::CLocalAddressBook(PPx::FSObject* fspec, const char* name)
	: CAddressBook(name)
{
	mSpecified = (fspec != NULL);
	if (mSpecified)
		mFileSpec = *fspec;
	SetFlags(eLocalAdbk);
	SetFlags(eCachedAdbk);
}

#else

CLocalAddressBook::CLocalAddressBook(const char* fname)
{
	mFileName = fname;
	SetFlags(eLocalAdbk);
	SetFlags(eCachedAdbk);
}

CLocalAddressBook::CLocalAddressBook(const char* fname, const char* name)
	: CAddressBook(name)
{
	mFileName = fname;
	SetFlags(eLocalAdbk);
	SetFlags(eCachedAdbk);
}

#endif

// Destructor
CLocalAddressBook::~CLocalAddressBook()
{
	Close();
}

// New visual address book on source
cdstring CLocalAddressBook::GetAccountName() const
{
	// Just use normal name
	return GetName();
}

// New visual address book on source
cdstring CLocalAddressBook::GetURL(bool full) const
{
	cdstring url = cFileURLScheme;
	url += cURLLocalhost;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (mSpecified)
	{
		MyCFString cfstr(mFileSpec.GetPath());
		cdstring fpath(cfstr);

		// Convert path to URL & convert directories
		fpath.EncodeURL(os_dir_delim);
		char* p = fpath;
		while(*p)
		{
			if (*p == os_dir_delim) *p = '/';
			p++;
		}
		url += fpath;
	}
#else
	cdstring fpath = mFileName;

	// Convert path to URL & convert directories
	fpath.EncodeURL(os_dir_delim);
	char* p = fpath;
	while(*p)
	{
		if (*p == os_dir_delim) *p = '/';
		p++;
	}
	url += fpath;
#endif
	return url;
}

// New visual address book on source
void CLocalAddressBook::New()
{
	// Now try to create document
	CAddressBookDoc* aDoc = NULL;
	try
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		aDoc = new CAddressBookDoc(this, NULL);
		mAdbkName = cdstring(aDoc->GetDescriptor());
		aDoc->GetWindow()->Show();
#elif __dest_os == __win32_os
		aDoc = CAddressBookWindow::ManualCreate(this, NULL);
		mAdbkName = aDoc->GetTitle();
		aDoc->GetAddressBookWindow()->GetParentFrame()->ShowWindow(SW_SHOW);
#elif __dest_os == __linux_os
		aDoc = CAddressBookWindow::ManualCreate(this, NULL);
		mAdbkName = aDoc->GetWindow()->GetTitle();
		aDoc->GetWindow()->Show();
#else
#error __dest_os
#endif

		// Add to manager
		CAddressBookManager::sAddressBookManager->AddLocal(this);

		// Do inherited
		CAddressBook::New();

		// Must mark as loaded
		CAddressBook::Read();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		delete aDoc;
#elif __dest_os == __win32_os
		if (aDoc)
			aDoc->OnCloseDocument();
#elif __dest_os == __linux_os
		if (aDoc)
			aDoc->Close();
#else
#error __dest_os
#endif
		
		// Always throw up
		CLOG_LOGRETHROW;
		throw;
	}
}

// Open visual address book from source
void CLocalAddressBook::Open()
{
	// Only do this once
	if (IsOpen())
		return;

	// Now try to create document
	CAddressBookDoc* aDoc = NULL;
	try
	{
		// Do inherited
		// Do this first to ensure flag is set
		CAddressBook::Open();

		// Now create document - this will call Open again
		// so we have to set the flag first (above)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		aDoc = new CAddressBookDoc(this, &mFileSpec);
		mAdbkName = cdstring(aDoc->GetDescriptor());
#elif __dest_os == __win32_os
		aDoc = CAddressBookWindow::ManualCreate(this, mFileName);
		mAdbkName = aDoc->GetTitle();
#elif __dest_os == __linux_os
		aDoc = CAddressBookWindow::ManualCreate(this, mFileName);
		mAdbkName = aDoc->GetWindow()->GetTitle();
#else
#error __dest_os
#endif

		// Add to manager
		CAddressBookManager::sAddressBookManager->AddLocal(this);

		// Must mark as loaded
		CAddressBook::Read();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		delete aDoc;
#elif __dest_os == __win32_os
		if (aDoc)
			aDoc->OnCloseDocument();
#elif __dest_os == __linux_os
		if (aDoc)
			aDoc->Close();
#else
#error __dest_os
#endif
	}
}

// Read in addresses
void CLocalAddressBook::Read()
{
}

// Save addresses
void CLocalAddressBook::Save()
{
}

// Close visual address book
void CLocalAddressBook::Close()
{
	// Only if already open
	if (!IsOpen())
		return;

	// Remove from manager
	CAddressBookManager::sAddressBookManager->RemoveLocal(this);

	// Do inherited
	CAddressBook::Close();
}

// Rename address book
void CLocalAddressBook::Rename(cdstring& new_name)
{
	try
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		MyCFString rename(new_name, kCFStringEncodingUTF8);
		mFileSpec.Rename(rename);
#elif __dest_os == __win32_os
		CFile::Rename(mFileName.win_str(), new_name.win_str());
		mFileName = new_name;
#elif __dest_os == __linux_os
		rename_utf8(mFileName, new_name);
		mFileName = new_name;
#else
#error __dest_os
#endif

		// Call inherited
		CAddressBook::Rename(new_name);
	}
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	catch(const PP_PowerPlant::LException& ex)
#elif __dest_os == __win32_os || __dest_os == __linux_os
	catch(CFileException* ex)
#else
#error __dest_os
#endif
	{
		CLOG_LOGCATCH(CFileException);

		// Inform user
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		CErrorHandler::PutOSErrAlertRsrc("Alerts::Adbk::RenameError", ex.GetErrorCode());
#elif __dest_os == __win32_os || __linux_os
		CErrorHandler::PutFileErrAlertRsrc("Alerts::Adbk::RenameError", *ex);
#else
#error __dest_os
#endif
		CLOG_LOGRETHROW;
		throw;
	}
}

// Rename address book
void CLocalAddressBook::Delete()
{
	// Do inherited first
	CAddressBook::Delete();

	try
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		mFileSpec.Delete();
#elif __dest_os == __win32_os
		CFile::Remove(mFileName.win_str());
#elif __dest_os == __linux_os
		unlink(mFileName);
#else
#error __dest_os
#endif
	}
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	catch(const PP_PowerPlant::LException& ex)
#elif __dest_os == __win32_os || __dest_os == __linux_os
	catch(CFileException* ex)
#else
#error __dest_os
#endif
	{
		CLOG_LOGCATCH(CFileException);

		// Inform user
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		CErrorHandler::PutOSErrAlertRsrc("Alerts::Adbk::DeleteError", ex.GetErrorCode());
#elif __dest_os == __win32_os || __dest_os == __linux_os
		CErrorHandler::PutFileErrAlertRsrc("Alerts::Adbk::DeleteError", *ex);
#else
#error __dest_os
#endif
		CLOG_LOGRETHROW;
		throw;
	}
}
