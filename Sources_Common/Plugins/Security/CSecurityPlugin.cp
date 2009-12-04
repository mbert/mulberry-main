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

// CSecurityPlugin.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 04-May-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for DLL based security plug-ins in Mulberry.
//
// History:
// CD:	 04-May-1998:	Created initial header and implementation.
//

//#define VISIBLE_TEMP_FILES

#include "CSecurityPlugin.h"

#include "CAddressList.h"
#include "CAttachment.h"
#include "CAttachmentList.h"
#include "CCertificateManager.h"
#include "CDataAttachment.h"
#include "CErrorHandler.h"
#include "CFileAttachment.h"
#include "CGeneralException.h"
#include "CGetPassphraseDialog.h"
#include "CLocalAttachment.h"
#include "CLocalCommon.h"
#include "CLocalMessage.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMessageWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CNetworkException.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CPreferenceVersions.h"
#include "CRFC822Parser.h"
#include "CSSLPlugin.h"
#include "CStreamAttachment.h"
#include "CStreamFilter.h"
#include "CStreamType.h"
#include "CStreamUtils.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"
#endif
#include "CStringUtils.h"
#include "CTextListChoice.h"
#include "CUtils.h"

#include "mimefilters.h"
#include "cdfstream.h"

#if __dest_os == __mac_os_x
#include "MyCFString.h"
#include <SysCFURL.h>
#endif

#include <algorithm>
#include <strstream>
#include <typeinfo>

CSecurityPlugin::SSecurityPluginHandlers CSecurityPlugin::sSecurityPlugins;
cdstring CSecurityPlugin::sPreferredPlugin;
cdstrmap CSecurityPlugin::sCanVerify;
cdstrmap CSecurityPlugin::sCanDecrypt;

cdstrmap CSecurityPlugin::sPassphrases;
cdstring CSecurityPlugin::sLastPassphraseUID;

const unsigned long cFileThreshold = 0x10000;

const char* cGPGName = "GPG Plugin";
const char* cPGPName = "PGP Plugin";
const char* cSMIMEName = "SMIME Plugin";

// Register a security plugin
void CSecurityPlugin::RegisterSecurityPlugin(CSecurityPlugin* plugin)
{
	// Add it to the list
	sSecurityPlugins.insert(SSecurityPluginHandlers::value_type(plugin->GetName(), plugin));
}

CSecurityPlugin* CSecurityPlugin::GetRegisteredPlugin(const cdstring& descriptor)
{
	SSecurityPluginHandlers::const_iterator found = sSecurityPlugins.find(descriptor);
	if (found != sSecurityPlugins.end())
		return (*found).second;
	else
		return NULL;
}

// Plugin used for sign/encrypt operations
CSecurityPlugin* CSecurityPlugin::GetDefaultPlugin()
{
	// Prompt user to choose if no preferred item and more than one is loaded
	if ((sSecurityPlugins.size() > 1) && CPreferences::sPrefs->mPreferredPlugin.GetValue().empty())
	{
		// Get list of available plugins
		cdstrvect plugins;
		for(SSecurityPluginHandlers::const_iterator iter = sSecurityPlugins.begin(); iter != sSecurityPlugins.end(); iter++)
			plugins.push_back((*iter).first);
		
		// Allow user to choose the one they want
		ulvector selected;
		if (CTextListChoice::PoseDialog("Alerts::General::ChooseCryptoPluginTitle", NULL, NULL, false, true, false, true, plugins, cdstring::null_str, selected, NULL))
		{
			// Save the choice in the preferences
			CPreferences::sPrefs->mPreferredPlugin.SetValue(plugins.at(selected.front()));
		}
		else
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
	}

	// Now try to find the appropriate plugin
	CSecurityPlugin* plugin = NULL;
	if (sSecurityPlugins.size() > 1)
	{
		// Look for one that matches the chosen name
		plugin = GetRegisteredPlugin(CPreferences::sPrefs->mPreferredPlugin.GetValue());
		
		if (plugin == NULL)
		{
			// Use alternate PGP/GPG depending on what is present
			if (CPreferences::sPrefs->mPreferredPlugin.GetValue() == cGPGName)
				plugin = GetRegisteredPlugin(cPGPName);
			else if (CPreferences::sPrefs->mPreferredPlugin.GetValue() == cPGPName)
				plugin = GetRegisteredPlugin(cGPGName);
		}
	}

	// Just use the first one in the list
	if (!plugin && sSecurityPlugins.size())
		plugin = sSecurityPlugins.begin()->second;

	return plugin;
}

// Make sure version matches
bool CSecurityPlugin::VerifyVersion() const
{
	// New API >= 3.1b5
	if (VersionTest(GetVersion(), VERS_3_1_0_B_5) >= 0)
		return true;
	else
	{
		CErrorHandler::PutStopAlertRsrcStr("Alerts::General::IllegalPluginCryptoVersion", GetName().c_str());
		return false;
	}
}

// Load information
void CSecurityPlugin::LoadPlugin()
{
	// Do inherited then set callback if all OK
	CPlugin::LoadPlugin();
	SetCallback();
	if (GetName() == cSMIMEName)
		SetContext();
}

#pragma mark ____________________________High Level

bool CSecurityPlugin::ProcessMessage(CMessage* msg, ESecureMessage mode, const char* key)
{
	// Only bother if something actually required
	if (mode == eNone)
		return true;

	// Cannot sign without key
	if (((mode == eSign) || (mode == eEncryptSign)) &&
		(!key || !*key))
		return false;

	// Load plugin
	StLoadPlugin load(this);

	bool result = false;
	while(true)
	{
		long err = 0;

		try
		{
			if (UseMIME())
				ProcessBody(msg, mode, key);
			else
				// Sign each part
				ProcessAttachment(msg, msg->GetBody(), mode, key);

			result = true;
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			err = HandleError();
			result = false;
		}
		
		// Try it again if error was bad passphrase
		if (result || (err != eSecurity_BadPassphrase))
			break;
	}

	return result;

}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
void CSecurityPlugin::CreateTempFile(PPx::FSObject* ftemp, ESecureMessage mode, const cdstring& name)
#else
void CSecurityPlugin::CreateTempFile(cdstring& ftemp, ESecureMessage mode, const cdstring& name)
#endif
{
	// Generate a suitable name
	cdstring new_name = name;
	if (new_name.empty())
	{
		static unsigned long sCtr = 0;
		
		new_name.reserve(L_tmpnam);
		::snprintf(new_name.c_str_mod(), L_tmpnam, "Temp_%lx%03ld", ::time(NULL), sCtr++ % 256);
	}
	switch(mode)
	{
	case eSign:
		new_name += ".sig";
		break;
	case eEncrypt:
	case eEncryptSign:
		new_name += ".asc";
		break;
	default:
		new_name += ".tmp";
	}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	::TempFileSpecSecurity(*ftemp, new_name);
	
	// Must create the file on Mac OS to ensure file path conversion will work
	LFile fileTemp(*ftemp);
	fileTemp.CreateNewFile('Mlby', 'SECR', smCurrentScript);
	ftemp->Update();
#else
	::TempFileSpecSecurity(ftemp, new_name);
#endif
}

void CSecurityPlugin::ApplyMIME(CAttachment* part, SMIMEInfo* info)
{
	// Add content type
	if (info->type)
		part->GetContent().SetContentType(info->type);
	
	// Add content subtype
	if (info->subtype)
		part->GetContent().SetContentSubtype(info->subtype);
	
	// Add parameters
	const char** p = info->params;
	while(p && *p)
	{
		const char* name = *p++;
		const char* value = *p++;
		if (name && *name && value && *value)
			part->GetContent().SetContentParameter(name, value);
	}
}

bool CSecurityPlugin::DoesEncryptSignAllInOne() const
{
	return GetName() != cSMIMEName;
}

bool CSecurityPlugin::UseMIME() const
{
	return (GetName() == cSMIMEName) || CPreferences::sPrefs->mUseMIMESecurity.GetValue();
}

