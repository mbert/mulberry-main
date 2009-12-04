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


// Source for CAddressBookDoc class

#include "CAddressBookDoc.h"

#include "CAddressBookWindow.h"
#include "CCommands.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CLocalAddressBook.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CRemoteAddressBook.h"

#include "MyCFString.h"

const char* cAddressBookInfoSectionLocal = "Address Book Info";		// Also defined in CPreferences.cp

unsigned long CAddressBookDoc::sAddressBookCount = 1;

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S B O O K D O C
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressBookDoc::CAddressBookDoc(CAddressBook* adbk, PPx::FSObject* inFileSpec)
		: CNavSafeDoc(CMulberryApp::sApp)
{
	// Cache useful bits
	mIsLocal = dynamic_cast<CLocalAddressBook*>(adbk) != NULL;

	if (inFileSpec == nil)
	{
		// Create window for our document
		mWindow = (LWindow*) CAddressBookWindow::CreateWindow(paneid_AddressBookWindow, this);

		if (!mIsLocal)
		{
			MyCFString name(adbk->GetName(), kCFStringEncodingUTF8);
			mWindow->SetCFDescriptor(name);
		}
		else
			NameNewDoc();				// Set name of untitled document

		static_cast<CAddressBookWindow*>(mWindow)->SetDocument(this);
		static_cast<CAddressBookWindow*>(mWindow)->SetAddressBook(adbk);

	}
	else
	{
		// Create window for our document
		mWindow = (LWindow*) CAddressBookWindow::CreateWindow(paneid_AddressBookWindow, this);
		mWindow->SetCFDescriptor(inFileSpec->GetName());
		static_cast<CAddressBookWindow*>(mWindow)->SetDocument(this);
		static_cast<CAddressBookWindow*>(mWindow)->SetAddressBook(adbk);

		// Load contents of file - must do this AFTER creating window and setting address book
		OpenFile(*inFileSpec);
		
		// Now reset the address book display
		static_cast<CAddressBookWindow*>(mWindow)->ResetAddressBook();

		static_cast<CAddressBookWindow*>(mWindow)->SetDirty(CNavSafeDoc::IsModified());
		mWindow->Show();
	}

}

// Default destructor
CAddressBookDoc::~CAddressBookDoc()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

CAddressBook* CAddressBookDoc::GetAddressBook()
{
	return static_cast<CAddressBookWindow*>(mWindow)->GetAddressBook();
}

// Set modified
void CAddressBookDoc::SetModified(Boolean modified)
{
	// Change dirty state
	mIsModified = modified;
	if (mWindow)
		static_cast<CAddressBookWindow*>(mWindow)->SetDirty(modified);
}

// Is modified
Boolean CAddressBookDoc::IsModified(void)
{
	// Document has changed if the window is dirty
	if (mWindow)
		mIsModified = static_cast<CAddressBookWindow*>(mWindow)->IsDirty();
	return dynamic_cast<CLocalAddressBook*>(GetAddressBook()) && mIsModified;
}

// Close window - not document
void CAddressBookDoc::Close(void)
{
	// Save traits before close
	if (mIsSpecified || !mIsLocal)
		static_cast<CAddressBookWindow*>(mWindow)->SaveState();

	CNavSafeDoc::Close();
}

// Save state on close
void CAddressBookDoc::AttemptClose(Boolean inRecordIt)
{
	// Look for dangling messages then process close actions
	if (!static_cast<CAddressBookWindow*>(mWindow)->GetAddressBookView()->TestClose())
		return;

	// Close the view  - this will close the actual window at idle time
	static_cast<CAddressBookWindow*>(mWindow)->GetAddressBookView()->DoClose();

	// Do inherited after our special bit
	CNavSafeDoc::AttemptClose(inRecordIt);
}

// Save state on quit
Boolean CAddressBookDoc::AttemptQuitSelf(SInt32 inSaveOption)
{
	// Save traits before quit
	if (mIsSpecified)
		static_cast<CAddressBookWindow*>(mWindow)->SaveState();

	// Close the view  - this will close the actual window at idle time
	static_cast<CAddressBookWindow*>(mWindow)->GetAddressBookView()->DoClose();

	// Carry on as before
	return CNavSafeDoc::AttemptQuitSelf(inSaveOption);
}

//	Pass back status of a (menu) command
void CAddressBookDoc::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_ImportAddressBook:
		outEnabled = true;
		break;

	case cmd_ExportAddressBook:
		outEnabled = true;
		break;

	case cmd_Print:
	case cmd_PrintOne:
		outEnabled = false;
		break;

	case cmd_SaveAs:
	case cmd_Save:
	case cmd_Revert:
		if (!mIsLocal)
		{
			outEnabled = false;
			break;
		}
		// Fall through

	default:
		CNavSafeDoc::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
}

// Get name for new doc
void CAddressBookDoc::NameNewDoc(void)
{
	// Change title by appending count
	MyCFString name(mWindow->CopyCFDescriptor());
	if (sAddressBookCount++ > 1)
	{
		cdstring title = name.GetString();
		title += " ";
		title += (long) sAddressBookCount;
		MyCFString temp(title, kCFStringEncodingUTF8);
		mWindow->SetCFDescriptor(temp);
	}
}

