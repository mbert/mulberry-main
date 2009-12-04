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


//	CFileAttachment.cp

#include "CFileAttachment.h"

#include "CAFFilter.h"
#include "CAttachmentList.h"
#include "CAttachmentManager.h"
#include "CFilter.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CICSupport.h"
#include "CLocalCommon.h"
#endif
#include "CMIMESupport.h"
#include "CPreferences.h"
#include "CProgress.h"
#include "CStreamType.h"
#include "CUnicodeStdLib.h"
#include "CUtils.h"

#if __dest_os == __win32_os
#include <WIN_LDataStream.h>
#include <WIN_LFileStream.h>
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <Finder.h>
//#include "path2fss.h"
#include "MyCFString.h"
#endif

#if __framework == __jx
#include <jDirUtil.h>
#include <jFileUtil.h>
#include <JString.h>
#endif

#include <stdio.h>
#include <string.h>

// Default constructor
CFileAttachment::CFileAttachment()
{
	// Init to default values
	InitFileAttachment();
}

// Construct from path
CFileAttachment::CFileAttachment(const cdstring& fpath)
{
	// Init to default values
	InitFileAttachment();

	// Set FSSpec only if it exists
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	MyCFString cfstr(fpath);
	PPx::FSObject spec(cfstr);
	SetFSSpec(spec);
#elif __dest_os == __win32_os
	// Set File path only if it exists
	CFileStatus fstat;
	if (CFile::GetStatus(fpath.win_str(), fstat))
		SetFilePath(fpath.win_str());
#else
	if (JFileExists(fpath)) {
		SetFilePath(fpath);
	}		
#endif
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
// Construct from FSSpec
CFileAttachment::CFileAttachment(const PPx::FSObject& spec)
{
	// Init to default values
	InitFileAttachment();

	// Set HFSFlavor
	SetFSSpec(spec);
}
#endif

// Copy constructor
CFileAttachment::CFileAttachment(const CFileAttachment& copy)
	: CAttachment(copy)
{
	// Init to default values
	InitFileAttachment();
	mFileSpec = copy.mFileSpec;

	mContent.SetContentDisposition(copy.GetContent().GetContentDisposition());
}

// Destructor
CFileAttachment::~CFileAttachment()
{
	// See if file has to be deleted (will only be required for temp files)
	if (IsDeleteFile())
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		mFileSpec.Delete();
#else
		::remove_utf8(GetFilePath());
#endif
}

// Common init
void CFileAttachment::InitFileAttachment()
{
	// Init to default values

	mContent.SetContentDisposition(eContentDispositionAttachment);
}

// Compare with same type
int CFileAttachment::operator==(const CFileAttachment& test) const
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Compare FSSpec's
	return mFileSpec.IsEqualTo(test.mFileSpec);
#else
	// Compare file paths
	return (mFileSpec == test.mFileSpec);
#endif
}

// Process files for sending
void CFileAttachment::ProcessSend()
{
	// If multipart, iterate over parts (not if already AppleDouble)
	if (mParts && !IsApplefile())
	{
		// Iterate over each part
		for(CAttachmentList::iterator iter = mParts->begin(); iter != mParts->end(); iter++)

			// Convert each sub-part
			(*iter)->ProcessSend();

	}

	// Check for AppleDouble encoding
	else if (mContent.GetTransferMode() == eAppleDoubleMode)
	{
		// Duplicate this part twice
		CAttachment* mSinglePart = CopyAttachment(*this);
		CAttachment* mDataPart = CopyAttachment(*this);

		// Set appropriate mode for each
		mSinglePart->GetContent().SetTransferMode(eAppleSingleMode);
		mDataPart->GetContent().SetTransferMode(eMIMEMode);

		// Recalculate mapping for data without resource fork
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		CICSupport::ICMapTypeCreator(*(CFileAttachment*) mDataPart, false);
#elif __dest_os == __win32_os || __dest_os == __linux_os
		CMIMESupport::MapFileToMIME(*(CFileAttachment*) mDataPart);
#else
#error __dest_os
#endif
		// Cannot recode an AppleDouble!
		if (mDataPart->IsApplefile() || mDataPart->GetContent().IsBinHexed() || mDataPart->GetContent().IsUUed())
		{
			mDataPart->GetContent().SetContent(eContentApplication, eContentSubOctetStream);
			mDataPart->GetContent().SetTransferMode(eMIMEMode);
		}
		mDataPart->ProcessContent();


		// Make this into multipart
		mContent.SetContent(eContentMultipart, eContentSubAppleDouble);

		// Add sub-parts
		AddPart(mSinglePart);
		AddPart(mDataPart);

	}
	else
		ProcessContent();
}