#pragma mark ____________________________Operations on entire body

void CSecurityPlugin::ProcessBody(CMessage* msg, ESecureMessage mode, const char* key)
{
	// Special processing for Encrypt&Sign separate
	if ((mode == eEncryptSign) && !DoesEncryptSignAllInOne())
	{
		// Do signature first
		ProcessBody(msg, eSign, key);
		
		// Change mode to encrypt for second operation
		mode = eEncrypt;
	}

	// Sign entire message
	CAttachment* part = msg->GetBody();
	CAttachment* generated_part = NULL;

	// Special processing for crypto - only needed for broken PGP implementations
	part->ProcessSendCrypto(mode, true);

	unsigned long size = 0;
	if (FileBody(part, size))
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x

		// Create temporary input file
		PPx::FSObject fs_fin;
		PPx::FSObject* fin = &fs_fin;
		CreateTempFile(fin, mode, cdstring::null_str);
		StRemoveFileSpec _remove_fin(fin);

		cdstring fin_path(fin->GetPath());

		// Create temporary output file
		PPx::FSObject fs_fout;
		PPx::FSObject* fout = &fs_fout;
		CreateTempFile(fout, mode, cdstring::null_str);
		StRemoveFileSpec _remove_fout(fout);
		PPx::CFURL fout_url = fout->GetURL();
#else 
		cdstring fin;
		CreateTempFile(fin, mode, cdstring::null_str);
		StRemoveFileSpec _remove_fin(fin);
		cdstring fin_path = fin;

		cdstring fout;
		CreateTempFile(fout, mode, cdstring::null_str);
		StRemoveFileSpec _remove_fout(fout);
#endif

		// Write it to a stream
		{
			cdofstream outs(fin_path, std::ios_base::out|std::ios_base::trunc|std::ios_base::binary);
			unsigned long level = 0;
			costream stream_out(&outs, lendl);
			part->WriteToStream(stream_out, level, false, nil);
		}

		// Now process file
		Process(msg, mode, NULL, fin, key, NULL, fout, NULL, true, false);

		// Create part containing PGP data - this takes ownership of the temp file
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		*fout = PPx::FSObject(fout_url);
		generated_part = new CFileAttachment(*fout);
#else
		generated_part = new CFileAttachment(fout);
#endif
		static_cast<CFileAttachment*>(generated_part)->SetDeleteFile(true);
		
		// Always NULL the file name to prevent temp file names leaking into MIME parameters
		generated_part->GetContent().SetMappedName(cdstring::null_str);
		
		// Make sure output file is not deleted via stack remove
		_remove_fout.release();
	}
	else
	{
		cdstring data;

		// Write it to a stream
		{
			std::ostrstream outs;
			unsigned long level = 0;
			costream stream_out(&outs, lendl);
			part->WriteToStream(stream_out, level, false, nil);
			outs << std::ends;
			data.steal(outs.str());
		}

		char* out = nil;
		unsigned long out_len = 0;
		Process(msg, mode, data, NULL, key, &out, NULL, &out_len, true, false);

		// Make a copy of the data
		char* local = new char[out_len + 1];
		::memcpy(local, out, out_len);
		local[out_len] = 0;

		DisposeData(out);

		// Create part containg PGP data
		generated_part = new CDataAttachment;
		generated_part->SetData(local);
	}

	// Now pocess into PGP/MIME

	// Get appropriate MIME params
	SMIMEMultiInfo mime;
	switch(mode)
	{
	case eSign:
		GetMIMESign(&mime);
		break;
	case eEncrypt:
		GetMIMEEncrypt(&mime);
		break;
	case eEncryptSign:
		GetMIMEEncryptSign(&mime);
		break;
	default:;
	}
	
	// Determine whether multipart format can be used
	bool use_multi_part = !::strcmpnocase(mime.multipart.type, "multipart");

	// Process PGP data part
	ApplyMIME(generated_part, &mime.second);
	
	// The content in the generated part is already transfer encoded so we must
	// not re-apply the encoding
	generated_part->GetContent().SetDontEncode();

	// Create actual message structure
	if (use_multi_part)
	{
		// Create the top-level multipart
		CAttachment* multi_part = new CAttachment;
		ApplyMIME(multi_part, &mime.multipart);

		// Give it to the message
		msg->SetBody(multi_part, false);

		// Now add subparts to top part
		switch(mode)
		{
		case eSign:
			// multipart/signed
			// 	type/subtype
			//	application/pgp-signature
			multi_part->AddPart(part);
			multi_part->AddPart(generated_part);
			break;
		case eEncrypt:
		case eEncryptSign:
			{
				// multipart/encrypted
				// 	application/pgp-encrypted
				//	application/octet-stream

				// Delete original part
				delete part;

				// Create new encryption part
				CDataAttachment* version_part = new CDataAttachment;
				ApplyMIME(version_part, &mime.first);

				cdstring temp("Version: 1");
				temp += os_endl;
				version_part->SetData(temp.grab_c_str());

				// Add the parts now
				multi_part->AddPart(version_part);
				multi_part->AddPart(generated_part);
				
				// NB last part must be set to CTE of 7bit
				generated_part->GetContent().SetTransferEncoding(e7bitEncoding);
			}
			break;
		default:;
		}
	}
	else
	{
		// Delete original part
		delete part;

		// Give new generated part to the message
		msg->SetBody(generated_part, false);
	}
}

// Determine whether body needs to be spooled to file
bool CSecurityPlugin::FileBody(const CAttachment* part, unsigned long& size) const
{
	// See if multipart
	if (part->IsMultipart() && !part->IsMessage() && part->GetParts())
	{
		for(CAttachmentList::iterator iter = part->GetParts()->begin(); iter != part->GetParts()->end(); iter++)
		{
			if (FileBody(*iter, size))
				return true;
		}
		
		return false;
	}
	else if (part->IsMessage())
		return FileBody(part->GetMessage()->GetBody(), size);
	else
	{
		// Files always require file processing
		if (FileAttachment(part))
			return true;

		// Total size > 64K require file processing
		size += part->GetSize();

		return (size >= cFileThreshold);
	}
}

#pragma mark ____________________________Operations on single parts

bool CSecurityPlugin::CanSecureAttachment(const CAttachment* part) const
{
	return part->CanChange();
}

bool CSecurityPlugin::FileAttachment(const CAttachment* part) const
{
	return typeid(*part) == typeid(CFileAttachment);
}

