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


// CAttachmentManager.h

// Class to handle all actions related to local & report address books,
// including opening/closing, nick-name resolution, searching etc

#include "CAttachmentManager.h"

#include "CAdminLock.h"
#include "CAFFilter.h"
#include "CAttachment.h"
#include "CAttachmentList.h"
#include "CBinHexFilter.h"
#include "CConnectionManager.h"
#include "CErrorHandler.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CAppLaunch.h"
#include "CDesktopIcons.h"
#endif
#include "CFileAttachment.h"
#include "CGeneralException.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CICSupport.h"
#endif
#include "CITIPProcessor.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMessageWindow.h"
#endif
#include "CMessage.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CStringUtils.h"
#include "CUnicodeStdLib.h"
#include "CUUFilter.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "MyCFString.h"
#include "FullPath.h"
#include <UStandardDialogs.h>
#elif __dest_os == __win32_os
#include "CSDIFrame.h"
#elif __dest_os == __linux_os
#include <jGlobals.h>
#include <jFileUtil.h>
#include <jDirUtil.h>
#include <jProcessUtil.h>
#include "CMailcapMap.h"
#include "CMIMETypesMap.h"
#endif

CAttachmentManager CAttachmentManager::sAttachmentManager;

// Try to quit
bool CAttachmentManager::CanQuit(cdstrvect& still_open)
{
	// Try to delete each temporary (view) attachment
	CAttachmentFileMap temp;
	
	for(CAttachmentFileMap::iterator iter = mMap.begin(); iter != mMap.end(); iter++)
	{
		// Try to remove its file
		bool did_delete = false;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		try
		{
			(*iter).second.mSpec.Delete();
			did_delete = true;
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);

		}
#else
		did_delete = !::remove_utf8((*iter).second.mSpec);
#endif
		// If not deleted copy it to temp list
		if (!did_delete)
		{
			temp.insert(*iter);
			
			// Copy the file name into the still open list
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			still_open.push_back(cdstring((*iter).second.mSpec.GetName()));
#else
			const char* fname = ::strrchr((*iter).second.mSpec.c_str(), os_dir_delim);
			if (!fname)
				fname = (*iter).second.mSpec.c_str();
			still_open.push_back(fname);
#endif
		}
	}
	
	// Replace exisiting with temp
	mMap.clear();
	mMap = temp;
	
	return !mMap.size();
}

bool CAttachmentManager::Exists(const cdstring& url) const
{
	CAttachmentFileMap::const_iterator found = mMap.find(url);
	return (found != mMap.end());
}

CAttachmentFileMap::const_iterator CAttachmentManager::GetEntry(const cdstring& url) const
{
	return mMap.find(url);
}

void CAttachmentManager::RemoveEntry(const cdstring& url)
{
	mMap.erase(url);
}

// See if this attachment should be handled internally by Mulberry
bool CAttachmentManager::HandleInternal(CMessage* owner, CAttachment* attach) const
{
	// Currently only text/calendar but don't allow this if admin locks it out
	if ((attach->GetContent().GetContentType() == eContentText) &&
		(attach->GetContent().GetContentSubtype() == eContentSubCalendar) &&
		CPreferences::sPrefs->mHandleICS.GetValue() &&
		!CAdminLock::sAdminLock.mPreventCalendars)
	{
		// Process the text/calendar object
		iCal::CITIPProcessor::ProcessAttachment(attach->ReadPart(owner), owner);
		return true;
	}
	
	return false;
}

// View an attachment
bool CAttachmentManager::ViewAttachment(CMessage* owner, CAttachment* attach)
{
	// See if the part should be handled internally
	if (HandleInternal(owner, attach))
		return true;

	// Get attachment URL
	cdstring url = owner->GetURL();
	url += attach->GetURL();
	
	// See if we already have a temp copy of it
	if (!Exists(url))
	{
		// Do extraction to temp file and record it
		if (!ExtractAttachment(owner, attach, false, true))
			return false;
	}

	// Just launch/relaunch it
	return LaunchURL(url);
}