void CFileAttachment::ProcessContent()
{
	// Process text to determine encoding and charset
	if (IsText() && !IsApplefile() && !GetContent().IsBinHexed())
	{
		EContentTransferEncoding encoding;
		bool non_ascii = false;


		{
			// Determine encoding from file data
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			LFileStream stream(mFileSpec);
#elif __dest_os == __win32_os
			LFileStream stream(GetFilePath().win_str(), CFile::modeRead | CFile::shareDenyNone);
#elif __dest_os == __linux_os
			LFileStream stream(GetFilePath(), O_RDONLY);
#else
#error __dest_os
#endif
			non_ascii = CMIMESupport::DetermineTextEncoding(stream, mContent.GetContentDisposition()) != e7bitEncoding;
		}

		if((mContent.GetContentSubtype() == eContentSubEnriched) || (mContent.GetContentSubtype() == eContentSubHTML))
			// Always quoted-printable
			encoding = eQuotedPrintableEncoding;
		else
		{
			encoding = non_ascii ? eQuotedPrintableEncoding : e7bitEncoding;
			
			// Inline text parts use format=flowed
			if (CPreferences::sPrefs->mFormatFlowed.GetValue() &&
				(mContent.GetContentDisposition() == eContentDispositionInline) &&
				!mContent.IsFlowed())
				mContent.SetContentParameter(cMIMEParameter[eFormat], cMIMEParameter[eFlowed]);
		}

		mContent.SetTransferEncoding(encoding);
		mContent.SetTransferMode(eMIMEMode);

		// Set charset: use US Ascii if no chars > 0x7F
		if (!non_ascii)
			// Use US Ascii if no chars > 0x7F
			mContent.SetCharset(i18n::eUSASCII);
		else
		{
			// Force to default if not already set or set to US-ASCII
			switch(mContent.GetCharset())
			{
			case i18n::eAutomatic:
			case i18n::eUSASCII:
				mContent.SetCharset(non_ascii ? i18n::eUTF8 : i18n::eUSASCII);
				break;
			default:;	// Must be set by user
			}
		}
	}
	else
		// Always have US-ASCII
		mContent.SetCharset(i18n::eUSASCII);

	// Do not do any more
}

// View the attachment
void CFileAttachment::ViewFile() const
{
	CAttachmentManager::sAttachmentManager.ViewFile(this);
}

// Set from FSSpec
#if __dest_os == __mac_os || __dest_os == __mac_os_x
void CFileAttachment::SetFSSpec(const PPx::FSObject& spec)
{
	// Set spec
	mFileSpec = spec;

	// Get finder info
	try
	{
		MyCFString cfstr(mFileSpec.GetName());

		SetName(cfstr.GetString());
		mContent.SetMappedName(mName);

		// Check for aliases & directories
		ResolveIfAlias();
		CheckForDirectory();

		// Calculate its size and determine mapping
		CalculateFileSize();
		MIMEMap();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Null out fields
		PPx::FSObject temp;
		mFileSpec = temp;
	}
}

// Check hierarchy for unique file
bool CFileAttachment::UniqueFile(const PPx::FSObject& spec) const
{
	if (mParts)
	{
		// For all sub-parts
		for(CAttachmentList::const_iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			if (!(*iter)->UniqueFile(spec))
				return false;
		}

		// No match found
		return true;
	}
	else
		return !mFileSpec.IsEqualTo(spec);
}

#elif __dest_os == __win32_os

