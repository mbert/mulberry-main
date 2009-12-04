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


// Source for CLetterDoc class

#include "CLetterDoc.h"

#include "CAdminLock.h"
#include "CAttachment.h"
#include "CAttachmentList.h"
#include "CCaptionParser.h"
#include "CCommands.h"
#include "CConnectionManager.h"
#include "CCopyToMenu.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CFileAttachment.h"
#include "CHeadAndFoot.h"
#include "CLetterWindow.h"
#include "CMailAccountManager.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSaveDraftDialog.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"
#include "CEditFormattedTextDisplay.h"

#include "MyCFString.h"
#include "diriterator.h"

#include <UStandardDialogs.h>
#include <UGraphicUtils.h>

#include <stdio.h>
#include <string.h>
#include <strstream>

// __________________________________________________________________________________________________
// C L A S S __ C L E T T E R D O C
// __________________________________________________________________________________________________

// Static members

unsigned long	CLetterDoc::sTemporaryCount = 1;
const char* cTemporaryDraftName = "MulberryDraft";
const char* cTemporaryDraftExtension = ".mbd";
const char* cRecoveredDraftName = "Recovered Draft ";

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CLetterDoc::CLetterDoc(LCommander* inSuper,
						 PPx::FSObject* inFileSpec)
		: CNavSafeDoc(inSuper)
{
									// Create window for our document
	mWindow = (LWindow*) CLetterWindow::CreateWindow(paneid_LetterWindow, this);

	if (inFileSpec == NULL) {
		NameNewDoc();				// Set name of untitled window
	} else {
		OpenFile(*inFileSpec);		// Display contents of file in window
	}
	
	mSaveToMailbox = false;
	mAutoSaveTime = ::time(NULL);

	StartRepeating();
}

// Default destructor
CLetterDoc::~CLetterDoc()
{
	DeleteTemporary();
}


// O T H E R  M E T H O D S ____________________________________________________________________________

Boolean CLetterDoc::IsSpecified() const
{
	// Look at preference option
	switch(CPreferences::sPrefs->mSaveOptions.GetValue().GetValue())
	{
	case eSaveDraftToFile:
		return mIsSpecified && mFile;
	case eSaveDraftToMailbox:
		return mIsSpecified;
	case eSaveDraftChoose:
		if (mSaveToMailbox)
			return mIsSpecified;
		else
			return mIsSpecified && mFile;
	}
	return mIsSpecified;
}

// Set modified
void CLetterDoc::SetModified(Boolean modified)
{
	// Change dirty state
	mIsModified = modified;
	static_cast<CLetterWindow*>(mWindow)->SetDirty(modified);
}

// Is modified
Boolean CLetterDoc::IsModified()
{
	// Document has changed if the window is dirty
	mIsModified = (mWindow != NULL) && static_cast<CLetterWindow*>(mWindow)->IsDirty();
	return mIsModified;
}

// Stop save action if demo
Boolean CLetterDoc::ObeyCommand(CommandT inCommand, void *ioParam)
{
	Boolean		cmdHandled = true;

	switch (inCommand)
	{
	// Fake a save
	case cmd_ToolbarFileSaveBtn:
		cmdHandled = CNavSafeDoc::ObeyCommand(cmd_Save, ioParam);
		break;

	case cmd_FileImport:
		AskImport();
		break;

	case cmd_ToolbarMessagePrintBtn:
		HandlePrint();
		break;

	default:
		cmdHandled = CNavSafeDoc::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CLetterDoc::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_ToolbarFileSaveBtn:
		outEnabled = IsModified() or  not IsSpecified();
		break;

	// Import - this is actually handled by the window, but the command executed by the document
	case cmd_FileImport:
	{
		outEnabled = static_cast<CLetterWindow*>(mWindow)->GetTextDisplay()->IsTarget();
		LStr255 txt(STRx_Standards, str_ImportText);
		::PLstrcpy(outName, txt);
		break;
	}

	case cmd_Print:
	case cmd_ToolbarMessagePrintBtn:
		outEnabled = true;
		break;

	case cmd_Revert:
		// Do default only if not locked
		if (CAdminLock::sAdminLock.mNoLocalDrafts)
		{
			outEnabled = false;
			break;	// break out to prevent fall through
		}

		// Fall through to default

	default:
		CNavSafeDoc::FindCommandStatus(inCommand, outEnabled, outUsesMark,
							outMark, outName);
		break;
	}
}

void CLetterDoc::AttemptClose(Boolean inRecordIt)
{
	// Check if save locked out
	if (CAdminLock::sAdminLock.mNoLocalDrafts && IsModified())
	{
		// See whether hide is really wanted
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::Close",
																		"ErrorDialog::Btn::Cancel",
																		NULL,
																		NULL,
																		"ErrorDialog::Text::NoLocalDraftSave");
		if (result == CErrorDialog::eBtn1)
			// Force dirty off and then just close it
			SetModified(false);
		else if (result == CErrorDialog::eBtn2)
			return;
	}

	// Do default action
	CNavSafeDoc::AttemptClose(inRecordIt);
}

