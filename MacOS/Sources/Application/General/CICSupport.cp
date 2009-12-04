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


// Source for CICSupport class

#include "CICSupport.h"
#include "CAttachment.h"
#include "CFileAttachment.h"
#include "CMIMEMap.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CStringUtils.h"

#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C I C S U P P O R T
// __________________________________________________________________________________________________

ICInstance CICSupport::sInstance = NULL;
bool CICSupport::sLoaded = false;

// O T H E R  M E T H O D S ____________________________________________________________________________

// Start InternetConfig
OSStatus CICSupport::ICStart(OSType appSig)
{
	if (!sLoaded)
	{
		short refnum = ::CurResFile();
		OSStatus err = ::ICStart(&sInstance, appSig);
		::UseResFile(refnum);

		if (!err)
			sLoaded = true;

		return err;
	}
	else
		return noErr;
}

// Stop InternetConfig
OSStatus CICSupport::ICStop()
{
	if (!sInstance) return noErr;

	short refnum = ::CurResFile();
	OSStatus err = ::ICStop(sInstance);
	::UseResFile(refnum);

	sInstance = NULL;

	return err;
}

// Get file from local/preferences directory
OSStatus CICSupport::ICFindConfigFile(FSSpec& dir)
{
	if (!sInstance) return ioErr;

#if TARGET_API_MAC_CARBON
	return noErr;
#else
	short refnum = ::CurResFile();
	OSStatus err = ::ICFindConfigFile(sInstance, 0, NULL);
	::UseResFile(refnum);

	return err;
#endif
}

// Launch a URL
OSStatus CICSupport::ICLaunchURL(char *url)
{
	long start = 0;
	long stop = strlen(url);
	return ::ICLaunchURL(sInstance, "\p", url, ::strlen(url), &start, &stop);
}

// Get preference from file
OSStatus CICSupport::ICGetPrefPstring(ConstStr255Param key, cdstring& pref)
{
	OSStatus err = ICGetPrefText(key, pref);
	if (err == noErr)
		p2cstr(reinterpret_cast<unsigned char*>(pref.c_str_mod()));

	return err;
}

// Get preference from file
OSStatus CICSupport::ICGetPrefText(ConstStr255Param key, cdstring& pref)
{
	pref = cdstring::null_str;

	if (!sInstance) return ioErr;

	ICAttr attr;
	long actual_size = 0;

	short refnum = ::CurResFile();

	// Determine actual size first
	OSStatus err = ::ICGetPref(sInstance, key, &attr, NULL, &actual_size);
	if (err == noErr)
	{
		// Reserve enough space and read it in
		pref.reserve(actual_size);
		err = ::ICGetPref(sInstance, key, &attr, pref, &actual_size);
		if (err)
			pref = cdstring::null_str;
	}
	::UseResFile(refnum);

	return err;
}

// Get preference from file
OSStatus CICSupport::ICGetPrefBoolean(ConstStr255Param key, bool& pref)
{
	Boolean result = false;
	OSStatus err = ICGetPref(key, reinterpret_cast<char*>(&result), sizeof(Boolean));
	pref = result;

	return err;
}

// Get preference from file
OSStatus CICSupport::ICGetPref(ConstStr255Param key, char* pref, long size)
{
	if (!sInstance) return ioErr;

	ICAttr attr;
	short refnum = ::CurResFile();

	// Get pref
	OSStatus err = ::ICGetPref(sInstance, key, &attr, pref, &size);
	::UseResFile(refnum);

	return err;
}

// Set preference in the file
OSStatus CICSupport::ICSetPref(ConstStr255Param key, char* pref, long size)
{
	if (!sInstance) return ioErr;

	ICAttr attr = kICAttrNoChange;
	short refnum = ::CurResFile();

	// Get pref
	OSStatus err = ::ICSetPref(sInstance, key, attr, pref, size);
	::UseResFile(refnum);

	return err;
}

void CICSupport::ICGetRealName(cdstring& name)
{
	ICGetPrefPstring(kICRealName, name);
}