void CSecurityPlugin::ProcessAttachment(CMessage* msg, CAttachment* part, ESecureMessage mode, const char* key)
{
	// See if multipart
	if (part->IsMultipart() && !part->IsMessage() && !part->IsApplefile() && part->GetParts())
	{
		for(CAttachmentList::iterator iter = part->GetParts()->begin(); iter != part->GetParts()->end(); iter++)
			ProcessAttachment(msg, *iter, mode, key);
	}
	else if (part->IsMessage())
		ProcessAttachment(part->GetMessage(), part->GetMessage()->GetBody(), mode, key);
	else
	{
		// Only do those that are modifiable
		if (!CanSecureAttachment(part))
			return;

		// Special processing for crypto - only needed for broken PGP implementations
		part->ProcessSendCrypto(mode, false);

		// Check for memory or file based attachment
		if (FileAttachment(part))
		{
#if __dest_os == __mac_os || __dest_os == __mac_os_x

			// Create temporary output file
			PPx::FSObject fs_fout;
			PPx::FSObject* fout = &fs_fout;
			cdstring old_name = static_cast<CFileAttachment*>(part)->GetFSSpec()->GetName();
			CreateTempFile(fout, mode, old_name);
			StRemoveFileSpec _remove_fout(fout);
			PPx::CFURL fout_url = fout->GetURL();

#else
			// Create temporary output file
			cdstring fout;
			cdstring old_path = static_cast<CFileAttachment*>(part)->GetFilePath();
			cdstring old_name;
			if (::strrchr(old_path.c_str(), os_dir_delim) != NULL)
				old_name = ::strrchr(old_path.c_str(), os_dir_delim) + 1;
			else
				old_name = old_path;
			
			CreateTempFile(fout, mode, old_name);
			StRemoveFileSpec _remove_fout(fout);
#endif

			// For file attachments we always create a detatched signature of the original file data on disk
			// allowing users to save the file part and the signature part to disk and to then verify that using
			// the desktop PGP tool. Thus we need to turn on the PGP/MIME behaviour for signing here to get the 
			// detached signature.
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			Process(msg, mode, NULL, static_cast<CFileAttachment*>(part)->GetFSSpec(), key, NULL, fout, NULL, mode == eSign, !part->IsText());
#else
			Process(msg, mode, NULL, static_cast<CFileAttachment*>(part)->GetFilePath(), key, NULL, fout, NULL, mode == eSign, !part->IsText());
#endif

			// Get appropriate MIME params
			SMIMEMultiInfo mime;
			switch(mode)
			{
			case eSign:
				GetMIMESign(&mime);
				break;
			case eEncrypt:
				GetMIMEEncrypt(&mime);
				break;
			case eEncryptSign:
				GetMIMEEncryptSign(&mime);
				break;
			default:;
			}
	
			// Now have detached file - decide what to do
			switch(mode)
			{
			case eSign:
			{
				// Turn into multipart and add detached signature
				CDataAttachment* mattach = new CDataAttachment;
				mattach->GetContent().SetContent(eContentMultipart, eContentSubMixed);

				// Attachment takes ownership of temp file
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				*fout = PPx::FSObject(fout_url);
				CFileAttachment* fattach = new CFileAttachment(*fout);
#else
				CFileAttachment* fattach = new CFileAttachment(fout);
#endif
				fattach->SetDeleteFile(true);
		
				// Always NULL the file name to prevent temp file names leaking into MIME parameters
				fattach->GetContent().SetMappedName(cdstring::null_str);
				
				// Make sure output file is not deleted via stack remove
				_remove_fout.release();

				ApplyMIME(fattach, &mime.second);

				// See if it has a parent
				if (part->GetParent())
				{
					CAttachment* pattach = part->GetParent();

					// Get index of part within parent
					unsigned long index = pattach->GetParts() ? pattach->GetParts()->FetchIndexOf(part) : 0;
					if (index)
					{
						// Remove existing part
						index--;
						pattach->RemovePart(part, false);

						// Add in multipart at old position
						pattach->AddPart(mattach, index);

						// Now add the sub-parts
						mattach->AddPart(part);
						mattach->AddPart(fattach);
					}
				}
				else
				{
					// Give multipart to message
					msg->SetBody(mattach, false);

					// Now add the sub-parts
					mattach->AddPart(part);
					mattach->AddPart(fattach);
				}
				break;
			}
			case eEncrypt:
			case eEncryptSign:
				// Special for AppleDouble - must do before setting FSSpec
				if (part->IsMultipart() && part->IsApplefile() && part->GetParts())
				{
					part->RemovePart(part->GetParts()->front());
					part->RemovePart(part->GetParts()->front());
				}

				// Replace existing part with new file - takes ownership of temp file
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				*fout = PPx::FSObject(fout_url);
				static_cast<CFileAttachment*>(part)->SetFSSpec(*fout);
#else
				static_cast<CFileAttachment*>(part)->SetFilePath(fout);
#endif
				static_cast<CFileAttachment*>(part)->SetDeleteFile(true);
		
				// Always NULL the file name to prevent temp file names leaking into MIME parameters
				part->GetContent().SetMappedName(cdstring::null_str);
				
				// Make sure output file is not deleted via stack remove
				_remove_fout.release();

				// Convert to stand alone application/pgp-encrypted
				ApplyMIME(part, &mime.first);
				break;
			default:;
			}
		}
		else
		{
			// Process data through plugin
			char* out = nil;
			unsigned long out_len = 0;

			Process(msg, mode, part->GetData(), NULL, key, &out, NULL, &out_len, false, !part->IsText());

			// Make a copy of the data
			char* local = new char[out_len + 1];
			::memcpy(local, out, out_len);
			local[out_len] = 0;

			DisposeData(out);

			part->SetData(local);
		}
	}
}

#pragma mark ____________________________Process some data

void CSecurityPlugin::Process(const CMessage* msg,
								ESecureMessage mode,
								const char* in,
								fspec fin,
								const char* key,
								char** out,
								fspec fout,
								unsigned long* out_len,
								bool useMIME,
								bool binary)
{
	bool do_data = (in != NULL);

	switch(mode)
	{
	case eSign:
		if (do_data)
		{
			// Clear sign data
			if (SignData(in, key, out, out_len, useMIME, binary) != 1)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}
		}
		else
		{
			// Clear sign file
			if (SignFile(fin, key, fout, useMIME, binary) != 1)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}
		}
		break;

	case eEncrypt:
		{
			// Create array of keys
			cdstrvect keylist;

			// Now add all keys
			for(CAddressList::const_iterator iter =  msg->GetEnvelope()->GetTo()->begin(); iter !=  msg->GetEnvelope()->GetTo()->end(); iter++)
				keylist.push_back((*iter)->GetMailAddress().c_str());
			for(CAddressList::const_iterator iter =  msg->GetEnvelope()->GetCC()->begin(); iter !=  msg->GetEnvelope()->GetCC()->end(); iter++)
				keylist.push_back((*iter)->GetMailAddress().c_str());
			for(CAddressList::const_iterator iter =  msg->GetEnvelope()->GetBcc()->begin(); iter !=  msg->GetEnvelope()->GetBcc()->end(); iter++)
				keylist.push_back((*iter)->GetMailAddress().c_str());
			if (CPreferences::sPrefs->mEncryptToSelf.GetValue())
				keylist.push_back(key);

			// Eliminate duplicates then create pointer array
			std::sort(keylist.begin(), keylist.end());
			keylist.erase(std::unique(keylist.begin(), keylist.end()), keylist.end());
			const char** key_list = cdstring::ToArray(keylist);

			try
			{
				if (do_data)
				{
					// Clear sign data
					if (EncryptData(in, key_list, out, out_len, useMIME, binary) != 1)
					{
						CLOG_LOGTHROW(CGeneralException, -1);
						throw CGeneralException(-1);
					}
				}
				else
				{
					// Clear sign data
					if (EncryptFile(fin, key_list, fout, useMIME, binary) != 1)
					{
						CLOG_LOGTHROW(CGeneralException, -1);
						throw CGeneralException(-1);
					}
				}
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Always delete key list
				cdstring::FreeArray(key_list);

				CLOG_LOGRETHROW;
				throw;
			}

			// Delete key list
			cdstring::FreeArray(key_list);
		}
		break;

	case eEncryptSign:
		{
			// Create array of keys
			cdstrvect keylist;

			// Now add all keys
			for(CAddressList::const_iterator iter =  msg->GetEnvelope()->GetTo()->begin(); iter !=  msg->GetEnvelope()->GetTo()->end(); iter++)
				keylist.push_back((*iter)->GetMailAddress().c_str());
			for(CAddressList::const_iterator iter =  msg->GetEnvelope()->GetCC()->begin(); iter !=  msg->GetEnvelope()->GetCC()->end(); iter++)
				keylist.push_back((*iter)->GetMailAddress().c_str());
			for(CAddressList::const_iterator iter =  msg->GetEnvelope()->GetBcc()->begin(); iter !=  msg->GetEnvelope()->GetBcc()->end(); iter++)
				keylist.push_back((*iter)->GetMailAddress().c_str());
			if (CPreferences::sPrefs->mEncryptToSelf.GetValue())
				keylist.push_back(key);

			// Eliminate duplicates then create pointer array
			std::sort(keylist.begin(), keylist.end());
			keylist.erase(std::unique(keylist.begin(), keylist.end()), keylist.end());
			const char** key_list = cdstring::ToArray(keylist);

			try
			{
				if (do_data)
				{
					// Clear sign data
					if (EncryptSignData(in, key_list, key, out, out_len, useMIME, binary) != 1)
					{
						CLOG_LOGTHROW(CGeneralException, -1);
						throw CGeneralException(-1);
					}
				}
				else
				{
					// Clear sign data
					if (EncryptSignFile(fin, key_list, key, fout, useMIME, binary) != 1)
					{
						CLOG_LOGTHROW(CGeneralException, -1);
						throw CGeneralException(-1);
					}
				}
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Always delete key list
				cdstring::FreeArray(key_list);

				CLOG_LOGRETHROW;
				throw;
			}

			// Delete key list
			cdstring::FreeArray(key_list);
		}
		break;
	default:;
	}
}