// Extract an attachment
bool CAttachmentManager::ExtractAttachment(CMessage* owner, CAttachment* attach, bool launch_app, bool view)
{
	// See if the part should be handled internally
	if (HandleInternal(owner, attach))
		return true;

	// If not viewing check to see whether previous view already done
	if (!view)
	{
		// Get attachment URL
		cdstring url = owner->GetURL();
		url += attach->GetURL();
		
		// See if we already have a temp copy of it
		if (Exists(url))
		{
			// Get name buried in AppleDouble first part if required
			cdstring fname;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			fname = attach->GetMappedName(true, false);
#else
			fname = attach->GetMappedName(true, true);
#endif
			// Get new location and name from user
			fspectype new_fspec;
			if (!MapToFile(fname, new_fspec, false))
				return false;
			
			// Get old file
			const fspectype& old_fspec = GetEntry(url)->second.mSpec;
			
			// Rename old to new
			if (!::moverename_file(old_fspec, new_fspec))
			{
				// Remove url map for old spec
				RemoveEntry(url);
				
				// Do new launch if required
				if (launch_app)
					TryLaunch(attach, new_fspec);
				
				// All done
				return true;
			}
		}
		
		// Fall through to save it again
	}

	// Handle apple double as special case
	if (attach->GetParts() && attach->IsApplefile())
		return ExtractADAttachment(owner, attach, launch_app, view);
	else
		return ExtractSingleAttachment(owner, attach, launch_app, view);
}

// Extract AppleDouble
bool CAttachmentManager::ExtractADAttachment(CMessage* owner, CAttachment* attach, bool launch_app, bool view)
{
	CFullFileStream* aFile = NULL;
	CFilter* aFilter = NULL;
	CAFFilter* aAFFilter = NULL;

	// Check validity of this part
	if (attach->GetParts()->size() != 2)
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}

	CAttachment* applefile = attach->GetParts()->at(0);
	CAttachment* datafile = attach->GetParts()->at(1);

	if ((applefile->GetContent().GetContentType() != eContentApplication) ||
		(applefile->GetContent().GetContentSubtype() != eContentSubApplefile))
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}

	try
	{
		// Create full file stream
		aFile = (CFullFileStream*) GetFileStream(*attach, view);

		// Might have been cancelled
		if (aFile)
		{
			// Do application/applefile part first

			// Get appropriate filter for decoding
			aFilter = CMIMESupport::GetFilter(applefile, true);

			// Get filter for applefile
			aAFFilter = new CAFFilter;

			// Set filters
			aFilter->SetStream(aAFFilter);
			aAFFilter->SetStream(aFile, false);
			aFilter->SetForNetwork(lendl);

			// If viewing the file is already specified and the filter musn't auto create
			aFilter->SetAutoCreate(!view);

			// Tell mbox to read this part
			applefile->ReadAttachment(owner, aFilter);

			applefile->SetExtracted(true);

			delete aFilter;
			aFilter = NULL;
			delete aAFFilter;
			aAFFilter = NULL;

			// Do data next

			// Get appropriate filter for encoding
			aFilter = CMIMESupport::GetFilter(datafile, true);

			// Set filters
			aFilter->SetStream(aFile);
			aFilter->SetForNetwork(lendl);

			// If viewing the file is already specified and the filter musn't auto create
			aFilter->SetAutoCreate(!view);

			// Tell mbox to read this part
			datafile->ReadAttachment(owner, aFilter);

			datafile->SetExtracted(true);

			attach->SetExtracted(true);

			// If viewing add to view map
			if (view)
			{
				// Get URL
				cdstring url = owner->GetURL();
				url += attach->GetURL();

				// Get file spec
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				PPx::FSObject spec;
				aFile->GetSpecifier(spec);
#else
				cdstring spec = aFile->GetFilePath();
#endif				

				// Get MIME info
				cdstring mime = CMIMESupport::GenerateContentHeader(attach, false, lendl, false);

				mMap.insert(CAttachmentFileMap::value_type(url, SAttachmentDesc(spec, mime)));
			}
			else
			{
				// Try to launch it
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				if (CAttachment::sDropLocation == NULL)
#endif
					TryLaunch(attach, aFile);
			}
		}

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up

		// Delete existing file
		if (aFile)
		{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			PPx::FSObject spec;
			aFile->GetSpecifier(spec);
			spec.Delete();
#elif __dest_os == __win32_os
			CString fpath = aFile->GetFilePath();	// Cache path as Abort empties it
			aFile->Abort();
			CFile::Remove(fpath);
#elif __dest_os == __linux_os
			cdstring fpath = aFile->GetFilePath();
			aFile->Abort();
			JRemoveFile(fpath.c_str());
#endif
		}
		delete aFile;
		delete aFilter;
		delete aAFFilter;
		CLOG_LOGRETHROW;
		throw;
	}

	// Clean up
	delete aFile;
	delete aFilter;
	delete aAFFilter;
	
	return true;
}