// Open its file
void CAddressBookDoc::OpenFile(PPx::FSObject&	inFileSpec)
{
	// nil out for expection processing
	mFile = nil;

	try
	{
		// Create a new File object, read the entire File contents,
		// put the contents into the text view, and set the Window
		// title to the name of the File.

		// Create and open file
		mFile = new LFile(inFileSpec);
		mFile->OpenDataFork(fsRdWrPerm);

		// Read in data and set lists
		ReadFile();

		// Mark as specified
		mIsSpecified = true;
		SetModified(false);

		// Close file
		mFile->CloseDataFork();
	}
	catch (const PP_PowerPlant::LException& ex)
	{
		CLOG_LOGCATCH(PP_PowerPlant::LException);

		// Inform user
		CErrorHandler::PutOSErrAlertRsrc("Alerts::Adbk::OpenError", ex.GetErrorCode());

		if (mFile)
		{
			// Clean up - will close file
			delete mFile;
			mFile = nil;
		}

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mFile)
		{
			// Clean up - will close file
			delete mFile;
			mFile = nil;
		}

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

}

// Read data from file
void CAddressBookDoc::ReadFile(void)
{
	Handle data = mFile->ReadDataFork();
	SInt32 pos_end = ::GetHandleSize(data);
	::SetHandleSize(data, pos_end + 2);
	ThrowIfMemError_();
	(*data)[pos_end] = '\0';
	::HLock(data);

	GetAddressBook()->ImportAddresses(*data);

	::DisposeHandle(data);
}

// Save with AE
void CAddressBookDoc::DoAESave(PPx::FSObject& inFileSpec, OSType inFileType)
{
	// Kill existing file
	delete mFile;
	mFile = nil;
	mIsSpecified = false;
	bool saving = false;

	try
	{
		// Make new file object
		mFile = new LFile(inFileSpec);

		// Find proper file type
		OSType fileType = kAddressBookDocType;

		// Make new file on disk
		mFile->CreateNewFile(kApplID, kAddressBookDocType, 0);
		inFileSpec.Update();

		// Update FSRef in adbk
		((CLocalAddressBook*) GetAddressBook())->SetSpec(&inFileSpec);

		// Write out data
		saving = true;
		DoSave();

	}
	catch (const PP_PowerPlant::LException& ex)
	{
		CLOG_LOGCATCH(PP_PowerPlant::LException);

		// Inform user if not already done in DoSave
		if (!saving)
			CErrorHandler::PutOSErrAlertRsrc("Alerts::Adbk::SaveError", ex.GetErrorCode());

		// Delete any saved file on disk
		if (mFile)
		{
			// Clean up - will close file
			delete mFile;
			mFile = nil;

			// Then delete
			try
			{
				inFileSpec.Delete();
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);

			}
		}

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Delete any saved file on disk
		if (mFile)
		{
			// Clean up - will close file
			delete mFile;
			mFile = nil;

			// Then delete
			try
			{
				inFileSpec.Delete();
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);

			}
		}

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}


	// Change window name
	if (mWindow)
	{
		mWindow->SetCFDescriptor(inFileSpec.GetName());
	}

	// Document now has a specified file
	mIsSpecified = true;

}

// Save the file
void CAddressBookDoc::DoSave(void)
{
	try
	{
		// Open file
		mFile->OpenDataFork(fsRdWrPerm);

		// Export to stream
		LHandleStream data;
		ExportTabbedAddresses(data);

		{
			StHandleLocker lock(data.GetDataHandle());
			mFile->WriteDataFork(*(data.GetDataHandle()), data.GetLength());
		}

		SetModified(false);

		// Close file
		mFile->CloseDataFork();
	}
	catch (const PP_PowerPlant::LException& ex)
	{
		CLOG_LOGCATCH(PP_PowerPlant::LException);

		// Inform user
		CErrorHandler::PutOSErrAlertRsrc("Alerts::Adbk::SaveError", ex.GetErrorCode());

		// Close file
		mFile->CloseDataFork();

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Close file
		mFile->CloseDataFork();

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
}

void CAddressBookDoc::ExportTabbedAddresses(LStream& out)
{
	for(CAddressList::const_iterator iter = GetAddressBook()->GetAddressList()->begin(); iter != GetAddressBook()->GetAddressList()->end(); iter++)
	{
		const char* out_addr = GetAddressBook()->ExportAddress(static_cast<const CAdbkAddress*>(*iter));
		out.WriteBlock(out_addr, ::strlen(out_addr));
		delete out_addr;
	}

	for(CGroupList::iterator iter = GetAddressBook()->GetGroupList()->begin(); iter != GetAddressBook()->GetGroupList()->end(); iter++)
	{
		const char* out_grp = GetAddressBook()->ExportGroup(*iter);
		out.WriteBlock(out_grp, ::strlen(out_grp));
		delete out_grp;
	}
}

// Revert the file
void CAddressBookDoc::DoRevert(void)
{
	// Delete items in existing lists
	GetAddressBook()->Clear();

	try
	{
		// Open file
		mFile->OpenDataFork(fsRdWrPerm);

		// Read data again from file
		ReadFile();

		// Close file
		mFile->CloseDataFork();

		// Force window to update
		if (mWindow)
			static_cast<CAddressBookWindow*>(mWindow)->GetAddressBookView()->ResetAddressBook();

		// No longer dirty
		SetModified(false);
	}
	catch (const PP_PowerPlant::LException& ex)
	{
		CLOG_LOGCATCH(PP_PowerPlant::LException);

		// Inform user
		CErrorHandler::PutOSErrAlertRsrc("Alerts::Adbk::RevertError", ex.GetErrorCode());

		// Close file
		mFile->CloseDataFork();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Close file
		mFile->CloseDataFork();
	}
}