void CICSupport::ICGetEmail(cdstring& email)
{
	ICGetPrefPstring(kICEmail, email);
}

void CICSupport::ICGetMailAccount(cdstring& user, cdstring& domain)
{
	cdstring txt;

	if (ICGetPrefPstring(kICMailAccount, txt) == noErr)
	{
		// Parse into bits
		char* p = ::strchr(txt.c_str_mod(), '@');
		if (p)
		{
			*p++ = '\0';
			user = txt;
		}
		else
			p = txt;

		domain = p;
	}
	else
	{
		user = cdstring::null_str;
		domain = cdstring::null_str;
	}

}

void CICSupport::ICGetSMTPHost(cdstring& smtp)
{
	ICGetPrefPstring(kICSMTPHost, smtp);
}

void CICSupport::ICGetPswd(cdstring& pswd)
{
	ICGetPrefPstring(kICMailPassword, pswd);
	ICDecrypt(pswd);
}

void CICSupport::ICGetFlashIcon(bool& flash)
{
	ICGetPrefBoolean(kICNewMailFlashIcon, flash);
}

void CICSupport::ICGetDisplayDialog(bool& dialog)
{
	ICGetPrefBoolean(kICNewMailDialog, dialog);
}

void CICSupport::ICGetPlaySound(bool& sounds)
{
	ICGetPrefBoolean(kICNewMailPlaySound, sounds);
}

void CICSupport::ICGetAlertSound(cdstring& snd)
{
	ICGetPrefPstring(kICNewMailSoundName, snd);
}

void CICSupport::ICGetListFont(ICFontRecord* font)
{
	if (ICGetPref(kICListFont, (char*) font, sizeof(ICFontRecord)) != noErr)
	{
		font->size = 0;
		font->face = 0;
		font->font[0] = '\0';
	}
}

void CICSupport::ICGetScreenFont(ICFontRecord* font)
{
	if (ICGetPref(kICScreenFont, (char*) font, sizeof(ICFontRecord)) != noErr)
	{
		font->size = 0;
		font->face = 0;
		font->font[0] = '\0';
	}
}

void CICSupport::ICGetPrintFont(ICFontRecord* font)
{
	if (ICGetPref(kICPrinterFont, (char*) font, sizeof(ICFontRecord)) != noErr)
	{
		font->size = 0;
		font->face = 0;
		font->font[0] = '\0';
	}
}

void CICSupport::ICGetTextCreator(OSType& creator)
{
//	ICAppSpec spec;

//	if (ICGetPref(kICTextCreator, (char*) &spec, sizeof(ICAppSpec)) == noErr)
//		creator = spec.fCreator;
//	else
		creator = '????';
}

void CICSupport::ICGetQuote(cdstring& quote)
{
	quote = cdstring::null_str;
	ICGetPrefPstring(kICQuotingString, quote);
}

void CICSupport::ICGetMailHeaders(cdstring& header)
{
	ICGetPrefText(kICMailHeaders, header);
}

void CICSupport::ICGetSignature(cdstring& signature)
{
	ICGetPrefText(kICSignature, signature);
}

bool CICSupport::ICCheckHelperMailto()
{
	return ICCheckHelper("\pmailto");
}

bool CICSupport::ICSetHelperMailto()
{
	return ICSetHelper("\pmailto");
}

bool CICSupport::ICCheckHelperWebcal()
{
	return ICCheckHelper("\pwebcal");
}

bool CICSupport::ICSetHelperWebcal()
{
	return ICSetHelper("\pwebcal");
}

