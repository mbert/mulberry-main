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


// Header for CICSupport class

#ifndef __CICSUPPORT__MULBERRY__
#define __CICSUPPORT__MULBERRY__

#include <InternetConfig.h>

#include "cdstring.h"

// Classes
class CMIMEContent;
class CFileAttachment;

class CICSupport {

public:
	// Starting/stopping
	static OSStatus		ICStart(OSType appSig);
	static OSStatus		ICStop();

	static bool			ICInstalled()
							{ return sLoaded; }

	// Get config file
	static OSStatus		ICFindConfigFile(FSSpec& dir);				// Get file from local/preferences directory

	// Get preference
	static OSStatus		ICGetPrefPstring(ConstStr255Param key,
									cdstring& pref);
	static OSStatus		ICGetPrefText(ConstStr255Param key,
									cdstring& pref);
	static OSStatus		ICGetPrefBoolean(ConstStr255Param key,
									bool& pref);

	static OSStatus		ICGetPref(ConstStr255Param key,
									char* pref,
									long size = 255);

	static OSStatus		ICSetPref(ConstStr255Param key,
									char* pref,
									long size = 255);

	static OSStatus		ICLaunchURL(char *url);

	// Get specific items
	static void			ICGetRealName(cdstring& name);
	static void			ICGetEmail(cdstring& email);
	static void			ICGetMailAccount(cdstring& user, cdstring& domain);
	static void			ICGetSMTPHost(cdstring& smtp);
	static void			ICGetPswd(cdstring& pswd);
	static void			ICGetFlashIcon(bool& flash);
	static void			ICGetDisplayDialog(bool& dialog);
	static void			ICGetPlaySound(bool& sound);
	static void			ICGetAlertSound(cdstring& snd);
	static void			ICGetListFont(ICFontRecord* font);
	static void			ICGetScreenFont(ICFontRecord* font);
	static void			ICGetPrintFont(ICFontRecord* font);
	static void			ICGetTextCreator(OSType& type);
	static void			ICGetQuote(cdstring& quote);
	static void			ICGetMailHeaders(cdstring& header);
	static void			ICGetSignature(cdstring& signature);
	
	static bool			ICCheckHelperMailto();
	static bool			ICSetHelperMailto();

	static bool			ICCheckHelperWebcal();
	static bool			ICSetHelperWebcal();

	static bool			ICCheckHelper(ConstStr255Param scheme);
	static bool			ICSetHelper(ConstStr255Param scheme);

	static void			ICDecrypt(cdstring& decrypt);

	// File mappings
	static OSStatus		ICMapTypeCreator(CFileAttachment& attach,
													bool use_rsrc);	// Map a file name of a file attachment
	static OSStatus		ICMapFileName(const CMIMEContent& content,
										ICMapEntry& entry,
										bool exact_mime = false);			// Map an attachment name
	static OSStatus		ICMapFileName(const cdstring& fname,
										ICMapEntry& entry,
										bool exact_mime = false);			// Map an attachment name
	static OSStatus		ICMapMIMEType(const cdstring& fname,
										const cdstring& type,
										ICMapEntry& entry,
										bool exact_mime = false);			// Map an attachment name

private:
	// Constructor/destructor are private: this class is never instantiated
	// it is always called through static members
						CICSupport() {};
			 			~CICSupport() {};

	static ICInstance	sInstance;									// ICInstance for connection
	static bool			sLoaded;									// Is it loaded
	
	static OSStatus		GetBestMatch(const cdstring& content,
										ICMapEntry& entry,
										const char* ext,
										bool exact_mime = false);			// Get best type/creator for type/subtype
	static OSStatus		GetDefaultMatch(const cdstring& content,
										ICMapEntry& entry);			// Get default type/creator to type/subtype

	static OSStatus		GetDefaultMIME(CFileAttachment& attach);	// Get default type/subtype to type/creator
};

#endif