#pragma mark ____________________________Verify/decrypt static apis

// Only called for inline parts
bool CSecurityPlugin::VerifyDecryptPart(CMessage* msg, CAttachment* part, CMessageCryptoInfo& info)
{
	CSecurityPlugin* splugin = NULL;

	try
	{
		// Is message top part multipart/signed
		if (msg->GetBody() && msg->GetBody()->IsSigned() &&
			msg->GetBody()->GetParts() && (msg->GetBody()->GetParts()->size() == 2))
		{
			// Get the protocol parameter
			const cdstring& protocol = msg->GetBody()->GetContent().GetContentParameter(cMIMEParameter[eCryptoProtocol]);
			
			// Get suitable plugin for verify
			splugin = GetVerifyPlugin(protocol);

			// Check for valid sigtype
			if (!splugin)
			{
				// Provide sensible indication for missing parameter
				cdstring err_protocol(protocol);
				if (err_protocol.empty())
					err_protocol = "missing protocol parameter";
				
				// Do error to indicate unsupported protocol
				cdstring errstr;
				errstr.FromResource("Alerts::Message::UNKNOWN_CRYPTO_SHORT");
				errstr.Substitute(err_protocol);
				info.SetError(errstr);

				// Only show alert if requested by user
				if (CPreferences::sPrefs->mUseErrorAlerts.GetValue())
					CErrorHandler::PutStopAlertRsrcStr("Alerts::Message::UNKNOWN_CRYPTO", err_protocol);
				return false;
			}
		}

		// Is parent a multipart/encrypted
		else if (msg->GetBody() && msg->GetBody()->IsEncrypted() &&
					msg->GetBody()->GetParts() && (msg->GetBody()->GetParts()->size() == 2))
		{

			// Get the protocol parameter
			const cdstring& protocol = msg->GetBody()->GetContent().GetContentParameter(cMIMEParameter[eCryptoProtocol]);
			
			// Get suitable plugin for verify
			splugin = GetDecryptPlugin(protocol);

			// Check for valid sigtype
			if (!splugin)
			{
				// Provide sensible indication for missing parameter
				cdstring err_protocol(protocol);
				if (err_protocol.empty())
					err_protocol = "missing protocol parameter";
				
				// Do error to indicate unsupported protocol
				cdstring errstr;
				errstr.FromResource("Alerts::Message::UNKNOWN_CRYPTO_SHORT");
				errstr.Substitute(err_protocol);
				info.SetError(errstr);

				// Only show alert if requested by user
				if (CPreferences::sPrefs->mUseErrorAlerts.GetValue())
					CErrorHandler::PutStopAlertRsrcStr("Alerts::Message::UNKNOWN_CRYPTO", err_protocol);
				return false;
			}
		}
		
		// Is parent application/(x-)pkcs7-mime
		else if (msg->GetBody() && msg->GetBody()->IsDecryptable())
		{
			// Get the protocol parameter
			cdstring type = CMIMESupport::GenerateContentHeader(msg->GetBody(), false, lendl, false);
			
			// Get suitable plugin for verify
			splugin = GetDecryptPlugin(type);

			// Check for valid sigtype
			if (!splugin)
			{
				// Provide sensible indication for missing parameter
				cdstring err_protocol(type);
				if (err_protocol.empty())
					err_protocol = "missing content type";
				
				// Do error to indicate unsupported protocol
				cdstring errstr;
				errstr.FromResource("Alerts::Message::UNKNOWN_CRYPTO_SHORT");
				errstr.Substitute(err_protocol);
				info.SetError(errstr);

				// Only show alert if requested by user
				if (CPreferences::sPrefs->mUseErrorAlerts.GetValue())
					CErrorHandler::PutStopAlertRsrcStr("Alerts::Message::UNKNOWN_CRYPTO", err_protocol);
				return false;
			}
		}
		
		else if (part)
		{
			// Must be inline PGP/GPG
			splugin = GetRegisteredPlugin(cGPGName);
			if (!splugin)
				splugin = GetRegisteredPlugin(cPGPName);

			// Check for valid sigtype
			if (!splugin)
			{
				// Provide sensible indication for missing parameter
				cdstring err_protocol("cannot process inline content");
				
				// Do error to indicate unsupported protocol
				cdstring errstr;
				errstr.FromResource("Alerts::Message::UNKNOWN_CRYPTO_SHORT");
				errstr.Substitute(err_protocol);
				info.SetError(errstr);

				// Only show alert if requested by user
				if (CPreferences::sPrefs->mUseErrorAlerts.GetValue())
					CErrorHandler::PutStopAlertRsrcStr("Alerts::Message::UNKNOWN_CRYPTO", err_protocol);
				return false;
			}
		}
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}

	return splugin ? splugin->VerifyDecryptPartInternal(msg, part, info) : false;
}

CSecurityPlugin* CSecurityPlugin::GetVerifyPlugin(const cdstring& type)
{
	// Force reset of cache if preferred plugin has changed
	if (sPreferredPlugin != CPreferences::sPrefs->mPreferredPlugin.GetValue())
	{
		sCanVerify.clear();
		sPreferredPlugin = CPreferences::sPrefs->mPreferredPlugin.GetValue();
	}

	// See whether result is already cached
	cdstrmap::const_iterator found = sCanVerify.find(type);
	if (found != sCanVerify.end())
	{
		if ((*found).second.length())
			return GetRegisteredPlugin((*found).second);
		else
			return NULL;
	}
	
	// First try the default plugin specified in the preferences. This will ensure that if both PGP and GPG
	// are installed, then the one from the prefs will be picked for PGP verifications
	CSecurityPlugin* splugin = GetRegisteredPlugin(CPreferences::sPrefs->mPreferredPlugin.GetValue());
	if ((splugin != NULL) && (splugin->CanVerifyThis(type) == 0))
	{
		// cache it and return
		sCanVerify.insert(cdstrmap::value_type(type, splugin->GetName()));
		return splugin;
	}

	// Must do lookup using each plugin
	for(SSecurityPluginHandlers::iterator iter = sSecurityPlugins.begin(); iter != sSecurityPlugins.end(); iter++)
	{
		// Try each one
		splugin = (*iter).second;
		if (splugin->CanVerifyThis(type) == 0)
		{
			// cache it and return
			sCanVerify.insert(cdstrmap::value_type(type, (*iter).first));
			return splugin;
		}
	}
	
	// No handler found - cache this as NULL to prevent testing again
	sCanVerify.insert(cdstrmap::value_type(type, cdstring::null_str));
	return NULL;
}