bool CICSupport::ICCheckHelper(ConstStr255Param scheme)
{
	// Create Helper¥xxx key
	LStr255 key1(kICHelperList);
	key1 += scheme;
	
	// First look to see if its listed as one of the defaults
	cdstring buffer;
	buffer.reserve(1024);
	ICAppSpecList* list = reinterpret_cast<ICAppSpecList*>(buffer.c_str_mod());
	OSStatus err = ICGetPref(key1, reinterpret_cast<char*>(list), 1024);
	if (err == noErr)
	{
		// Scan list looking for us
		bool found = false;
		for(int i = 0; i < list->numberOfItems; i++)
		{
			if (list->appSpecs[i].fCreator == kApplID)
			{
				found = true;
				break;
			}
		}
		
		// If not found try and add it
		if (!found)
		{
			// Create buffer for new list
			short newsize = list->numberOfItems + 1;
			cdstring newbuffer;
			newbuffer.reserve(sizeof(short) + newsize * sizeof(ICAppSpec));
			ICAppSpecList* newlist = reinterpret_cast<ICAppSpecList*>(newbuffer.c_str_mod());
			
			// Init size and add us as the first item
			newlist->numberOfItems = newsize;
			newlist->appSpecs[0].fCreator = kApplID;
			::PLstrcpy(newlist->appSpecs[0].name, "\pMulberry");
			
			// Copy in remaining items
			for(int i = 0; i < list->numberOfItems; i++)
				newlist->appSpecs[i + 1] = list->appSpecs[i];
			
			// Write the preference - ignore error
			err = ICSetPref(key1, reinterpret_cast<char*>(newlist), sizeof(short) + newsize * sizeof(ICAppSpec));
		}
	}

	// Create Helper¥xxx key
	LStr255 key2(kICHelper);
	key2 += scheme;
	
	// Read preference
	ICAppSpec spec;
	err = ICGetPref(key2, reinterpret_cast<char*>(&spec), sizeof(spec));
	if (err == noErr)
		// Check creator code
		return (spec.fCreator == kApplID);
	else
		return false;
}

bool CICSupport::ICSetHelper(ConstStr255Param scheme)
{
	// Create Helper¥mailto key
	LStr255 key(kICHelper);
	key += scheme;
	
	// Setup application preference
	ICAppSpec spec;
	spec.fCreator = kApplID;
	::PLstrcpy(spec.name, "\pMulberry");

	// Write preference
	OSStatus err = ICSetPref(key, reinterpret_cast<char*>(&spec), sizeof(spec));
	return (err == noErr);
}

// Decrypt IC entry
void CICSupport::ICDecrypt(cdstring& decrypt)
{
	char* p = (char*) decrypt.c_str();

	char xor_it = 0x56;
	while(*p)
		*p++ = *p ^ xor_it++;
}