Boolean CLetterDoc::AttemptQuitSelf(SInt32 inSaveOption)
{
	// Check if save locked out
	if (CAdminLock::sAdminLock.mNoLocalDrafts && IsModified())
	{
		// See whether hide is really wanted
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::Close",
																		"ErrorDialog::Btn::Cancel",
																		NULL,
																		NULL,
																		"ErrorDialog::Text::NoLocalDraftSave");
		if (result == CErrorDialog::eBtn1)
			// Force dirty off and then just close it
			SetModified(false);
		else if (result == CErrorDialog::eBtn2)
			return false;
	}

	// Do default action
	return CNavSafeDoc::AttemptQuitSelf(inSaveOption);
}

PPx::CFString CLetterDoc::GetDescriptor() const
{
	// Must remove illegal file name characters
	PPx::CFString result = CNavSafeDoc::GetDescriptor();
	cdstring temp(result);
	::strreplace(temp.c_str_mod(), "/", '_');
	return PPx::CFString(temp.c_str(), kCFStringEncodingUTF8);
}

// Get name for new doc
void CLetterDoc::NameNewDoc()
{
	// Change title by appending count
	MyCFString name(mWindow->CopyCFDescriptor());
	if (CLetterWindow::sLetterWindowCount > 1)
	{
		cdstring title = name.GetString();
		title += " ";
		title += (long) CLetterWindow::sLetterWindowCount;
		MyCFString temp(title, kCFStringEncodingUTF8);
		mWindow->SetCFDescriptor(temp);
	}
}

// Open its file
void CLetterDoc::OpenFile(PPx::FSObject& inFileSpec)
{
	// NULL out for expection processing
	mFile = NULL;

	try
	{
		// Create a new File object, read the entire File contents,
		// put the contents into the text view, and set the Window
		// title to the name of the File.

		// Open file
		mFile = new LFile(inFileSpec);
		mFile->OpenDataFork(fsRdWrPerm);
		mFile->OpenResourceFork(fsRdWrPerm);

		// Read in data
		ReadEnvelope();
		ReadFile();
		ReadAttachments();

		// Read in window traits
		ReadTraits();

		// Close file
		mFile->CloseResourceFork();
		mFile->CloseDataFork();
	}
	catch (const PP_PowerPlant::LException& ex)
	{
		CLOG_LOGCATCH(PP_PowerPlant::LException);

		// Inform user
		CErrorHandler::PutOSErrAlertRsrc("Alerts::Letter::NoLetterOpen", ex.GetErrorCode());

		if (mFile)
		{
			// Clean up - will close file
			delete mFile;
			mFile = NULL;
		}

		// Must throw up if opening => total failure
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
			mFile = NULL;
		}

		// Must throw up if opening => total failure
		CLOG_LOGRETHROW;
		throw;
	}

	// Set window name and mark as specified
	mWindow->SetCFDescriptor(inFileSpec.GetName());
	mIsSpecified = true;
	SetModified(false);
}

// Read data from file
void CLetterDoc::ReadFile()
{
	Handle data = mFile->ReadDataFork();
	SInt32 length = ::GetHandleSize(data);

	// Add trailing zero for c-string
	::SetHandleSize(data, ++length);
	(*data)[length - 1] = 0;

	StHandleLocker lock(data);
	static_cast<CLetterWindow*>(mWindow)->SetText(*data);

	::DisposeHandle(data);
}

// Read envelope from file
void CLetterDoc::ReadEnvelope()
{
	try
	{
		// Get traits resource
		Handle rsrc_env = ::Get1Resource(cLetterEnvelope_Type, 128);
		ThrowIfNil_(rsrc_env);
		::HLock(rsrc_env);

		static_cast<CLetterWindow*>(mWindow)->SetEnvelope(rsrc_env, ::GetHandleSize(rsrc_env));

		::ReleaseResource(rsrc_env);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Missing resource - never mind
	}
}