CSecurityPlugin* CSecurityPlugin::GetDecryptPlugin(const cdstring& type)
{
	// Force reset of cache if preferred plugin has changed
	if (sPreferredPlugin != CPreferences::sPrefs->mPreferredPlugin.GetValue())
	{
		sCanDecrypt.clear();
		sPreferredPlugin = CPreferences::sPrefs->mPreferredPlugin.GetValue();
	}

	// See whether result is already cached
	cdstrmap::const_iterator found = sCanDecrypt.find(type);
	if (found != sCanDecrypt.end())
	{
		if ((*found).second.length())
			return GetRegisteredPlugin((*found).second);
		else
			return NULL;
	}
	
	// First try the default plugin specified in the preferences. This will ensure that if both PGP and GPG
	// are installed, then the one from the prefs will be picked for PGP decryptions
	CSecurityPlugin* splugin = GetRegisteredPlugin(CPreferences::sPrefs->mPreferredPlugin.GetValue());
	if ((splugin != NULL) && (splugin->CanDecryptThis(type) == 0))
	{
		// cache it and return
		sCanDecrypt.insert(cdstrmap::value_type(type, splugin->GetName()));
		return splugin;
	}

	// Must do lookup using each plugin
	for(SSecurityPluginHandlers::iterator iter = sSecurityPlugins.begin(); iter != sSecurityPlugins.end(); iter++)
	{
		// Try each one
		splugin = (*iter).second;
		if (splugin->CanDecryptThis(type) == 0)
		{
			// cache it and return
			sCanDecrypt.insert(cdstrmap::value_type(type, (*iter).first));
			return splugin;
		}
	}
	
	// No handler found - cache this as NULL to prevent testing again
	sCanDecrypt.insert(cdstrmap::value_type(type, cdstring::null_str));
	return NULL;
}

#pragma mark ____________________________Verify/decrypt local apis

// Only called for inline parts
bool CSecurityPlugin::VerifyDecryptPartInternal(CMessage* msg, CAttachment* part, CMessageCryptoInfo& info)
{
	// Load plugin
	StLoadPlugin load(this);

	bool result = false;
	const char* old_data = NULL;
	CAttachment* temp_attach = NULL;
	bool remove_temp_data = false;
	try
	{
		// Is message top part multipart/signed
		if (msg->GetBody() &&
			(msg->GetBody()->GetContent().GetContentType() == eContentMultipart) &&
			(msg->GetBody()->GetContent().GetContentSubtype() == eContentSubSigned) &&
			msg->GetBody()->GetParts() &&
			(msg->GetBody()->GetParts()->size() == 2))
		{
			// Check message size first
			if (!CMailControl::CheckSizeWarning(msg, true))
				return false;

			// Do verification
			result = VerifyMessage(msg, info);

			// Now see if signature failed
			if (!result)
				HandleError(&info);
		}

		// Is parent a multipart/encrypted
		else if (msg->GetBody() &&
			(msg->GetBody()->GetContent().GetContentType() == eContentMultipart) &&
			(msg->GetBody()->GetContent().GetContentSubtype() == eContentSubEncrypted) &&
			msg->GetBody()->GetParts() &&
			(msg->GetBody()->GetParts()->size() == 2))
		{
			// Check message size first
			if (!CMailControl::CheckSizeWarning(msg, true))
				return false;

			// Do decrypt
			result = DecryptMessage(msg, info, true);

			// Now see if signature failed
			if (!result)
				HandleError(&info);
		}
		
		// Is parent application/(x-)pkcs7-mime
		else if (msg->GetBody() && msg->GetBody()->IsDecryptable())
		{
			// Check message size first
			if (!CMailControl::CheckSizeWarning(msg, true))
				return false;

			// Do decrypt
			result = DecryptMessage(msg, info, false);

			// Now see if signature failed
			if (!result)
				HandleError(&info);
		}
		
		else if (part)
		{
			cdstring from;
			if (msg->GetEnvelope() && msg->GetEnvelope()->GetFrom() && (msg->GetEnvelope()->GetFrom()->size() != 0))
				from = msg->GetEnvelope()->GetFrom()->front()->GetMailAddress();

			// Just use current data in this part
			const char* in = part->GetData();
			char* out = NULL;
			unsigned long out_len = 0;
			char* signed_by = NULL;
			char* encrypted_to = NULL;
			bool did_signature = false;
			bool signature_ok = false;
			if (DecryptVerifyData(in, NULL, from, &out, &out_len, &signed_by, &encrypted_to, &result, &did_signature, &signature_ok, !part->IsText()) != 1)
			{
				info.SetSuccess(false);
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}
			info.SetSuccess(result);
			info.SetDidSignature(did_signature);
			info.SetSignatureOK(signature_ok);

			// Get signed by info
			if (info.GetDidSignature() && signed_by)
				cdstring::FromArray((const char**) signed_by, info.GetSignedBy());

			// Get encrypted to info
			cdstrvect encryptedTo;
			if (encrypted_to)
			{
				info.SetDidDecrypt(true);
				cdstring::FromArray((const char**) encrypted_to, info.GetEncryptedTo());
			}

			// Add data to part, remove any old cached data
			if (result)
			{
				// Replace existing part data with output
				if (out)
					part->SetData(::strdup(out));
			}
			else
				HandleError(&info);

			DisposeData(out);
		}

	}
	catch(CNetworkException& ex)
	{
		CLOG_LOGCATCH(CNetworkException&);

		// Must throw out if disconnected/reconnected because
		// message object is no longer valid
		if (ex.disconnected() || ex.reconnected())
			throw;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		HandleError(&info);
		result = false;
	}

	return result;
}

// Determine whether body needs to be spooled to file
bool CSecurityPlugin::FileVerifyDecrypt(const CMessage* msg) const
{
	// Do based on size of message
	return (msg->GetSize() > cFileThreshold);
}