// Map a file name of a file attachment
OSStatus CICSupport::ICMapTypeCreator(CFileAttachment& attach, bool use_rsrc)
{
	// Policy:

	// Get name and look for suffix
	LStr255 name(attach.GetName());
	UInt8 pos = name.ReverseFind('.');

	// Create dummy entry for text file
	FinderInfo finfo;
	attach.GetFSSpec()->GetFinderInfo(&finfo);
	LStr255 blank(((finfo.file.fileType == 'TEXT') &&
				   (finfo.file.fileCreator == 'ttxt')) ? ".txt" : "");

	ICMapEntry entry;
	OSStatus err;

	// Only do IC if present
	if (sInstance)
	{
		short refnum = ::CurResFile();
		err = ::ICMapTypeCreator(sInstance, finfo.file.fileType,
									finfo.file.fileCreator, (pos > 1) ? name : blank, &entry);

		// Use IC to lookup based on extension if type/creator fails
		if ((err != noErr) && (pos >= 1))
		{
			// Do mapping
			err = ::ICMapFilename(sInstance, name, &entry);
		}

		::UseResFile(refnum);
	}

	// Check for successful IC map or no IC
	if (!err && sInstance)
	{
		// Special hack for extensionless text files where the creator cannot be found
		if ((finfo.file.fileType == 'TEXT') &&
			(finfo.file.fileCreator != entry.fileCreator) &&
			!pos)
		{
			// Force MIME to text/plain
			::PLstrcpy(entry.MIMEType, "\ptext/plain");
			::PLstrcpy(entry.extension, "\p.txt");
		}

		// No longer remap the file extension on outgoing attachments
		// We now assume whatever is there is correct for sending
#if 0
		// Add extension to filename stripping off any existing extension

		// Only do if '.' is not first char
		if (pos != 1)
		{
			// Delete to end of string if extension already exists
			if (pos)
				name.Remove(pos, 255);

			// Append new extension
			name.Append(entry.extension);
		}
#endif

		// Give new name back to attachment
		attach.GetContent().SetMappedName(name);

		// Options for content/encoding
		//
		// 1 - Check mime type:
		//	   if not default always or doing non-resource calc. (e.g. AppleDouble second part)
		//     if type/sub-type exists set these in attachment (this will set encoding as well)
		//
		// 2 - No mime type:
		//     if not binary set attachment to text/plain
		//     if binary without rsrc fork set to application/octet-stream
		//     if binary and rsrc fork force mode to binhex
		//

		// If type available then set
		if ((!CPreferences::sPrefs->mDefault_Always.GetValue() || !use_rsrc) && ::PLstrlen(entry.MIMEType))
			attach.GetContent().SetContent(entry.MIMEType);
		else
		{
			// Check for text only
			if (!(entry.flags & kICMapBinaryMask))
				attach.GetContent().SetContent(eContentText, eContentSubPlain);

			// Check for binary without rsrc or no process of resource
			else if (!(entry.flags & kICMapResourceForkMask) || !use_rsrc)
			{
				// Force to default encoding?
				if (CPreferences::sPrefs->mDefault_Always.GetValue())
					attach.GetContent().SetTransferMode(CPreferences::sPrefs->mDefault_mode.GetValue());
				else
				{
					attach.GetContent().SetContent(eContentApplication, eContentSubOctetStream);
					attach.GetContent().SetTransferMode(eMIMEMode);
				}
			}
			// Must be binary + rsrc => always use default encoding
			else
				attach.GetContent().SetTransferMode(CPreferences::sPrefs->mDefault_mode.GetValue());
		}
	}
	else
	{
		// Mapped name is the same as unmapped
		attach.GetContent().SetMappedName(name);

		// Check for text only
		if (finfo.file.fileType == 'TEXT')
			attach.GetContent().SetContent(eContentText, eContentSubPlain);

		else
		{
			// Check for resource fork
			try
			{
				FSCatalogInfo catInfo;
				attach.GetFSSpec()->GetCatalogInfo(kFSCatInfoDataSizes | kFSCatInfoRsrcSizes, catInfo);

				// Check for binary without rsrc or no process resource
				if ((catInfo.rsrcLogicalSize == 0) || !use_rsrc)
				{
					// Force to default encoding?
					if (CPreferences::sPrefs->mDefault_Always.GetValue())
						attach.GetContent().SetTransferMode(CPreferences::sPrefs->mDefault_mode.GetValue());
					else
					{
						attach.GetContent().SetContent(eContentApplication, eContentSubOctetStream);
						attach.GetContent().SetTransferMode(eMIMEMode);
					}
				}
				// Must be binary + rsrc => default
				else
					attach.GetContent().SetTransferMode(CPreferences::sPrefs->mDefault_mode.GetValue());
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Completely unknown => default to default mode
				attach.GetContent().SetTransferMode(CPreferences::sPrefs->mDefault_mode.GetValue());
			}
		}
	}

	return err;
}