void CFileAttachment::SetFilePath(const cdstring& fpath)
{
	// Set spec
	mFileSpec = fpath;

	// Get finder info
	try
	{
		// Force to application/octet-stream for now
		GetContent().SetContent(eContentApplication, eContentSubOctetStream);

		TCHAR buf[256];
		::AfxGetFileName(mFileSpec.win_str(), buf, 256);
		SetName(cdstring(buf));
		mContent.SetMappedName(mName);

		// Check for aliases & directories
		ResolveIfAlias();
		CheckForDirectory();

		// Calculate its size and determine mapping
		CalculateFileSize();
		MIMEMap();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Check hierarchy for unique path
bool CFileAttachment::UniqueFile(const cdstring& fpath) const
{
	if (mParts)
	{
		// For all sub-parts
		for(CAttachmentList::const_iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			if (!(*iter)->UniqueFile(fpath))
				return false;
		}

		// No match found
		return true;
	}
	else
		return (fpath != mFileSpec);
}
#elif __dest_os == __linux_os
void CFileAttachment::SetFilePath(const cdstring& fpath)
{
	// Set spec
	mFileSpec = fpath;

	// Get finder info
	try
	{
		// Force to application/octet-stream for now
		GetContent().SetContent(eContentApplication, eContentSubOctetStream);

		JString path, fname;
		JSplitPathAndName(mFileSpec, &path, &fname);
		SetName(fname);
		mContent.SetMappedName(mName);

		// Check for aliases & directories
		ResolveIfAlias();
		CheckForDirectory();

		// Calculate its size and determine mapping
		CalculateFileSize();
		MIMEMap();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Check hierarchy for unique path
bool CFileAttachment::UniqueFile(const cdstring& fpath) const
{
	if (mParts)
	{
		// For all sub-parts
		for(CAttachmentList::const_iterator iter = mParts->begin(); iter != mParts->end(); iter++)
		{
			if (!(*iter)->UniqueFile(fpath))
				return false;
		}

		// No match found
		return true;
	}
	else
		return (fpath != mFileSpec);
}
#endif

// Check for valid file
bool CFileAttachment::ValidFile() const
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Look at each bit of FSSpec
	return mFileSpec.IsValid();
#else
	// Must have something in the path
	return !mFileSpec.empty();
#endif
}

// Get suite of icons from icon cache
const CIconRef* CFileAttachment::GetIconRef() const
{
	if (IsDirectory())
		return CDesktopIcons::GetFolderIcons();
	else
	{
		return mFileIcon.LoadIcons(mFileSpec);
	}
}

// Calculate size of file from spec
void CFileAttachment::CalculateFileSize()
{
	if (IsDirectory()) return;

	try
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		FSCatalogInfo catInfo;
		mFileSpec.GetCatalogInfo(kFSCatInfoDataSizes | kFSCatInfoRsrcSizes, catInfo);

		mContent.SetContentSize(catInfo.dataLogicalSize + catInfo.rsrcLogicalSize);
#elif __dest_os == __win32_os
		CFileStatus status;
		CFile::GetStatus(mFileSpec.win_str(), status);
		mContent.SetContentSize(status.m_size);
#else
		struct stat finfo;
		if (!::stat_utf8(mFileSpec, &finfo))
			mContent.SetContentSize(finfo.st_size);
#endif
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Check whether specified file is a directory
void CFileAttachment::CheckForDirectory()
{
	try
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		mFlags.Set(eDirectory, mFileSpec.IsFolder());
#elif __dest_os == __win32_os
		CFileStatus status;
		CFile::GetStatus(mFileSpec.win_str(), status);
		mFlags.Set(eDirectory, status.m_attribute & CFile::directory);
#else
		struct stat finfo;
		if (!::stat_utf8(mFileSpec, &finfo))
			mFlags.Set(eDirectory, finfo.st_mode & S_IFDIR);
#endif
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

}

// Resolve an alias file/directory
void CFileAttachment::ResolveIfAlias()
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// See if it is an alias
	FinderInfo finfo;
	mFileSpec.GetFinderInfo(&finfo);

	if (finfo.file.finderFlags & kIsAlias)
	{
		FSRef resolved = mFileSpec.UseRef();
		Boolean wasAliased;
		Boolean is_dir = IsDirectory();
		ThrowIfOSErr_(::FSResolveAliasFile(&resolved, true, &is_dir, &wasAliased));
		mFlags.Set(eDirectory, is_dir);

		PPx::FSObject temp(resolved);
		SetFSSpec(temp);
	}
#else
	// Assume others will resolve shortcut
#endif
}

// Set default transfer mode based on file type
void CFileAttachment::MIMEMap()
{
	if (IsDirectory())
	{
		// Directories are not yet handled => no mode
		mContent.SetTransferMode(eNoTransferMode);
	}
	else
	{

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Do file mapping info now
		CICSupport::ICMapTypeCreator(*this, true);
#elif __dest_os == __win32_os || __dest_os == __linux_os
		CMIMESupport::MapFileToMIME(*this);
#else
#error __dest_os
#endif
	}
}

// Write file to stream
void CFileAttachment::WriteDataToStream(costream& stream, bool dummy_files, CProgress* progress, CMessage* owner,
										unsigned long count, unsigned long start) const
{
	// Bump progress counter
	if (progress)
		progress->BumpCount();

	// Only do this if cached
	if (IsNotCached())
		return;

	// Use dummy text for files?
	if (!dummy_files)
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		CFullFileStream theFile(mFileSpec);
		theFile.OpenDataFork(fsRdPerm);

		if (mContent.IsBinHexed() || IsApplefile())
			theFile.OpenResourceFork(fsRdPerm);
#elif __dest_os == __win32_os
		CFullFileStream theFile(mFileSpec.win_str(), CFile::modeRead | CFile::shareDenyNone);
#elif __dest_os == __linux_os
		CFullFileStream theFile(mFileSpec, O_RDONLY);
#else
#error __dest_os
#endif

		CFilter* aFilter = CMIMESupport::GetFilter(this, false);
		CAFFilter* aAFFilter = NULL;

		if (IsApplefile())
		{
			aAFFilter = new CAFFilter();
			aFilter->SetStream(aAFFilter);
			aAFFilter->SetStream(&theFile, !mParent || !mParent->IsApplefile());
		}
		else
			aFilter->SetStream(&theFile);
		aFilter->SetProgress(progress);
		aFilter->SetForNetwork(stream.GetEndlType());

		{
			char* buffer = new char[8192];
			while(!aFilter->Complete())
			{
				long read = 8192;
				aFilter->GetBytes(buffer, read);
				stream.Stream().write(buffer, read);
			}
			delete buffer;
		}
		delete aFilter;
		delete aAFFilter;
	}
	else
	{
		// Send attachment name
		stream.Stream() << stream.endl() << "Attachment sent: \"";
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		stream.Stream() << ::GetFullPath(&mFileSpec.UseRef());
#else
		stream.Stream() << mFileSpec.c_str();
#endif
		stream.Stream() << "\""  << stream.endl() << stream.endl();
	}
}