// Read attachments from file
void CLetterDoc::ReadAttachments()
{
	CAttachmentList attachments;

	try
	{
		// Get attachments/encoding resource
		Handle rsrc_attachments = ::Get1Resource(cLetterAttachments_Type, 128);
		ThrowIfNil_(rsrc_attachments);

		// Read in each spec and add to list
		FSRef aSpec;
		LHandleStream data(rsrc_attachments);
		SInt32 read_len = sizeof(FSRef);
		data.GetBytes(&aSpec, read_len);

		while(read_len)
		{
			// Create new attachment and add
			CFileAttachment* aFile;
			aFile = new CFileAttachment(PPx::FSObject(aSpec));
			attachments.push_back(aFile);

			// Try to read some more
			read_len = sizeof(FSRef);
			data.GetBytes(&aSpec, read_len);
		}

		// Copy into list
		//static_cast<CLetterWindow*>(mWindow)->SetAttachList(attachments);

		// Detach prior to release
		data.DetachDataHandle();
		::ReleaseResource(rsrc_attachments);

		// Get encoding resource
		Handle rsrc_encoding = ::Get1Resource(cLetterEncoding_Type, 128);
		ThrowIfNil_(rsrc_encoding);

		// Get data from handle
		data.SetDataHandle(rsrc_encoding);

#if 0
		// Get attachments
		CAttachmentList* attachments = static_cast<CLetterWindow*>(mWindow)->GetAttachList();

		// Add all items in list
		for(CAttachmentList::iterator iter = attachments->begin(); iter != attachments->end(); iter++)
		{
			ETransferMode mode;
			data.ReadBlock(&mode, sizeof(ETransferMode));
			(*iter)->GetContent().SetTransferMode(mode);
		}
#endif

		// Detach prior to release
		data.DetachDataHandle();
		::ReleaseResource(rsrc_encoding);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Missing resource - never mind
	}
}

// Read and set bounds from file
void CLetterDoc::ReadTraits()
{
	try
	{
		// Get traits resource
		Handle rsrc_traits = ::Get1Resource(cLetterTraits_Type, 128);
		ThrowIfNil_(rsrc_traits);
		::HLock(rsrc_traits);

		static_cast<CLetterWindow*>(mWindow)->SetTraits((SLetterTraits&) **rsrc_traits);

		::ReleaseResource(rsrc_traits);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Missing resource - never mind
	}

}

// Check whether to save to mailbox or file
Boolean CLetterDoc::AskSaveAs(PPx::FSObject& outFSSpec, Boolean inRecordIt)
{
	// Look at preference option
	switch(CPreferences::sPrefs->mSaveOptions.GetValue().GetValue())
	{
	case eSaveDraftToFile:
	default:
		return AskFileSaveAs(outFSSpec, inRecordIt);

	case eSaveDraftToMailbox:
		mMailboxSave = CPreferences::sPrefs->mSaveMailbox.GetValue();
		DoMailboxSave();
		return true;

	case eSaveDraftChoose:
		{
			CSaveDraftDialog::SSaveDraft details;
			if (CSaveDraftDialog::PoseDialog(details, !CAdminLock::sAdminLock.mNoLocalDrafts))
			{
				mSaveToMailbox = !details.mFile;
				mMailboxSave = details.mMailboxName;
				if (mSaveToMailbox)
				{
					DoMailboxSave();
					return true;
				}
				else
					return AskFileSaveAs(outFSSpec, inRecordIt);
			}
			else
				return false;
		}
	}

}

// Exact copy from PP but with Navigation format popup turned off
Boolean CLetterDoc::AskFileSaveAs(PPx::FSObject& outFSSpec, Boolean inRecordIt)
{
	Boolean		saveOK = false;
	bool		replacing;

	if ( PP_StandardDialogs::AskSaveFile(GetDescriptor(),
										 GetFileType(),
										 outFSSpec,
										 replacing, kNavDefaultNavDlogOptions | kNavNoTypePopup) )
	{

		// Fix for window manager bug after a replace operation
		GetWindow()->Activate();

		if (replacing && UsesFileSpec(outFSSpec)) {
									// User chose to replace the file with
									//   one of the same name. This is the
									//   same thing as a regular save.
			if (inRecordIt) {
				SendSelfAE(kAECoreSuite, kAESave, ExecuteAE_No);
			}

			DoSave();
			saveOK = true;

		} else {

			if (inRecordIt) {
				try {
					SendAESaveAs(outFSSpec, fileType_Default, ExecuteAE_No);
				}

				catch (...) { }
			}

			if (replacing) {		// Delete existing file
				outFSSpec.Delete();
			}

			DoAESave(outFSSpec, fileType_Default);
			saveOK = true;
		}
	}

	return saveOK;
}

// Import text
void CLetterDoc::AskImport()
{
	PPx::FSObject	fspec;
	if (PP_StandardDialogs::AskOpenOneFile(0, fspec, kNavDefaultNavDlogOptions | kNavSelectAllReadableItem | kNavAllowPreviews | kNavAllFilesInPopup))
		// Send AE for recording
		DoAEImport(fspec);
}

// Import text file
void CLetterDoc::DoAEImport(PPx::FSObject &inFileSpec)
{
	try
	{
		// Make the file and open
		LFile import(inFileSpec);
		import.OpenDataFork(fsRdPerm);

		// Read in data and insert
		Handle data = import.ReadDataFork();
		{
			StHandleLocker lock(data);

			// Do line end translation
			cdstring converted(*data, ::GetHandleSize(data));
			converted.ConvertEndl();

			static_cast<CLetterWindow*>(mWindow)->GetTextDisplay()->InsertUTF8(converted);
		}
		DISPOSE_HANDLE(data);

		// Close file
		import.CloseDataFork();
	}
	catch (const PP_PowerPlant::LException& ex)
	{
		CLOG_LOGCATCH(PP_PowerPlant::LException);

		// Inform user
		CErrorHandler::PutOSErrAlertRsrc("Alerts::Letter::NoLetterImport", ex.GetErrorCode());

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}
}