// Map an attachment name
OSStatus CICSupport::ICMapFileName(const CMIMEContent& content, ICMapEntry& entry, bool exact_mime)
{
	LStr255 name(content.GetMappedName());
	OSStatus err;

	// If binhex mode ignore extension
	if (content.IsBinHexed())
	{
		entry.totalLength = 0;
		entry.fixedLength = 0;
		entry.version = 0;
		entry.fileType = 'BINA';
		entry.fileCreator = 'hDmp';
		entry.postCreator = 0L;
		entry.flags = 0L;
		entry.extension[0] = '\0';
		entry.creatorAppName[0] = '\0';
		entry.postAppName[0] = '\0';
		entry.MIMEType[0] = '\0';
		entry.entryName[0] = '\0';

		return noErr;
	}

	// Get type/subtype
	cdstring type = content.GetContentTypeText();
	type += "/";
	type += content.GetContentSubtypeText();

	// Check for explicit mapping first
	if (CPreferences::sPrefs->mExplicitMapping.GetValue())
	{
		if (GetDefaultMatch(type, entry) == noErr) return noErr;
	}

	// Check for extension first
	UInt8 pos = name.ReverseFind(".", 1);
	cdstring ext;
	if (pos >= 1)
	{
		ext.append(name.ConstTextPtr()[pos], name.Length() - pos + 1);
	}

	// Use IC to lookup extension
	if ((pos >= 1) && sInstance)
	{

		// Do mapping
		short refnum = ::CurResFile();
		err = ::ICMapFilename(sInstance, name, &entry);
		::UseResFile(refnum);

		if (!err) return err;

	}

	// Try default match if not already done
	if (!CPreferences::sPrefs->mExplicitMapping.GetValue())
	{
		// Get the default match
		if (GetDefaultMatch(type, entry) == noErr) return noErr;
	}

	// Everything else has failed so try this last

	// Get first match for MIME type/subtype description
	err = GetBestMatch(type, entry, ext, exact_mime);

	return err;
}

// Map an attachment name
OSStatus CICSupport::ICMapFileName(const cdstring& fname, ICMapEntry& entry, bool exact_mime)
{
	OSStatus err = noErr;

	// Init entry data
	entry.totalLength = 0;
	entry.fixedLength = 0;
	entry.version = 0;
	entry.fileType = '????';
	entry.fileCreator = '????';
	entry.postCreator = 0L;
	entry.flags = 0L;
	entry.extension[0] = '\0';
	entry.creatorAppName[0] = '\0';
	entry.postAppName[0] = '\0';
	entry.MIMEType[0] = '\0';
	entry.entryName[0] = '\0';

	// Check for extension first
	LStr255 name(fname);
	UInt8 pos = name.ReverseFind(".", 1);

	// Use IC to lookup extension
	if ((pos >= 1) && sInstance)
	{
		// Do mapping
		short refnum = ::CurResFile();
		err = ::ICMapFilename(sInstance, name, &entry);
		::UseResFile(refnum);
	}

	return err;
}

// Map an attachment name
OSStatus CICSupport::ICMapMIMEType(const cdstring& fname, const cdstring& type, ICMapEntry& entry, bool exact_mime)
{
	OSStatus err;

	// Check for explicit mapping first
	if (CPreferences::sPrefs->mExplicitMapping.GetValue())
	{
		if (GetDefaultMatch(type, entry) == noErr) return noErr;
	}

	// Check for extension first
	const char* pos = ::strrchr(fname.c_str(), '.');

	// Use IC to lookup extension
	if (pos && sInstance)
	{

		// Do mapping
		short refnum = ::CurResFile();
		LStr255 name(fname);
		err = ::ICMapFilename(sInstance, name, &entry);
		::UseResFile(refnum);

		if (!err) return err;

	}

	// Try default match if not already done
	if (!CPreferences::sPrefs->mExplicitMapping.GetValue())
	{
		// Get the default match
		if (GetDefaultMatch(type, entry) == noErr) return noErr;
	}

	// Everything else has failed so try this last

	// Get first match for MIME type/subtype description
	err = GetBestMatch(type, entry, pos, exact_mime);

	return err;
}

