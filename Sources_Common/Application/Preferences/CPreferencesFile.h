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


// Header for CPreferencesFile class

#ifndef __CPREFERENCESFILE__MULBERRY__
#define __CPREFERENCESFILE__MULBERRY__

#include "CMulberryCommon.h"

#include "cdstring.h"

// Consts

#if __dest_os == __mac_os || __dest_os == __mac_os_x
const OSType kPrefFileCreator = kApplID;				// File creator id
const OSType kPrefFileType = 'Pref';					// File type

const OSType krsrcType = 'Prfs';			// Resource type for stored prefs
const OSType kStatusInfoType = 'Xwin';		// Resource type for status window info
const OSType kServerInfoType = 'Swin';		// Resource type for server window info
const OSType kMailboxInfoType = 'Mwin';		// Resource type for mailbox window info
const OSType kMessageInfoType = 'Pwin';		// Resource type for message window info
const OSType kLetterInfoType = 'Lwin';		// Resource type for letter window info
const OSType kAddressInfoType = 'Awin';		// Resource type for address book window info
const short krsrcID = 1;					// Resource id for stored prefs
const short kAddressBookAlias = 1;			// Resource id for address book alias

#else
const char cPrefFileExtension[] = ".mbp";				// File extension
#endif
#if __dest_os == __linux_os
#include "UNX_LFileStream.h"
#endif
#if __dest_os == __mac_os || __dest_os == __mac_os_x
// STR# errors
const ResIDT	STRx_PrefsErrorID = 1024;
enum {
	xxstr_CannotCreatePrefs = 1,			// Cannot create the prefs for some reason
	xxstr_CannotSavePrefs,				// Cannot save the prefs for some reason
	xxstr_CannotReadPrefs,				// Cannot read the prefs for some reason
	xxstr_OldVersPrefs,					// Tried to use old version
	xxstr_ReallyReplacePrefs,				// Really replace existing prefs
	xxstr_UIDReplacePrefs					// UID change when saving prefs
};
#endif

// Classes

class CPreferences;
class COptionsMap;

#ifdef PREFS_USE_LSTREAM
class CPreferencesFile : public LFileStream
#else
class CPreferencesFile : public CFile
#endif
{
private:
	CPreferences*	mItsPrefs;							// Copy of its prefs
	bool			mIsSpecified;						// Has file been accessed at least once?
	COptionsMap*	mMap;								// Options map
	bool			mDefaultFile;						// Default file
	bool			mRemoteFile;						// Remote file
	bool			mOriginalLocal;						// Has an original local file
	bool			mOriginalDefault;					// Original local file was default
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	PPx::FSObject	mOriginalLocalFileSpec;				// Specifier of original local file
	cdstring		mFileName;							// Name of file
#elif __dest_os == __win32_os
	cdstring		mFileName;							// Name of file
	cdstring		mOriginalLocalFileName;				// Name of original local file
#elif __dest_os == __linux_os
	cdstring		mOriginalLocalFileName;				// Name of original local file
#endif

public:

					CPreferencesFile(bool default_file, bool remote_file);
	virtual 		~CPreferencesFile();

public:
			bool	IsRemote() const
				{ return mRemoteFile; }
			void	SetRemote(bool remote);
			bool	IsDefault() const
				{ return mDefaultFile; }
			bool	IsOriginalLocal() const
				{ return mOriginalLocal; }
			bool	IsOriginalDefault() const
				{ return mOriginalDefault; }
#ifndef PREFS_USE_LSTREAM
	virtual void	SetSpecifier(const char* name);
	virtual cdstring&	GetSpecifier()
						{ return mFileName; }
#else
	virtual void		 	SetName(const char* name);		// Set the file name
	virtual cdstring&	GetName();				// Get the file name
#endif
	virtual void	SetSpecified(bool specified)
						{ mIsSpecified = specified; }		// Set specified state
	virtual bool	IsSpecified() const
						{ return mIsSpecified; }			// Is file specified?

			CPreferences*	GetPrefs() const			// Get the prefs
				{ return mItsPrefs; }
			void	SetPrefs(CPreferences* prefs)		// Set prefs
				{ mItsPrefs = prefs; }

			void	CreateDefault();					// Create default prefs file in System Folder

			void	SavePrefs(bool verify, bool force_dirty);	// Save prefs in file
			void	SaveOriginalLocal();
			bool	ReadPrefs(NumVersion vers_app,
								NumVersion& vers_prefs);					// Read prefs from file
			bool	VerifyRead(bool is_local,								// Verify and read preferences
								bool write_back = true,
								bool* pending_update = NULL);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	virtual void	DoSaveAs(PPx::FSObject& sfFile, bool replacing);		// Save prefs file as
#elif __dest_os == __win32_os
	virtual void	DoSaveAs(const CString& fileName);		// Save prefs file as
#elif __dest_os == __linux_os
	virtual void DoSaveAs(const cdstring& fileName); //Save prefs file as
	static void GetDefaultLocalFileName(cdstring& fileName);
	static const cdstring& GetDefaultLocalFileName();
#else
#error __dest_os
#endif

private:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			void	WriteResource(Handle rsrc_data, OSType rsrc, short rsrc_id);
#elif __dest_os == __linux_os
			static cdstring mDefaultFileName;
#endif

private:
			void	InitMap();

			void	AskForRemoteSet();
};

#endif