// Verify multipart/signed
bool CSecurityPlugin::VerifyMessage(CMessage* msg, CMessageCryptoInfo& info)
{
	bool result = false;
	const char* old_data = NULL;
	CAttachment* temp_attach = NULL;
	bool remove_temp_data = false;

	// See whether to use file or not
	if (FileVerifyDecrypt(msg))
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Create temporary input file
		PPx::FSObject fs_fin;
		PPx::FSObject* fin = &fs_fin;
		CreateTempFile(fin, eNone, cdstring::null_str);
		StRemoveFileSpec _remove_fin(fin);
		cdstring fin_path(fin->GetPath());

		// Create temporary data file
		PPx::FSObject fs_fin_d;
		PPx::FSObject* fin_d = &fs_fin_d;
		CreateTempFile(fin_d, eNone, cdstring::null_str);
		StRemoveFileSpec _remove_fin_d(fin_d);
		cdstring fin_d_path(fin_d->GetPath());
#else
		// Create temporary input file
		cdstring fin;
		CreateTempFile(fin, eNone, cdstring::null_str);
		StRemoveFileSpec _remove_fin(fin);
		cdstring fin_path = fin;

		// Create temporary data file
		cdstring fin_d;
		CreateTempFile(fin_d, eNone, cdstring::null_str);
		StRemoveFileSpec _remove_fin_d(fin_d);
		cdstring fin_d_path = fin_d;
#endif

		{
			// Create the temporary file
			cdofstream finstream(fin_path, std::ios_base::out|std::ios_base::trunc|std::ios_base::binary);
			costream stream_out(&finstream, eEndl_CRLF);
			msg->WriteToStream(stream_out, false, NULL);
		}

		// Try to parse it out as a local message
		cdstring sig;
		{
			cdifstream buf_in(fin_path, std::ios_base::in|std::ios_base::binary);
			CRFC822Parser parser;
			std::auto_ptr<CLocalMessage> lmsg(parser.MessageFromStream(buf_in));
			buf_in.clear();

			// Must have message
			if (!lmsg.get() ||
				!lmsg->GetBody()->GetParts() ||
				(lmsg->GetBody()->GetParts()->size() != 2))
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}

			// Now get pointers to relevant bits
			CAttachmentList* parts = lmsg->GetBody()->GetParts();
			unsigned long data_start = static_cast<CLocalAttachment*>(parts->at(0))->GetIndexStart();
			unsigned long data_length = static_cast<CLocalAttachment*>(parts->at(0))->GetIndexLength();
			unsigned long sig_start = static_cast<CLocalAttachment*>(parts->at(1))->GetIndexBodyStart();
			unsigned long sig_length = static_cast<CLocalAttachment*>(parts->at(1))->GetIndexBodyLength();
			
			// Write data into another temp file
			{
				cdofstream buf_out(fin_d_path, std::ios_base::out|std::ios_base::trunc|std::ios_base::binary);
				::StreamCopy(buf_in, buf_out, data_start, data_length);
			}
			
			// Grab signature into internal buffer
			// NB Must get the signature data via the message to ensure MIME decoding has taken place for PGP only
			{
				CAttachment* sig_part = msg->GetBody()->GetParts()->at(1);
				msg->ReadAttachment(sig_part, true, GetName() != cSMIMEName);
				sig = sig_part->GetData();
			}
		}

		cdstring from;
		if (msg->GetEnvelope() && msg->GetEnvelope()->GetFrom() && (msg->GetEnvelope()->GetFrom()->size() != 0))
			from = msg->GetEnvelope()->GetFrom()->front()->GetMailAddress();

		char* signed_by = NULL;
		char* encrypted_to = NULL;
		bool did_signature = false;
		bool signature_ok = false;
		if (DecryptVerifyFile(fin_d, sig, from, NULL, &signed_by, &encrypted_to, &result, &did_signature, &signature_ok, true) != 1)
		{
			info.SetSuccess(false);
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
		info.SetSuccess(result);
		info.SetDidSignature(did_signature);
		info.SetSignatureOK(signature_ok);

		// Get signed by info
		if (info.GetDidSignature() && signed_by)
			cdstring::FromArray((const char**) signed_by, info.GetSignedBy());

		// Get encrypted to info
		cdstrvect encryptedTo;
		if (encrypted_to)
		{
			info.SetDidDecrypt(true);
			cdstring::FromArray((const char**) encrypted_to, info.GetEncryptedTo());
		}
	}
	else
	{
		// Read raw message data into temp buffer
		std::ostrstream buf;
		costream stream_out(&buf, eEndl_CRLF);
		msg->WriteToStream(stream_out, false, NULL);
		buf << std::ends;
		const char* in = buf.str();
		buf.freeze(false);

		// Try to parse it out as a local message
		std::istrstream buf_in(in);
		CRFC822Parser parser;
		std::auto_ptr<CLocalMessage> lmsg(parser.MessageFromStream(buf_in));

		// Must have message
		if (!lmsg.get() ||
			!lmsg->GetBody()->GetParts() ||
			(lmsg->GetBody()->GetParts()->size() != 2))
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}

		// Now get pointers to relevant bits
		CAttachmentList* parts = lmsg->GetBody()->GetParts();
		unsigned long data_start = static_cast<CLocalAttachment*>(parts->at(0))->GetIndexStart();
		unsigned long data_length = static_cast<CLocalAttachment*>(parts->at(0))->GetIndexLength();
		unsigned long sig_start = static_cast<CLocalAttachment*>(parts->at(1))->GetIndexBodyStart();
		unsigned long sig_length = static_cast<CLocalAttachment*>(parts->at(1))->GetIndexBodyLength();
		
		cdstring from;
		if (msg->GetEnvelope() && msg->GetEnvelope()->GetFrom() && (msg->GetEnvelope()->GetFrom()->size() != 0))
			from = msg->GetEnvelope()->GetFrom()->front()->GetMailAddress();

		// Now form strings
		const char* data = in + data_start;
		const_cast<char*>(data)[data_length] = 0;

		// Grab signature into internal buffer
		// NB Must get the signature data via the message to ensure MIME decoding has taken place for PGP only
		cdstring sig;
		{
			CAttachment* sig_part = msg->GetBody()->GetParts()->at(1);
			msg->ReadAttachment(sig_part, true, GetName() != cSMIMEName);
			sig = sig_part->GetData();
		}

		char* signed_by = NULL;
		char* encrypted_to = NULL;
		bool did_signature = false;
		bool signature_ok = false;
		if (DecryptVerifyData(data, sig, from, NULL, NULL, &signed_by, &encrypted_to, &result, &did_signature, &signature_ok, true) != 1)
		{
			info.SetSuccess(false);
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
		info.SetSuccess(result);
		info.SetDidSignature(did_signature);
		info.SetSignatureOK(signature_ok);

		// Get signed by info
		if (info.GetDidSignature() && signed_by)
			cdstring::FromArray((const char**) signed_by, info.GetSignedBy());

		// Get encrypted to info
		cdstrvect encryptedTo;
		if (encrypted_to)
		{
			info.SetDidDecrypt(true);
			cdstring::FromArray((const char**) encrypted_to, info.GetEncryptedTo());
		}
	}

	return result;
}