bool CAttachmentManager::ExtractSingleAttachment(CMessage* owner, CAttachment* attach, bool launch_app, bool view)
{
	// Handle single part
	CFullFileStream* aFile = NULL;
	CFilter* aFilter = NULL;
	CFilter* aAuxFilter = NULL;

	try
	{
		// Create full file stream
		aFile = (CFullFileStream*) GetFileStream(*attach, view);

		// Might have been cancelled
		if (aFile)
		{
			// Get appropriate filter
			aFilter = CMIMESupport::GetFilter(attach, true);

			// Set filter chain for more content decoding
			
			// Check for applefile
			if (attach->IsApplefile())
			{
				aAuxFilter = new CAFFilter;
				aFilter->SetStream(aAuxFilter);
				static_cast<CAFFilter*>(aAuxFilter)->SetStream(aFile, true);
			}

			// Check for binhex first
			else if (attach->GetContent().IsBinHexed())
			{
				// Create a binhex filter
				aAuxFilter = new CBinHexFilter;
				aFilter->SetStream(aAuxFilter);
				aAuxFilter->SetStream(aFile);
			}

			// Check for uu next
			else if (attach->GetContent().IsUUed())
			{
				// Create a UU filter
				aAuxFilter = new CUUFilter;
				aFilter->SetStream(aAuxFilter);
				aAuxFilter->SetStream(aFile);
			}
			else
				aFilter->SetStream(aFile);
			aFilter->SetForNetwork(lendl);

			// If viewing the file is already specified and the filter musn't auto create
			aFilter->SetAutoCreate(!view);

			// Tell mbox to read this part
			attach->ReadAttachment(owner, aFilter);

			attach->SetExtracted(true);

			// If viewing add to view map
			if (view)
			{
				// Get URL
				cdstring url = owner->GetURL();
				url += attach->GetURL();

				// Get file spec
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				PPx::FSObject spec;
				aFile->GetSpecifier(spec);
#else
				cdstring spec = aFile->GetFilePath();
#endif				

				// Get MIME info
				cdstring mime = CMIMESupport::GenerateContentHeader(attach, false, lendl, false);

				mMap.insert(CAttachmentFileMap::value_type(url, SAttachmentDesc(spec, mime)));
			}
			else
			{
				// Try to launch it
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				if (CAttachment::sDropLocation == NULL)
#endif
					TryLaunch(attach, aFile);
			}
		}

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up

		// Delete existing file
		if (aFile)
		{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			PPx::FSObject spec;
			aFile->GetSpecifier(spec);
			spec.Delete();
#elif __dest_os == __win32_os
			CString fpath = aFile->GetFilePath();	// Cache path as Abort empties it
			aFile->Abort();
			CFile::Remove(fpath);
#elif __dest_os == __linux_os
			cdstring fpath = aFile->GetFilePath();
			aFile->Abort();
			JRemoveFile(fpath.c_str());
#else
#error __dest_os
#endif
		}
		delete aFile;
		delete aFilter;
		delete aAuxFilter;
		CLOG_LOGRETHROW;
		throw;
	}

	// Clean up
	delete aFile;
	delete aFilter;
	delete aAuxFilter;
	
	return true;
}