// Get best type/creator for attachment
OSStatus CICSupport::GetBestMatch(const cdstring& content, ICMapEntry& entry, const char* ext, bool exact_mime)
{
	// Copy in type/subtype
	cdstring txt_full = content;
	
	// Get just the type
	cdstring txt_type = content;
	if (::strchr(txt_type.c_str(), '/'))
		*strchr(txt_type.c_str_mod(), '/') = 0;
	
	// Some types will always be partially matched
	if ((txt_type.compare("text", true) == 0) ||
		(txt_type.compare("image", true) == 0) ||
		(txt_type.compare("audio", true) == 0) ||
		(txt_type.compare("video", true) == 0))
		exact_mime = false;

	// Get extension
	cdstring fext;
	if (ext != NULL)
		fext = ext;

	// Now get entries handle from IC
	OSStatus err = noErr;
	Handle entries = ::NewHandle(0);
	ICAttr junk_attr;
	bool match = false;
	short refnum = ::CurResFile();
	try
	{
		// Read in mappings pref
		ThrowIfError_(::ICFindPrefHandle(sInstance, kICMapping, &junk_attr, entries));

		// Count number of entries
		long count;
		ThrowIfError_(::ICCountMapEntries(sInstance, entries, &count));

		// Iterate over all entries looking for match (or partial match)
		ICMapEntry partial;
		bool got_partial_exact = false;
		bool got_partial = false;
		for(long i = 1; i <= count; i++)
		{
			long pos;

			// Get indexed entry
			ThrowIfError_(::ICGetIndMapEntry(sInstance, entries, i, &pos, &entry));
			cdstring entry_type(entry.MIMEType);
			::strlower(entry_type.c_str_mod());

			cdstring entry_ext(entry.extension);
			::strlower(entry_ext.c_str_mod());

			// Check for match to type/subtype
			if (txt_full == entry_type)
			{
				// Check for extension match
				if (fext == entry_ext)
				{
					match = true;
					break;
				}
				
				// Check for partial match
				else if (!got_partial_exact)
				{
					got_partial_exact = true;
					got_partial = true;
					partial = entry;
					continue;
				}
			}

			// Check for partial match to type
			if (!exact_mime && !got_partial && entry_type.compare_start(txt_type))
			{
				got_partial = true;
				partial = entry;
			}
		}

		// If only partial copy and cal it a match
		if (got_partial && !match)
		{
			entry = partial;
			match = true;
		}

		// Forget entries
		::DisposeHandle(entries);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (entries)
			::DisposeHandle(entries);

		// Force default setting if any error
		match = false;
		err = -1;
	}

	// If not found fill in default values
	if (!exact_mime && !match)
	{
		entry.totalLength = 0;
		entry.fixedLength = 0;
		entry.version = 0;
		entry.fileType = 'BINA';
		entry.fileCreator = 'hDmp';
		entry.postCreator = 0L;
		entry.flags = 0L;
		entry.extension[0] = '\0';
		entry.creatorAppName[0] = '\0';
		entry.postAppName[0] = '\0';
		entry.MIMEType[0] = '\0';
		entry.entryName[0] = '\0';
		err = noErr;
	}
	else if (!match)
		err = 1;
		

	::UseResFile(refnum);

	return err;
}

// Get default match to attachment
OSStatus CICSupport::GetDefaultMatch(const cdstring& content, ICMapEntry& entry)
{
	// Find a match
	const CMIMEMap* found = CMIMEMap::Find(content);

	// Set details if found
	if (found)
	{
		entry.totalLength = 0;
		entry.fixedLength = 0;
		entry.version = 0;
		entry.fileType = found->GetFileType();
		entry.fileCreator = found->GetFileCreator();
		entry.postCreator = 0L;
		entry.flags = 0L;
		entry.extension[0] = '\0';
		entry.creatorAppName[0] = '\0';
		entry.postAppName[0] = '\0';
		entry.MIMEType[0] = '\0';
		entry.entryName[0] = '\0';

		return noErr;
	}

	return icPrefNotFoundErr;
}

// Get default match to attachment
OSStatus CICSupport::GetDefaultMIME(CFileAttachment& attach)
{
	FinderInfo finfo;
	attach.GetFSSpec()->GetFinderInfo(&finfo);

	// Find a match
	const CMIMEMap* found = CMIMEMap::Find(finfo.file.fileType, finfo.file.fileCreator);

	// Set details if found
	if (found)
	{
		attach.GetContent().SetContentType(found->GetMIMEType());
		attach.GetContent().SetContentSubtype(found->GetMIMESubtype());
		return noErr;
	}

	return icPrefNotFoundErr;
}