// Decrypt multipart/encrypted
bool CSecurityPlugin::DecryptMessage(CMessage* msg, CMessageCryptoInfo& info, bool use_multi_part)
{
	bool result = false;
	const char* old_data = NULL;
	CAttachment* temp_attach = NULL;
	bool remove_temp_data = false;

	// See whether to use file or not
	if (FileVerifyDecrypt(msg))
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Create temporary input file
		PPx::FSObject fs_fin;
		PPx::FSObject* fin = &fs_fin;
		CreateTempFile(fin, eNone, cdstring::null_str);
		StRemoveFileSpec _remove_fin(fin);
		cdstring fin_path(fin->GetPath());

		// Create temporary data file
		PPx::FSObject fs_fin_d;
		PPx::FSObject* fin_d = &fs_fin_d;
		CreateTempFile(fin_d, eNone, cdstring::null_str);
		StRemoveFileSpec _remove_fin_d(fin_d);
		cdstring fin_d_path(fin_d->GetPath());
#else
		// Create temporary input file
		cdstring fin;
		CreateTempFile(fin, eNone, cdstring::null_str);
		StRemoveFileSpec _remove_fin(fin);
		cdstring fin_path = fin;

		// Create temporary data file
		cdstring fin_d;
		CreateTempFile(fin_d, eNone, cdstring::null_str);
		StRemoveFileSpec _remove_fin_d(fin_d);
		cdstring fin_d_path = fin_d;
#endif

		{
			// Get attachment to write to disk
			CAttachment* part2 = NULL;
			if (use_multi_part)
				part2 = msg->GetBody()->GetParts()->at(1);
			else
				part2 = msg->GetBody();

			// Create the temporary file
			cdofstream finstream(fin_path, std::ios_base::out|std::ios_base::trunc|std::ios_base::binary);
			
			// Get encoding filter
			std::auto_ptr<CStreamFilter> filter;

			// May need to filter  - SMIME always gets raw base64 data
			if (GetName() != cSMIMEName)
			{
				switch(part2->GetContent().GetTransferEncoding())
				{
				case eNoTransferEncoding:
				case e7bitEncoding:
				case e8bitEncoding:
					// Do nothing
					break;
				case eQuotedPrintableEncoding:
					// Convert from QP
					filter.reset(new CStreamFilter(new mime_qp_filterbuf(false)));
					filter->SetStream(&finstream);
					break;
				case eBase64Encoding:
					// Convert from base64
					filter.reset(new CStreamFilter(new mime_base64_filterbuf(false)));
					filter->SetStream(&finstream);
					break;
				default:;
				}
			}

			costream stream_out(filter.get() ? static_cast<std::ostream*>(filter.get()) : static_cast<std::ostream*>(&finstream), eEndl_CRLF);
			part2->WriteDataToStream(stream_out, false, NULL, msg);
		}

		cdstring from;
		if (msg->GetEnvelope() && msg->GetEnvelope()->GetFrom() && (msg->GetEnvelope()->GetFrom()->size() != 0))
			from = msg->GetEnvelope()->GetFrom()->front()->GetMailAddress();

		char* signed_by = NULL;
		char* encrypted_to = NULL;
		bool did_signature = false;
		bool signature_ok = false;
		if (DecryptVerifyFile(fin, NULL, from, fin_d, &signed_by, &encrypted_to, &result, &did_signature, &signature_ok, false) != 1)
		{
			info.SetSuccess(false);
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
		info.SetSuccess(result);
		info.SetDidSignature(did_signature);
		info.SetSignatureOK(signature_ok);

		// Get signed by info
		if (info.GetDidSignature() && signed_by)
			cdstring::FromArray((const char**) signed_by, info.GetSignedBy());

		// Get encrypted to info
		cdstrvect encryptedTo;
		if (encrypted_to)
		{
			info.SetDidDecrypt(true);
			cdstring::FromArray((const char**) encrypted_to, info.GetEncryptedTo());
		}

		// Add data to part, remove any old cached data
		if (result)
		{
			// Replace existing part data with output
			// Create fstream data
			std::auto_ptr<cdifstream> stream(new cdifstream(fin_d_path, std::ios_base::in|std::ios_base::binary));
			
			// Parse RFC822 parts
			CRFC822Parser parser(true, msg);
			CAttachment* new_body = parser.AttachmentFromStream(*stream, NULL);
			static_cast<CStreamAttachment*>(new_body)->SetStream(stream.get(), NULL, fin_d_path);
			msg->ReplaceBody(static_cast<CStreamAttachment*>(new_body));
			
			// Stream & temp file are now owned by attachment
			_remove_fin_d.release();
			stream.release();
		}
		else
			HandleError(&info);

	}
	else
	{
		// Get the encrypted data part
		CAttachment* part2 = NULL;
		if (use_multi_part)
			part2 = msg->GetBody()->GetParts()->at(1);
		else
			part2 = msg->GetBody();

		// Make sure its treated as text even though its application/octet-stream
		part2->SetFakeText(true);
		part2->GetContent().SetDontEncode();

		cdstring from;
		if (msg->GetEnvelope() && msg->GetEnvelope()->GetFrom() && (msg->GetEnvelope()->GetFrom()->size() != 0))
			from = msg->GetEnvelope()->GetFrom()->front()->GetMailAddress();

		// Just use current data in this part
		msg->ReadAttachment(part2);
		const char* in = part2->GetData();
		char* out = NULL;
		unsigned long out_len = 0;
		char* signed_by = NULL;
		char* encrypted_to = NULL;
		bool did_signature = false;
		bool signature_ok = false;
		if (DecryptVerifyData(in, NULL, from, &out, &out_len, &signed_by, &encrypted_to, &result, &did_signature, &signature_ok, false) != 1)
		{
			info.SetSuccess(false);
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
		info.SetSuccess(result);
		info.SetDidSignature(did_signature);
		info.SetSignatureOK(signature_ok);

		// Get signed by info
		if (info.GetDidSignature() && signed_by)
			cdstring::FromArray((const char**) signed_by, info.GetSignedBy());

		// Get encrypted to info
		cdstrvect encryptedTo;
		if (encrypted_to)
		{
			info.SetDidDecrypt(true);
			cdstring::FromArray((const char**) encrypted_to, info.GetEncryptedTo());
		}

		// Add data to part, remove any old cached data
		if (result)
		{
			// Replace existing part data with output
			if (out)
			{
				// Create strstream data
				cdstring temp(out);
				std::auto_ptr<std::istrstream> stream(new std::istrstream(temp.c_str()));
				
				// Parse RFC822 parts
				CRFC822Parser parser(true, msg);
				CAttachment* new_body = parser.AttachmentFromStream(*stream, NULL);
				static_cast<CStreamAttachment*>(new_body)->SetStream(stream.get(), temp.grab_c_str(), cdstring::null_str);
				msg->ReplaceBody(static_cast<CStreamAttachment*>(new_body));
				
				// Stream is now owned by attachment
				stream.release();
			}
		}
		else
			HandleError(&info);

		DisposeData(out);
	}

	// Look for signed content after decrypt (i.e. original was signed then encrypted)
	if (result && msg->GetBody()->IsVerifiable())
	{
		// First make sure the message header is cached as it will be needed when writing to stream
		msg->GetHeader();

		// Need to create a temporarily remove the message from its mailbox so that WriteToStream writes the parts
		// to stream rather than tries to get raw message from mailbox and write that
		CMbox* mboxold = msg->GetMbox();
		msg->SetMbox(NULL);

		try
		{
			// Now verify signature
			CMessageCryptoInfo info2;
			result = VerifyDecryptPart(msg, NULL, info2);
			
			// Merge signature data into current info item
			info.SetSuccess(info2.GetSuccess());
			info.SetDidSignature(info2.GetDidSignature());
			info.SetSignatureOK(info2.GetSignatureOK());
			info.GetSignedBy() = info2.GetSignedBy();
			if (info.GetError().empty())
				info.SetError(info2.GetError());
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);

			// Reset old mailbox info
			msg->SetMbox(mboxold);
		}

		msg->SetMbox(mboxold);
		
	}

	return result;
}

#pragma mark ____________________________Errors

long CSecurityPlugin::HandleError(CMessageCryptoInfo* info)
{
	// Get error string from plugin
	long err = eSecurity_NoErr;
	char* error = NULL;
	GetLastError(&err, &error);

	// Put into verify/decrypt info if present
	if (info)
	{
		// Copy only first line of error
		const char* p1 = ::strchr(error, '\r');
		const char* p2 = ::strchr(error, '\n');
		if ((p1 != NULL) && (p2 != NULL))
			p1 = (p1 > p2) ? p2 : p1;
		else if (p2 != NULL)
			p1 = p2;
		if (p1 != NULL)
			info->SetError(cdstring(error, p1 - error));
		else
			info->SetError(error);
		
		if (err == eSecurity_BadPassphrase)
			info->SetBadPassphrase(true);
	}

	// Show error alert in some cases
	if ((err != 0) && (err != eSecurity_UserAbort) && ((info == NULL) || CPreferences::sPrefs->mUseErrorAlerts.GetValue()))
		CErrorHandler::PutStopAlert(error, true);

	// Special support for certian errors
	switch(err)
	{
	case eSecurity_BadPassphrase:
		// Remove thre last cached passphrase
		if (CPreferences::sPrefs->mCachePassphrase.GetValue())
			sPassphrases.erase(sLastPassphraseUID);
		break;
	default:;
	}
	
	return err;
}

#pragma mark ____________________________Memory based

// Sign data
long CSecurityPlugin::SignData(const char* in, const char* key,
								char** out, unsigned long* out_len,
								bool useMime, bool binary)
{
	SSignData info;
	info.mInputData = in;
	info.mKey = key;
	info.mOutputData = out;
	info.mOutputDataLength = out_len;
	info.mUseMIME = useMime;
	info.mBinary = binary;

	return CallPlugin(eSecuritySignData, &info);
}

// Encrypt data
long CSecurityPlugin::EncryptData(const char* in, const char** to,
								char** out, unsigned long* out_len,
								bool useMime, bool binary)
{
	SEncryptData info;
	info.mInputData = in;
	info.mKeys = to;
	info.mOutputData = out;
	info.mOutputDataLength = out_len;
	info.mUseMIME = useMime;
	info.mBinary = binary;

	return CallPlugin(eSecurityEncryptData, &info);
}

// Encrypt & sign data
long CSecurityPlugin::EncryptSignData(const char* in, const char** to, const char* key,
										char** out, unsigned long* out_len,
										bool useMime, bool binary)
{
	SEncryptSignData info;
	info.mInputData = in;
	info.mKeys = to;
	info.mSignKey = key;
	info.mOutputData = out;
	info.mOutputDataLength = out_len;
	info.mUseMIME = useMime;
	info.mBinary = binary;

	return CallPlugin(eSecurityEncryptSignData, &info);
}

// Decrypt/verify data
long CSecurityPlugin::DecryptVerifyData(const char* in, const char* sig, const char* in_from,
										char** out, unsigned long* out_len, char** out_signedby, char** out_encryptedto,
										bool* success, bool* did_sig, bool* sig_ok, bool binary)
{
	SDecryptVerifyData info;
	info.mInputData = in;
	info.mInputSignature = sig;
	info.mInputFrom = in_from;
	info.mOutputData = out;
	info.mOutputDataLength = out_len;
	info.mOutputSignedby = out_signedby;
	info.mOutputEncryptedto = out_encryptedto;
	info.mSuccess = success;
	info.mDidSig = did_sig;
	info.mSigOK = sig_ok;
	info.mBinary = binary;

	return CallPlugin(eSecurityDecryptVerifyData, &info);
}