// Get a file stream for an encoding type
LStream* CAttachmentManager::GetFileStream(CAttachment& attach, bool view)
{
	CFullFileStream* aFile = NULL;

	// Look for BinHex or UU which will handle the file itself
	if (!view && (attach.GetContent().IsBinHexed() || attach.GetContent().IsUUed()))
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		PPx::FSObject aSpec;

		// Just create new stream
		aFile = new CFullFileStream(aSpec);
#elif __dest_os == __win32_os || __dest_os == __linux_os
		// Just create new empty stream
		aFile = new CFullFileStream();
#else
#error __dest_os
#endif

	}

	else
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Get file spec from user
		PPx::FSObject file;

		// Get name buried in AppleDouble first part if required
		cdstring fname;
		if (attach.IsApplefile() && attach.IsMultipart())
			fname = attach.GetMappedName(true, false);
		else
			fname = attach.GetMappedName(true, false);

		if (MapToFile(fname, file, view))
		{
			// Create new stream
			aFile = new CFullFileStream(file);

			// BinHex & UU will do there own thing
			if (!attach.GetContent().IsBinHexed() && !attach.GetContent().IsUUed())
			{
				// Give this FSSpec to attachment
				HFSFlavor newFlavor;
				newFlavor.fileType = '****';
				newFlavor.fileCreator = '****';
				newFlavor.fdFlags = 0;

				// Create data/resource forks
				try
				{
					switch(attach.GetContent().GetTransferMode())
					{
					case eBinHex4Mode:
						// Binhex filter will create file
						break;

					case eAppleSingleMode:
					case eAppleDoubleMode:
						// No filter - copy as is
						ICMapEntry entry;
						CICSupport::ICMapFileName(attach.GetContent(), entry);
						aFile->CreateNewFile(entry.fileCreator, entry.fileType, smCurrentScript);
						newFlavor.fileType = entry.fileType;
						newFlavor.fileCreator = entry.fileCreator;
						newFlavor.fdFlags = 0;
						break;

					case eNoTransferMode:
					case eTextMode:
					case eMIMEMode:
					default:
						switch(attach.GetContent().GetTransferEncoding())
						{
						case eNoTransferEncoding:
						case e7bitEncoding:
						case eQuotedPrintableEncoding:
						case eBase64Encoding:
						case e8bitEncoding:
						case eBinaryEncoding:
						case eXtokenEncoding:
							// No filter - copy as is
							ICMapEntry entry;
							CICSupport::ICMapFileName(attach.GetContent(), entry);
							aFile->CreateNewDataFile(entry.fileCreator, entry.fileType, smCurrentScript);
							newFlavor.fileType = entry.fileType;
							newFlavor.fileCreator = entry.fileCreator;
							newFlavor.fdFlags = 0;
							break;

						}
						break;

					}

					//attach.SetHFSFlavor(newFlavor);
					
					// Now open it here - Win32/unix always open before leaving this method
					aFile->OpenDataFork(fsWrPerm);
					if (attach.IsApplefile())
						aFile->OpenResourceFork(fsWrPerm);
				}
				catch (const PP_PowerPlant::LException& ex)
				{
					CLOG_LOGCATCH(const);

					CErrorHandler::PutOSErrAlertRsrc("Alerts::Attachments::BinHexNoFile", ex.GetErrorCode());
					delete aFile;
					aFile = NULL;
				}
			}
		}
#else
		cdstring fpath;
		if (MapToFile(attach.GetMappedName(true, true), fpath, view))
		{
			try
			{
				if (!attach.GetContent().IsBinHexed() && !attach.GetContent().IsUUed())
				{
					// Create file object and specify it
#if __dest_os == __win32_os
					aFile = new CFullFileStream(fpath.win_str(), CFile::modeCreate | CFile::modeWrite);
#elif __dest_os == __linux_os
					aFile = new CFullFileStream(fpath, O_WRONLY | O_CREAT);
#endif
				}
				else
				{
					aFile = new CFullFileStream;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					aFile->SetFilePath(fpath);
#elif __dest_os == __win32_os
					aFile->SetFilePath(fpath.win_str());
#elif __dest_os == __linux_os
					aFile->SetSpecifier(fpath);
#endif
				}
			}
			catch (CFileException* ex)
			{
				CLOG_LOGCATCH(CFileException*);

				CErrorHandler::PutFileErrAlertRsrc("Alerts::Attachments::BinHexNoFile", *ex);
				aFile = NULL;
			}
		}
#endif
	}
	return aFile;
}

