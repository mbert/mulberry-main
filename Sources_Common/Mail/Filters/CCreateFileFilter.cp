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


// Source for CCreateFileFilter class

#include "CCreateFileFilter.h"

#include "CAttachment.h"
#include "CBinHexFilter.h"
#include "CErrorHandler.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMessageWindow.h"
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "MyCFString.h"

#include <UStandardDialogs.h>
#endif

#if __dest_os == __win32_os
#include <winsock.h>
#endif

#if __dest_os == __linux_os
#include <jFileUtil.h>
#endif

// Constants

// __________________________________________________________________________________________________
// C L A S S __ C B I N H E X F I L T E R
// __________________________________________________________________________________________________

// Constructor
CCreateFileFilter::CCreateFileFilter(LStream* aStream, CProgress* progress) : CFilter(aStream, progress)
{
	InitCreateFileFilter();

	// Store cast local copy
	mFileStream = (CFullFileStream*) aStream;
}

CCreateFileFilter::~CCreateFileFilter()
{
	mFileStream = NULL;
}

void CCreateFileFilter::InitCreateFileFilter()
{
	// Init params
	mFileStream = NULL;
	mFileCreated = false;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mUseRsrc = false;
#endif
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Create file for decode
void CCreateFileFilter::CreateFile()
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	bool				done = false;
	PPx::FSObject		fspec;

	// Copy name from spec
	MyCFString cfstr(mDecodedName, kCFStringEncodingUTF8);

	// Do standard save as dialog with directory Adjustment if required
	bool replacing = false;
	if (CAttachment::sDropLocation != NULL)
	{
		done = true;
		fspec = PPx::FSObject(*CAttachment::sDropLocation, cfstr);
		replacing = fspec.Exists();
	}
	else if (mAutoCreate)
	{
		done = PP_StandardDialogs::AskSaveFile(cfstr, mDecodedInfo.file.fileType, fspec, replacing, kNavDefaultNavDlogOptions | kNavNoTypePopup);
	}
	else
	{
		done = true;
		mFileStream->GetSpecifier(fspec);
	}

	// Check for good reply
	if (done)
	{
		// Delete any existing file
		if (replacing)
			fspec.Delete();

		// Specify it
		if (mAutoCreate)
			mFileStream->SetSpecifier(fspec);

		// Create a new file of the required type & creator with/without resource fork
		if (mUseRsrc)
			mFileStream->CreateNewFile(mDecodedInfo.file.fileCreator, mDecodedInfo.file.fileType, smCurrentScript);
		else
			mFileStream->CreateNewDataFile(mDecodedInfo.file.fileCreator, mDecodedInfo.file.fileType, smCurrentScript);
		fspec.Update();

		// Set Finder flags (get them first)
		FinderInfo finfo;
		fspec.GetFinderInfo(&finfo, NULL, NULL);
		finfo.file.finderFlags = mDecodedInfo.file.finderFlags & ~(kIsOnDesk | kIsInvisible | kHasBeenInited);
		fspec.SetFinderInfo(&finfo);

		mFileCreated = true;
	}
	else
		mFileCreated = false;
#else
	cdstring name = mDecodedName;
	cdstring fpath = mFileStream->GetFilePath();
	if (mAutoCreate && CMIMESupport::MapToFile(name, fpath))
	{
#if __dest_os == __win32_os
		// Open file object and specify it
		CFileException ex;
		if ((mFileCreated = mFileStream->Open(fpath.win_str(), CFile::modeCreate | CFile::modeWrite, &ex)) == false)
		{
			CErrorHandler::PutFileErrAlertRsrc("Alerts::Attachments::FilterNoFile", ex);
			mFileStream->Abort();
		}
#elif __dest_os == __linux_os
		try
		{
			mFileStream->Open(fpath, O_WRONLY | O_CREAT);
			mFileCreated = true;
		}
		catch (CFileException& ex)
		{
			CLOG_LOGCATCH(CFileException&);

			CErrorHandler::PutFileErrAlertRsrc("Alerts::Attachments::FilterNoFile", ex);
			mFileStream->Abort();
			mFileCreated = false;
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			mFileCreated = false;
		}
#endif
	}
	else
		mFileCreated = false;
#endif
}