// Save with AE
void CLetterDoc::DoAESave(PPx::FSObject& inFileSpec, OSType inFileType)
{
	// Kill existing file
	delete mFile;
	mFile = NULL;
	mIsSpecified = false;
	bool saving = false;

	try
	{
		// Make new file object
		mFile = new LFile(inFileSpec);

		// Make new file on disk
		mFile->CreateNewFile(kApplID, kLetterDocType, 0);
		inFileSpec.Update();

		// Write out data
		saving = true;
		DoSave();
	}
	catch (const PP_PowerPlant::LException& ex)
	{
		CLOG_LOGCATCH(PP_PowerPlant::LException);

		// Inform user if not already done in DoSave
		if (!saving)
			CErrorHandler::PutOSErrAlertRsrc("Alerts::Letter::NoLetterSave", ex.GetErrorCode());

		// Delete any saved file on disk
		if (mFile)
		{
			// Clean up - will close file
			delete mFile;
			mFile = NULL;

			// Then delete
			inFileSpec.Delete();
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
			mFile = NULL;

			// Then delete
			inFileSpec.Delete();
		}

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

	// Change window name
	mWindow->SetCFDescriptor(inFileSpec.GetName());

	// Document now has a specified file
	mIsSpecified = true;
}

// Save the drarft
void CLetterDoc::DoSave()
{
	// Look at preference option
	switch(CPreferences::sPrefs->mSaveOptions.GetValue().GetValue())
	{
	case eSaveDraftToFile:
		DoFileSave();
		break;
	case eSaveDraftToMailbox:
		mMailboxSave = CPreferences::sPrefs->mSaveMailbox.GetValue();
		DoMailboxSave();
		break;
	case eSaveDraftChoose:
		if (mSaveToMailbox)
			DoMailboxSave();
		else
			DoFileSave();
		break;
	}
}

// Save to a mailbox
void CLetterDoc::DoMailboxSave()
{
	CMbox* mbox = NULL;

	// Must have a mailbox name
	if (mMailboxSave.empty())
	{
	}	
	// Resolve mailbox name
	else if (mMailboxSave == "\1")
	{
		// Fake mailbox popup choice to do browse dialog - always return if cancelled
		const short cPopupChoose = 2;
		if (!CCopyToMenu::GetPopupMbox(false, cPopupChoose, mbox, true))
			return;
	}
	else
	{
		// Resolve mailbox name
		mbox = CMailAccountManager::sMailAccountManager->FindMboxAccount(mMailboxSave);		
	}

	// Force mailbox choice if mailbox is missing
	if (!mbox)
	{
		// Fake mailbox popup choice to do browse dialog - return if cancelled
		const short cPopupChoose = 2;
		if (!CCopyToMenu::GetPopupMbox(false, cPopupChoose, mbox, true))
			return;
	}

	// Do copy, always as draft
	static_cast<CLetterWindow*>(mWindow)->CopyNow(mbox, !CPreferences::sPrefs->mAppendDraft.GetValue());

	// Fake it as specified so that Save command is active but results in another mailbox save
	mIsSpecified = true;
}

// Save to a file
void CLetterDoc::DoFileSave(bool auto_save)
{
	try
	{
		// File must be specified
		if (!IsSpecified())
		{
		}

		cdstring text;
		static_cast<CLetterWindow*>(mWindow)->GetTaggedText(text);

		if (!text.empty())
		{
			mFile->OpenDataFork(fsRdWrPerm);
			//StHandleLocker lock(text);
			mFile->WriteDataFork(text.c_str(), text.length());
			mFile->CloseDataFork();
		}

		mFile->OpenResourceFork(fsRdWrPerm);
		SaveEnvelope();
		SaveAttachments();

		SaveTraits();

		mFile->CloseResourceFork();
	}
	catch (const PP_PowerPlant::LException& ex)
	{
		CLOG_LOGCATCH(PP_PowerPlant::LException);

		// Inform user
		CErrorHandler::PutOSErrAlertRsrc("Alerts::Letter::NoLetterSave", ex.GetErrorCode());

		// Just close file
		mFile->CloseDataFork();
		mFile->CloseResourceFork();

		// Must throw up to signal failure to others
		CLOG_LOGRETHROW;
		throw;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Just close file
		mFile->CloseDataFork();
		mFile->CloseResourceFork();

		// Must throw up to signal failure to others
		CLOG_LOGRETHROW;
		throw;
	}

	if (!auto_save)
	{
		// Mark as unmodified
		SetModified(false);
	
		// Mark as saved
		static_cast<CLetterWindow*>(mWindow)->DraftSaved();
	}
}

// Save envelope in the file
void CLetterDoc::SaveEnvelope()
{
	// Get traits
	Handle env;
	long length;
	static_cast<CLetterWindow*>(mWindow)->GetEnvelope(env, length);

	try
	{
		// Delete any existing resource
		Handle old_end = ::Get1Resource(cLetterEnvelope_Type, 128);
		if (old_end)
		{
			::RemoveResource(old_end);
			ThrowIfResError_();
			::UpdateResFile(mFile->GetResourceForkRefNum());
		}

		// Create new resource
		::AddResource(env, cLetterEnvelope_Type, 128, "\p");
		ThrowIfResError_();
		::WriteResource(env);
		ThrowIfResError_();
		::ReleaseResource(env);
		ThrowIfResError_();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Could not write resource - never mind
	}
}

// Save attachments in the file
void CLetterDoc::SaveAttachments()
{
#if 0
	// Get attachments
	CAttachmentList* attachments = static_cast<CLetterWindow*>(mWindow)->GetAttachList();

	try
	{
		// Delete any existing resource
		Handle old_attachments = ::Get1Resource(cLetterAttachments_Type, 128);
		if (old_attachments)
		{
			::RemoveResource(old_attachments);
			ThrowIfResError_();
			::UpdateResFile(mFile->GetResourceForkRefNum());
		}

		// Create new handle
		LHandleStream data;

		// Add all items in list
		for(CAttachmentList::iterator iter = attachments->begin(); iter != attachments->end(); iter++)
		{
			data.WriteBlock(&((CFileAttachment*) *iter)->GetFSSpec()->UseRef(), sizeof(FSRef));
		}

		// Detach ready to add as resource
		Handle new_attachments = data.DetachDataHandle();

		// Create new resource
		::AddResource(new_attachments, cLetterAttachments_Type, 128, "\p");
		ThrowIfResError_();
		::WriteResource(new_attachments);
		ThrowIfResError_();
		::ReleaseResource(new_attachments);
		ThrowIfResError_();

		// Delete any existing resource
		Handle old_encoding = ::Get1Resource(cLetterEncoding_Type, 128);
		if (old_encoding)
		{
			::RemoveResource(old_encoding);
			ThrowIfResError_();
			::UpdateResFile(mFile->GetResourceForkRefNum());
		}

		// Add all items in list
		for(CAttachmentList::iterator iter = attachments->begin(); iter != attachments->end(); iter++)
		{
			ETransferMode mode = (*iter)->GetTransferMode();
			data.WriteBlock(&mode, sizeof(ETransferMode));
		}

		// Detach ready to add as resource
		Handle new_encoding = data.DetachDataHandle();

		// Create new resource
		::AddResource(new_encoding, cLetterEncoding_Type, 128, "\p");
		ThrowIfResError_();
		::WriteResource(new_encoding);
		ThrowIfResError_();
		::ReleaseResource(new_encoding);
		ThrowIfResError_();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Could not write resource - never mind
	}
#endif
}

// Save traits in the file
void CLetterDoc::SaveTraits()
{
	// Get traits
	SLetterTraits traits;
	static_cast<CLetterWindow*>(mWindow)->GetTraits(traits);

	try
	{
		// Delete any existing resource
		Handle old_traits = ::Get1Resource(cLetterTraits_Type, 128);
		if (old_traits)
		{
			::RemoveResource(old_traits);
			ThrowIfResError_();
			::UpdateResFile(mFile->GetResourceForkRefNum());
		}

		// Create new handle
		Handle new_traits = ::NewHandle(sizeof(SLetterTraits));
		::HLock(new_traits);
		::BlockMoveData(&traits, *new_traits, sizeof(SLetterTraits));
		::HUnlock(new_traits);

		// Create new resource
		::AddResource(new_traits, cLetterTraits_Type, 128, "\p");
		ThrowIfResError_();
		::WriteResource(new_traits);
		ThrowIfResError_();
		::ReleaseResource(new_traits);
		ThrowIfResError_();

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Could not write resource - never mind
	}
}

// Revert the file
void CLetterDoc::DoRevert()
{
	try
	{
		// Open file
		mFile->OpenDataFork(fsRdWrPerm);
		mFile->OpenResourceFork(fsRdWrPerm);

		// Read data again from file - this creates new lists and deletes old lists
		ReadEnvelope();
		ReadFile();
		ReadAttachments();

		// Close file
		mFile->CloseDataFork();
		mFile->CloseResourceFork();

		// No longer dirty
		SetModified(false);
	}
	catch (const PP_PowerPlant::LException& ex)
	{
		CLOG_LOGCATCH(PP_PowerPlant::LException);

		// Inform user
		CErrorHandler::PutOSErrAlertRsrc("Alerts::Letter::NoLetterRevert", ex.GetErrorCode());

		// Close file
		mFile->CloseDataFork();
		mFile->CloseResourceFork();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Close file
		mFile->CloseDataFork();
		mFile->CloseResourceFork();
	}
}

// Called during idle
void CLetterDoc::SpendTime(const EventRecord &inMacEvent)
{
	// See if we are equal or greater than trigger
	if (::time(NULL) >= mAutoSaveTime + CPreferences::sPrefs->mAutoSaveDraftsInterval.GetValue())
	{
		// Do auto save
		SaveTemporary();
	}
}

// Save the file in the temporary directory
void CLetterDoc::SaveTemporary()
{
	// Only if allowed by preferences and admin
	if (CAdminLock::sAdminLock.mNoLocalDrafts || (CPreferences::sPrefs != NULL) && !CPreferences::sPrefs->mAutoSaveDrafts.GetValue())
		return;

	LFile* save = mFile;
	LFile* temp = NULL;
	PPx::FSObject newTemporary;

	try
	{
		// Find temporary folder
		MyCFString dircfstr(CConnectionManager::sConnectionManager.GetSafetySaveDraftDirectory());
		PPx::FSObject stemp(dircfstr);

		// Find unused temp name
		while(true)
		{
			cdstring tempname(cTemporaryDraftName);
			tempname += cdstring(sTemporaryCount++);
			tempname += cTemporaryDraftExtension;

			MyCFString cfstr(tempname);
			newTemporary = PPx::FSObject(stemp.UseRef(), cfstr);
			if (!newTemporary.Exists())
				break;
		}

		// Create new file
		temp = new LFile(newTemporary);
		mFile = temp;
		mFile->CreateNewFile(kApplID, kLetterDocType, 0);
		newTemporary.Update();

		DoFileSave(true);							// Write out data

		delete temp;
		temp = NULL;
		mFile = save;

		// Delete the existing file here
		DeleteTemporary();
		
		// Assign temporary file
		mTemporary = newTemporary;
		
		// Reset timer
		mAutoSaveTime = ::time(NULL);
	}
	catch (const PP_PowerPlant::LException& ex)
	{
		CLOG_LOGCATCH(PP_PowerPlant::LException);

		// Delete any saved file on disk
		if (temp)
		{
			// Clean up - will close file
			delete temp;
			temp = NULL;

			// Then delete
			if (newTemporary.Exists())
				newTemporary.Delete();
		}

		// Restore original
		mFile = save;

		// Special case - ignore locked volumes, might be running from a bootable CDROM
		if ((ex.GetErrorCode() != wPrErr) &&
			(ex.GetErrorCode() != vLckdErr))
		{
			// Inform user
			CErrorHandler::PutOSErrAlertRsrc("Alerts::Letter::NoTempLetterSave", ex.GetErrorCode());

			// Must throw up as this is fatal!
			CLOG_LOGRETHROW;
			throw;
		}
	}
}

// Delete the file in the temporary directory
void CLetterDoc::DeleteTemporary()
{
	// Only if allowed by preferences and admin
	if (CAdminLock::sAdminLock.mNoLocalDrafts || (CPreferences::sPrefs != NULL) && !CPreferences::sPrefs->mAutoSaveDrafts.GetValue())
		return;

	try
	{
		if (mTemporary.Exists())
			mTemporary.Delete();
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Read the file in the temporary directory
void CLetterDoc::ReadTemporary()
{
	// Only if allowed by preferences and admin
	if (CAdminLock::sAdminLock.mNoLocalDrafts || (CPreferences::sPrefs != NULL) && !CPreferences::sPrefs->mAutoSaveDrafts.GetValue())
		return;

	unsigned long ctr = 1;
	try
	{
		// Find temporary folder
		MyCFString dircfstr(CConnectionManager::sConnectionManager.GetSafetySaveDraftDirectory());
		PPx::FSObject temp(dircfstr);

		diriterator diter(temp.UseRef(), cTemporaryDraftExtension);
		const char* fname = NULL;
		while(diter.next(&fname))
		{
			// Create Fref for temp file
			MyCFString cfstr(fname);
			PPx::FSObject fref = PPx::FSObject(temp.UseRef(), cfstr);
			
			// Create new draft from the temp file
			MakeTemporary(fref, ctr++);
		}
	}
	catch (const PP_PowerPlant::LException& ex)
	{
		CLOG_LOGCATCH(PP_PowerPlant::LException);

		// Inform user
		CErrorHandler::PutOSErrAlertRsrc("Alerts::Letter::NoTempLetterOpen", ex.GetErrorCode());

		// Not much we can do so do not throw up
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Not much we can do so do not throw up
	}
}

// Make a draft from a temporary file spec
void CLetterDoc::MakeTemporary(PPx::FSObject& inFileSpec, unsigned long ctr)
{
	// Create new doc
	CLetterDoc* theLetterDoc = new CLetterDoc(CMulberryApp::sApp, &inFileSpec);

	// Now make sure draft nows about the current temp file
	theLetterDoc->mTemporary = inFileSpec;

	// Unspecify it and make it dirty
	theLetterDoc->mFile = NULL;
	theLetterDoc->mIsSpecified = false;
	theLetterDoc->SetModified(true);
	
	// Give window a suitable title
	cdstring tempname(cRecoveredDraftName);
	tempname += cdstring(ctr);
	MyCFString cftext(tempname.c_str(), kCFStringEncodingUTF8);
	theLetterDoc->GetWindow()->SetCFDescriptor(cftext);

	theLetterDoc->GetWindow()->Show();
}

void CLetterDoc::HandlePrint()
{
	//  Hide status window as Print Manager gets name of top window and does not know about floats
	{
		StPrintSession	session(mPrintSpec);
		StStatusWindowHide hide;

		if (UPrinting::AskPrintJob(mPrintSpec))
		{
			SendSelfAE(kCoreEventClass, kAEPrint, false);
			DoPrint();
		}
	}

	// Print job status window might mess up window order
	UDesktop::NormalizeWindowOrder();
}

// Print message
void CLetterDoc::DoPrint()
{
	LPrintout* 		thePrintout = NULL;
	CHeadAndFoot*	header = NULL;
	CHeadAndFoot*	footer = NULL;
	CMessage*		msg = NULL;

	// Prevent any drawing while printing
	StStopRedraw noDraw(static_cast<CLetterWindow*>(mWindow)->mText);

	// Save selection and remove
	CTextDisplay::StPreserveSelection _selection(static_cast<CLetterWindow*>(mWindow)->mText);
	static_cast<CLetterWindow*>(mWindow)->mText->SetSelectionRange(0, 0);

	// See if header insertion required
	cdstring header_insert;
	bool do_header_insert = CPreferences::sPrefs->mPrintSummary.GetValue();

	// See if printer font change required
	bool change_font = static_cast<CLetterWindow*>(mWindow)->mCurrentPart &&
						(static_cast<CLetterWindow*>(mWindow)->mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain);

	// See if hard wrap change required
	bool do_hard_wrap = static_cast<CLetterWindow*>(mWindow)->mText->GetHardWrap();

	try
	{
		// Create the message
		msg = static_cast<CLetterWindow*>(mWindow)->CreateMessage(false);

		// Create printout
		thePrintout = LPrintout::CreatePrintout(paneid_MessagePrintout);
		ThrowIfNil_(thePrintout);
		thePrintout->SetPrintSpec(mPrintSpec);

		// Get placeholder
		LPlaceHolder* headPlace = (LPlaceHolder*) thePrintout->FindPaneByID(paneid_MessagePrintHead);
		LPlaceHolder* textPlace = (LPlaceHolder*) thePrintout->FindPaneByID(paneid_MessagePrintArea);
		LPlaceHolder* footPlace = (LPlaceHolder*) thePrintout->FindPaneByID(paneid_MessagePrintFooter);

		// Make it size of page with a 1/2" margin all round (less header & footer size)
		Rect pageRect;
		GetPrintSpec().GetPageRect(pageRect);
		Rect paperRect;
		GetPrintSpec().GetPaperRect(paperRect);
		short iHRes;
		short iVRes;
#if PP_Target_Carbon
		PMResolution res;
		::PMGetResolution(GetPrintSpec().GetPageFormat(), &res);
		iHRes = res.hRes;
		iVRes = res.vRes;
#else
		iHRes = (**(THPrint) GetPrintSpec().GetPrintRecord()).prInfo.iHRes;
		iVRes = (**(THPrint) GetPrintSpec().GetPrintRecord()).prInfo.iVRes;
#endif

		short page_hSize = UGraphicUtils::RectWidth(pageRect) - iHRes;
		short page_vSize = UGraphicUtils::RectHeight(pageRect) - iVRes;

		// Do this here to prevent superview of text object being affected by printing
		textPlace->InstallOccupant(static_cast<CLetterWindow*>(mWindow)->mText, atNone);

		// If plain text force change in text traits
		if (change_font)
			static_cast<CLetterWindow*>(mWindow)->mText->SetFont(CPreferences::sPrefs->mPrintTextTraits.GetValue().traits);

		// Check for summary headers
		if (do_header_insert)
		{
			// Get summary from envelope
			std::ostrstream hdr;
			msg->GetEnvelope()->GetSummary(hdr);
			hdr << std::ends;
			header_insert.steal(hdr.str());

			// Parse as header
			static_cast<CLetterWindow*>(mWindow)->mText->InsertFormattedHeader(header_insert.c_str());
		}

		// Change hard wrap if needed
		if (do_hard_wrap)
			static_cast<CLetterWindow*>(mWindow)->mText->SetHardWrap(false);

		// Create header edit text and set text traits and set to full width of page
		header = new CHeadAndFoot(CPreferences::sPrefs->mHeaderBox.GetValue());
		UTextTraits::SetTETextTraits(&CPreferences::sPrefs->mCaptionTextTraits.GetValue().traits, header->GetMacTEH());
		header->ResizeFrameTo(page_hSize, page_vSize, false);

		cdstring headTxt = CCaptionParser::ParseCaption(CPreferences::sPrefs->mLtrHeaderCaption.GetValue(), msg, false);
		c2pstr(headTxt.c_str_mod());
		header->SetDescriptor(reinterpret_cast<unsigned char*>(headTxt.c_str_mod()));

		// Create footer edit text and set text traits and set to full width of page
		footer = new CHeadAndFoot(CPreferences::sPrefs->mFooterBox.GetValue());
		UTextTraits::SetTETextTraits(&CPreferences::sPrefs->mCaptionTextTraits.GetValue().traits, footer->GetMacTEH());
		footer->ResizeFrameTo(page_hSize, page_vSize, false);

		cdstring footTxt = CCaptionParser::ParseCaption(CPreferences::sPrefs->mLtrFooterCaption.GetValue(), msg, false);
		c2pstr(footTxt.c_str_mod());
		footer->SetDescriptor(reinterpret_cast<unsigned char*>(footTxt.c_str_mod()));

		// Now do resizing of place holders
		long caption_line_height = ::TEGetHeight(1, 1, header->GetMacTEH());
		long text_line_height = static_cast<CLetterWindow*>(mWindow)->mText->GetLineHeight(0);

		// Resize header
		long header_height = headTxt.length() ? caption_line_height * (**header->GetMacTEH()).nLines : 0;
		if (header_height)
		{
			if (CPreferences::sPrefs->mHeaderBox.GetValue())
				header_height += cHeadAndFootInset;
			headPlace->ResizeFrameTo(page_hSize, header_height, false);
		}
		else
			headPlace->Hide();

		// Resize footer
		long footer_height = headTxt.length() ? caption_line_height * (**footer->GetMacTEH()).nLines : 0;
		if (footer_height)
		{
			if (CPreferences::sPrefs->mFooterBox.GetValue())
				footer_height += cHeadAndFootInset;
			footPlace->ResizeFrameTo(page_hSize, footer_height, false);
		}
		else
			footPlace->Hide();

		// Resize text
		long text_height = page_vSize - (header_height + footer_height + (header_height ? caption_line_height : 0) + (footer_height ? caption_line_height : 0));
		text_height -= (text_height % text_line_height);
		textPlace->ResizeFrameTo(page_hSize, text_height, false);

		// Set origin 1/2" in from corner
		short hOrigin = -paperRect.left + iHRes/2;
		short vOrigin = -paperRect.top + iVRes/2;
		headPlace->PlaceInSuperFrameAt(hOrigin, vOrigin, false);
		textPlace->PlaceInSuperFrameAt(hOrigin, vOrigin + header_height + (header_height ? caption_line_height : 0), false);
		footPlace->PlaceInSuperFrameAt(hOrigin, vOrigin + header_height + (header_height ? caption_line_height : 0) +
															text_height + (footer_height ? caption_line_height : 0), false);

		headPlace->InstallOccupant(header, atNone);
		//textPlace->InstallOccupant(static_cast<CLetterWindow*>(mWindow)->mText, atNone);
		footPlace->InstallOccupant(footer, atNone);

		// Dump message
		delete msg;

		// Do print job
		thePrintout->DoPrintJob();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		delete msg;
	}

	// Remove summary headers
	if (do_header_insert)
	{
		cdustring temp(header_insert);
		static_cast<CLetterWindow*>(mWindow)->mText->SetSelectionRange(0, temp.length());
		static_cast<CLetterWindow*>(mWindow)->mText->InsertUTF8(cdstring::null_str);
	}

	// If plain text force change in text traits
	if (change_font)
	{
		static_cast<CLetterWindow*>(mWindow)->mText->FocusDraw();
		static_cast<CLetterWindow*>(mWindow)->mText->SetFont(CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits);
	}

	// Change hard wrap if needed
	if (do_hard_wrap)
		static_cast<CLetterWindow*>(mWindow)->mText->SetHardWrap(true);

	// Kill footer & printout/placeholder
	delete thePrintout;
	delete header;			// <- by some odd accident this also results in footer being deleted!!??
	//delete footer;

	// Force text refresh
	static_cast<CLetterWindow*>(mWindow)->mText->FocusDraw();
	//static_cast<CLetterWindow*>(mWindow)->mText->AdjustImageToText();
	static_cast<CLetterWindow*>(mWindow)->mText->Refresh();
}