// Get local filename for MIME part
bool CAttachmentManager::MapToFile(const cdstring& name, fspectype& file, bool view)
{
	// Make local copy
	cdstring fname = name;

	// Make name safe for OS
	MakeSafeFileName(fname);

#if __dest_os == __mac_os || __dest_os == __mac_os_x

	// Decide whether to use default path (don't prompt if doing view)
	if (!view && (CPreferences::sPrefs->mAskDownload.GetValue() || CPreferences::sPrefs->mDefaultDownload.GetValue().empty()))
	{
		bool			done = false;
		PPx::FSObject	fspec;
		MyCFString		cfstr(fname, kCFStringEncodingUTF8);
		bool			replacing = false;

		// Do standard save as dialog with directory Adjustment if required
		if (CAttachment::sDropLocation != NULL)
		{
			done = true;
			fspec = PPx::FSObject(*CAttachment::sDropLocation, cfstr);
			replacing = fspec.Exists();
		}
		else
		{
			// Get file spec from user
			done = PP_StandardDialogs::AskSaveFile(cfstr, '****', fspec, replacing, kNavDefaultNavDlogOptions | kNavNoTypePopup);
		}


		if (done)
		{
			if (replacing)	// Delete existing file
			{
				fspec.Delete();
			}

			file = fspec;
			return true;
		}
		else
			return false;
	}
	
	// If viewing create temp directory
	else if (view)
	{
		::TempFileSpecAttachments(file, fname);

		return true;
	}

	// Use the default download directory from the preferences
	else
	{
		// Concat path and name
		cdstring fpath = CPreferences::sPrefs->mDefaultDownload.GetValue() + fname;

		// Make sure its unique
		MyCFString cfstr(fpath, kCFStringEncodingUTF8);
		PPx::FSObject fspec(cfstr);
		cdstring nname = fname;
		int ctr = 1;

		while(fspec.Exists())
		{
			// Add number to name
			nname = fname + cdstring((long) ctr++);
			fpath = CPreferences::sPrefs->mDefaultDownload.GetValue() + nname;
			MyCFString cfstr(fpath, kCFStringEncodingUTF8);
			fspec = PPx::FSObject(cfstr);
		}

		// Alert if directory not found
		if (!fspec.IsValid())
		{
			UDesktop::Deactivate();		// Alert will swallow Deactivate event
			::SysBeep(1);
			::StopAlert(206, NULL);
			UDesktop::Activate();
		}
		
		file = fspec;

		return true;
	}
#elif __dest_os == __win32_os

	// Decide whether to use default path
	if (!view && (CPreferences::sPrefs->mAskDownload.GetValue() || CPreferences::sPrefs->mDefaultDownload.GetValue().empty()))
	{
		const char* fext = ::strchr(fname.c_str(), '.');
		if (fext)
			fext++;
		CFileDialog dlg(false, cdstring(fext).win_str(), fname.win_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, CSDIFrame::GetAppTopWindow());

		if (dlg.DoModal() == IDOK)
		{
			CString fpath = dlg.GetPathName();

			// Delete any existing file
			try
			{
				if (!::DeleteFile(fpath))
					CFileException::ThrowOsError((LONG)::GetLastError());
			}
			catch (CFileException* ex)
			{
				CLOG_LOGCATCH(CFileException*);

				// Only allow file not found
				if (ex->m_cause != CFileException::fileNotFound)
				{
					CLOG_LOGRETHROW;
					throw;
				}
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				fext = NULL;
			}

			file = fpath;
			return true;
		}
		else
			return false;
	}

	// If viewing create temp directory
	else if (view)
	{
		::TempFileSpecAttachments(file, fname);
		return true;
	}

	// Use the default download directory from the preferences
	else
	{
		// Concat path and name
		file = CPreferences::sPrefs->mDefaultDownload.GetValue() + fname;

		// Make sure its unique
		cdstring nname = fname;
		int ctr = 1;
		while(GetFileAttributes(file.win_str()) != -1)
		{
			// Add number to name
			const char* p = ::strrchr(fname.c_str(), '.');
			nname = cdstring(fname, 0, p - fname.c_str());
			nname += cdstring((long) ctr++);
			nname += p;
			file = CPreferences::sPrefs->mDefaultDownload.GetValue() + nname;
		}

		// Alert if directory not found
		DWORD err = ::GetLastError();
		if (err == ERROR_PATH_NOT_FOUND)
			CErrorHandler::PutStopAlertRsrc("Alerts::Attachments::DownloadDirFail");
		if (err != ERROR_FILE_NOT_FOUND)
		{
			CLOG_LOGTHROW(CGeneralException, err);
			throw CGeneralException(err);
		}

		return true;
	}
#elif __dest_os == __linux_os
	// Decide whether to use default path
	if (!view && (CPreferences::sPrefs->mAskDownload.GetValue() || CPreferences::sPrefs->mDefaultDownload.GetValue().empty()))
	{
		JString newfile;
		if (JGetChooseSaveFile()->SaveFile("prompt", NULL, fname.c_str(), &newfile))
		{
			// Delete any existing file
			if (JFileExists(newfile))
				JRemoveFile(newfile);
			file = newfile;
			return true;
		}
		else
			return false;
	}

	// If viewing create temp directory
	else if (view)
	{
		// Use view files temp directory as the directory for attachments
		file = CConnectionManager::sConnectionManager.GetViewAttachmentDirectory();
		::TempFileSpecAttachments(file, fname);
		return true;
	}

	// Use the default download directory from the preferences
	else
	{
		// Concat path and name
		file = CPreferences::sPrefs->mDefaultDownload.GetValue() + fname;
		
		// Make sure its unique
		cdstring nname = fname;
		int ctr = 1;
		while(JFileExists(file))
		{
			// Add number to name
			const char* p = ::strrchr(fname.c_str(), '.');
			nname = cdstring(fname, 0, p - fname.c_str());
			nname += cdstring((long) ctr++);
			nname += p;
			file = JCombinePathAndName(CPreferences::sPrefs->mDefaultDownload.GetValue(), nname);
		}
		
		// Alert if directory not found
		JString dir, name;
		JSplitPathAndName(file, &dir, &name);
		if (!JDirectoryExists(dir))
		{
			CErrorHandler::PutStopAlertRsrc("Alerts::Attachments::DownloadDirFail");
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		return true;
	}
#endif
}

// Try to launch the temp file
bool CAttachmentManager::LaunchURL(const cdstring& url) const
{
	// URL must exist
	if (!Exists(url))
		return false;

	// Get fspec for this URL
	const fspectype& fspec = GetEntry(url)->second.mSpec;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Name
	cdstring name(fspec.GetName());

	// Try default map first
	OSType appCreator = CMIMESupport::MapMIMEToCreator(name, GetEntry(url)->second.mMimeType);

	// If not found or decoded file use file's creator
	if (!appCreator)
	{
		FinderInfo finfo;
		fspec.GetFinderInfo(&finfo, NULL, NULL);
		appCreator = finfo.file.fileCreator;
	}

	CAppLaunch::OpenDocumentWithApp(&fspec, appCreator);
#elif __dest_os == __win32_os
	TCHAR dir[MAX_PATH];
	if (::GetCurrentDirectory(MAX_PATH, dir))
	{
		// Look for a shell launch spec
		if (CPreferences::sPrefs->mShellLaunch.GetValue().length())
		{
			// Get file part of launch spec
			cdstring temp = CPreferences::sPrefs->mShellLaunch.GetValue();
			char* p = temp.c_str_mod();
			cdstring file = ::strgetquotestr(&p);

			// Insert parameter
			cdstring params;
			size_t params_reserve = ::strlen(p) + fspec.length() + 1;
			params.reserve(params_reserve);
			::snprintf(params.c_str_mod(), params_reserve, p, fspec);

			::ShellExecute(*::AfxGetMainWnd(), _T("open"), file.win_str(), params.win_str(), dir, SW_SHOWNORMAL);
		}
		else
			::ShellExecute(*::AfxGetMainWnd(), _T("open"), fspec.win_str(), NULL, dir, SW_SHOWNORMAL);
	}
#elif __dest_os == __linux_os
	// Get type/subtype text
	const cdstring type = GetEntry(url)->second.mMimeType;

	// Get view-command from mailcap
	cdstring cmd = CMailcapMap::sMailcapMap.GetCommand(type);

	// Check for valid command
	if (cmd.empty())
		return false;

	// Do file name substitution
	cdstring buf;
	size_t buf_reserve = cmd.length() + fspec.length() + 1;
	buf.reserve(buf_reserve);
	::snprintf(buf.c_str_mod(), buf_reserve, cmd.c_str(), fspec.c_str());

	// Execute the command
	pid_t childPID;
	JExecute(buf.c_str(), &childPID);
#endif

	return true;
}

// View the content of a file attachment
void CAttachmentManager::ViewFile(const CFileAttachment* fattach)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	TryLaunch(fattach, *fattach->GetFSSpec());
#else
	TryLaunch(fattach, fattach->GetFilePath());
#endif
}