#pragma mark ____________________________File based

// Sign file
long CSecurityPlugin::SignFile(fspec in, const char* key, fspec out, bool useMime, bool binary)
{
	SSignFile info;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	FSSpec fsin;
	in->GetFSSpec(fsin);
	info.mInputFile = &fsin;
#else
	info.mInputFile = in;
#endif
	info.mKey = key;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	FSSpec fsout;
	out->GetFSSpec(fsout);
	info.mOutputFile = &fsout;
#else
	info.mOutputFile = out;
#endif
	info.mUseMIME = useMime;
	info.mBinary = binary;

	return CallPlugin(eSecuritySignFile, &info);
}

// Encrypt file
long CSecurityPlugin::EncryptFile(fspec in, const char** to, fspec out, bool useMime, bool binary)
{
	SEncryptFile info;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	FSSpec fsin;
	in->GetFSSpec(fsin);
	info.mInputFile = &fsin;
#else
	info.mInputFile = in;
#endif
	info.mKeys = to;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	FSSpec fsout;
	out->GetFSSpec(fsout);
	info.mOutputFile = &fsout;
#else
	info.mOutputFile = out;
#endif
	info.mUseMIME = useMime;
	info.mBinary = binary;

	return CallPlugin(eSecurityEncryptFile, &info);
}

// Encrypt & sign file
long CSecurityPlugin::EncryptSignFile(fspec in, const char** to, const char* key, fspec out, bool useMime, bool binary)
{
	SEncryptSignFile info;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	FSSpec fsin;
	in->GetFSSpec(fsin);
	info.mInputFile = &fsin;
#else
	info.mInputFile = in;
#endif
	info.mKeys = to;
	info.mSignKey = key;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	FSSpec fsout;
	out->GetFSSpec(fsout);
	info.mOutputFile = &fsout;
#else
	info.mOutputFile = out;
#endif
	info.mUseMIME = useMime;
	info.mBinary = binary;

	return CallPlugin(eSecurityEncryptSignFile, &info);
}

// Decrypt/verify file
long CSecurityPlugin::DecryptVerifyFile(fspec in, const char* sig, const char* in_from,
										fspec out, char** out_signedby, char** out_encryptedto,
										bool* success, bool* did_sig, bool* sig_ok, bool binary)
{
	SDecryptVerifyFile info;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	FSSpec fsin;
	in->GetFSSpec(fsin);
	info.mInputFile = &fsin;
#else
	info.mInputFile = in;
#endif
	info.mInputSignature = sig;
	info.mInputFrom = in_from;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	FSSpec fsout;
	if (out != NULL)
		out->GetFSSpec(fsout);
	info.mOutputFile = (out != NULL) ? &fsout : NULL;
#else
	info.mOutputFile = out;
#endif
	info.mOutputSignedby = out_signedby;
	info.mOutputEncryptedto = out_encryptedto;
	info.mSuccess = success;
	info.mDidSig = did_sig;
	info.mSigOK = sig_ok;
	info.mBinary = binary;

	return CallPlugin(eSecurityDecryptVerifyFile, &info);
}

#pragma mark ____________________________Others

long CSecurityPlugin::DisposeData(const char* data)
{
	if (data)
		return CallPlugin(eSecurityDisposeData, (void*) data);
	else
		return 1;
}

// Get last error from plugin
long CSecurityPlugin::GetLastError(long* errnum, char** error)
{
	SGetLastError info;
	info.errnum = errnum;
	info.error = error;

	return CallPlugin(eSecurityGetLastError, (void*) &info);
}

// Get MIME parameters for signing
long CSecurityPlugin::GetMIMESign(SMIMEMultiInfo* params)
{
	return CallPlugin(eSecurityGetMIMEParamsSign, (void*) params);
}

// Get MIME parameters for encryption
long CSecurityPlugin::GetMIMEEncrypt(SMIMEMultiInfo* params)
{
	return CallPlugin(eSecurityGetMIMEParamsEncrypt, (void*) params);
}

// Get MIME parameters for encryption and signing
long CSecurityPlugin::GetMIMEEncryptSign(SMIMEMultiInfo* params)
{
	return CallPlugin(eSecurityGetMIMEParamsEncryptSign, (void*) params);
}

// Check that MIME type is verifiable by this plugin
long CSecurityPlugin::CanVerifyThis(const char* type)
{
	// This can be called when not loaded
	StLoadPlugin load(this);

	return CallPlugin(eSecurityCanVerifyThis, (void*) type);
}

// Check that MIME type is decryptable by this plugin
long CSecurityPlugin::CanDecryptThis(const char* type)
{
	// This can be called when not loaded
	StLoadPlugin load(this);

	return CallPlugin(eSecurityCanDecryptThis, (void*) type);
}

#pragma mark ____________________________Callbacks

// Set callback into Mulberry
long CSecurityPlugin::SetCallback()
{
	return CallPlugin(eSecuritySetCallback, (void*) Callback);
}

// Set callback into Mulberry
long CSecurityPlugin::SetContext()
{
	SSMIMEContext context;

	if (CPluginManager::sPluginManager.HasSSL())
	{
		CPluginManager::sPluginManager.GetSSL()->InitSSL();
	
		context.mDLL = CPluginManager::sPluginManager.GetSSL()->GetConnection();
		context.mCertMgr = CCertificateManager::sCertificateManager;

		return CallPlugin(eSecuritySetSMIMEContext, (void*) &context);
	}
	else
		return 0;
}

bool CSecurityPlugin::Callback(ESecurityPluginCallback type, void* data)
{
	switch(type)
	{
	case eCallbackPassphrase:
	{
		SCallbackPassphrase* context = reinterpret_cast<SCallbackPassphrase*>(data);
		return GetPassphrase(context->users, context->passphrase, context->chosen);
	}
	default:
		return false;
	}
}

bool CSecurityPlugin::GetPassphrase(const char** users, char* passphrase, unsigned long& chosen)
{
	// Look in passphrase cache for a user
	if (CPreferences::sPrefs->mCachePassphrase.GetValue())
	{
		const char** p = users;
		unsigned long index = 0;
		while(*p)
		{
			cdstrmap::const_iterator found = sPassphrases.find(*p++);
			if (found != sPassphrases.end())
			{
				sLastPassphraseUID = (*found).first;
				if ((*found).second.length() < 256)
				{
					// Get temporary passphrase
					cdstring temp((*found).second);
					temp.Decrypt(cdstring::eEncryptSimplemUTF7);
					::strcpy(passphrase, temp);

					// Clear memory
					::memset(temp.c_str_mod(), 0, temp.length());

					chosen = index;
					return true;
				}
				else
					return false;
			}
			index++;
		}
	}
	else
		sPassphrases.clear();

	// Ask user for passphrase
	cdstring new_phrase;
	cdstring chosen_user;
	unsigned long index = 0;

	if (CGetPassphraseDialog::PoseDialog(new_phrase, users, chosen_user, index))
	{
		if (new_phrase.length() < 256)
		{
			// Cache the new passphrase
			if (CPreferences::sPrefs->mCachePassphrase.GetValue())
			{
				cdstring temp(new_phrase);
				temp.Encrypt(cdstring::eEncryptSimplemUTF7);
				sPassphrases.insert(cdstrmap::value_type(chosen_user, temp));
				sLastPassphraseUID = chosen_user;
			}

			::strcpy(passphrase, new_phrase);
			
			// Clear memory
			::memset(new_phrase.c_str_mod(), 0, new_phrase.length());

			chosen = index;
			return true;
		}
	}

	return false;
}

void CSecurityPlugin::ClearLastPassphrase()
{
	sPassphrases.erase(sLastPassphraseUID);
}