// Try to launch the decoded file
void CAttachmentManager::TryLaunch(const CAttachment* attach, CFullFileStream* aFile) const
{
	// Get current file spec (do this before closing the stream as that will clear the spec)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	PPx::FSObject fspec;
	aFile->GetSpecifier(fspec);
#else
	cdstring fspec = aFile->GetFilePath();
#endif

	// Close file stream prior to launch
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	aFile->CloseDataFork();
	aFile->CloseResourceFork();
#else
	aFile->Close();
#endif

	// Now do launch
	TryLaunch(attach, fspec);
}

void CAttachmentManager::TryLaunch(const CAttachment* attach, const fspectype& fspec) const
{
	// Check that user wants to launch and there is something to launch
	if ((CMIMEMap::FindAppLaunch(*attach) != eAppLaunchNever) &&
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		fspec.Exists() &&
#else
		fspec.length() &&
#endif
		(!attach->CanDisplay() || CPreferences::sPrefs->mLaunchText.GetValue()))
	{
		bool launch = true;

		// Ask user first?
		if (CMIMEMap::FindAppLaunch(*attach) == eAppLaunchAsk)
		{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			cdstring fname(fspec.GetName());
#else
			cdstring fname = fspec;
#endif
			cdstring appName = CMIMESupport::MapMIMEToApp(*attach);

			// If no default app name try to get it from decoded file's creator
			if (appName.empty())
			{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				FinderInfo finfo;
				fspec.GetFinderInfo(&finfo, NULL, NULL);
				OSType appCreator = finfo.file.fileCreator;
				appName.reserve(64);
				CDesktopIcons::GetAPPLName(appCreator, (unsigned char*) (char*) appName);
				p2cstr((unsigned char*) (char*) appName);
#endif
			}

			// Must have name
			if (!appName.empty())

				launch = (CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::Attachments::DownloadLaunchAsk", fname, appName.c_str(), NULL, NULL, true) == CErrorHandler::Ok);
			else
				launch = false;
		}


		if (launch)
		{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			// Try default map first
			OSType appCreator = CMIMESupport::MapMIMEToCreator(*attach);

			// If not found or decoded file use file's creator
			if (!appCreator)
			{
				FinderInfo finfo;
				fspec.GetFinderInfo(&finfo, NULL, NULL);
				appCreator = finfo.file.fileCreator;
			}

			CAppLaunch::OpenDocumentWithApp(&fspec, appCreator);
#elif __dest_os == __win32_os
			TCHAR dir[MAX_PATH];
			if (::GetCurrentDirectory(MAX_PATH, dir))
			{
				// Look for a shell launch spec
				if (CPreferences::sPrefs->mShellLaunch.GetValue().length())
				{
					// Get file part of launch spec
					cdstring temp = CPreferences::sPrefs->mShellLaunch.GetValue();
					char* p = temp.c_str_mod();
					cdstring file = ::strgetquotestr(&p);

					// Insert parameter
					cdstring params;
					size_t params_reserve = ::strlen(p) + fspec.length() + 1;
					params.reserve(params_reserve);
					::snprintf(params.c_str_mod(), params_reserve, p, fspec);

					::ShellExecute(*::AfxGetMainWnd(), _T("open"), file.win_str(), params.win_str(), dir, SW_SHOWNORMAL);
				}
				else
					::ShellExecute(*::AfxGetMainWnd(), _T("open"), fspec.win_str(), NULL, dir, SW_SHOWNORMAL);
			}
#elif __dest_os == __linux_os
			// Get type/subtype text
			const cdstring type = CMIMESupport::GenerateContentHeader(attach, false, lendl, false);

			// Get view-command from mailcap
			cdstring cmd = CMailcapMap::sMailcapMap.GetCommand(type);

			// Check for valid command
			if (cmd.empty())
				return;

			// Do file name substitution
			cdstring buf;
			size_t buf_reserve = cmd.length() + fspec.length() + 1;
			buf.reserve(buf_reserve);
			::snprintf(buf.c_str_mod(), buf_reserve, cmd.c_str(), fspec.c_str());

			// Execute the command
			pid_t childPID;
			JExecute(buf.c_str(), &childPID);
#endif
		}
	}
}
