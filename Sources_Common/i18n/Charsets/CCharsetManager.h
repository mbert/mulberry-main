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


// Header for CCharsetManager class

#ifndef __CCHARSETMANAGER__MULBERRY__
#define __CCHARSETMANAGER__MULBERRY__

#include <map>
#include <ostream>

#include "CCharsetCodes.h"
#include "cdstring.h"
#include "cdustring.h"

namespace i18n 
{

class CConverterBase;

class CCharsetManager
{
public:
	typedef std::map<cdstring, ECharsetCode> name_code_map;
	typedef std::map<ECharsetCode, cdstring> code_name_map;
	typedef std::map<ECharsetCode, ECharsetCode> code_code_map;
	typedef std::map<ECharsetCode, EFontMapCode> code_font_map;

	static CCharsetManager sCharsetManager;

	CCharsetManager();
	~CCharsetManager() {}

	ECharsetCode GetCodeFromName(const char* name) const;			// Get charset enum from charset name
	const char* GetNameFromCode(ECharsetCode code) const;			// Get charset name from a code
	
	CConverterBase* GetConverter(const char* name) const;			// Get unicode converter from charset name
	CConverterBase* GetConverter(ECharsetCode code) const;			// Get unicode converter from charset enum
	
	ECharsetCode GetHostCharset(ECharsetCode network) const;		// Get corresponding local host charset for network charset
	ECharsetCode GetNetworkCharset(ECharsetCode network,			// Get corresponding network charset for local host charset
									const char* data = NULL,
									unsigned long length = 0) const;

	EFontMapCode GetFontMapCode(ECharsetCode charset) const;		// Get font map code for charset
	const char* GetFontMapDescriptor(EFontMapCode code) const;		// Get descriptor for font map code

	bool CanMerge(ECharsetCode code1, ECharsetCode code2) const; 	// Allow merging of common charset subsets

	bool Transcode(ECharsetCode from, ECharsetCode to,				// Convert between charsets
					const char* in, size_t len, const char*& out) const;
	cdstring Transcode(ECharsetCode from, ECharsetCode to, const cdstring& txt) const;

	bool ToUnicode(ECharsetCode from,								// Convert to unicode
				   const char* in, size_t len, std::ostream& out) const;

	bool FromUnicode(ECharsetCode to,								// Convert from unicode
					const wchar_t* in, size_t wlen, std::ostream& out) const;

	cdustring ToUTF16(ECharsetCode from, const cdstring& txt) const;
	cdstring FromUTF16(ECharsetCode to, const cdustring& txt) const;

	bool ToUTF16(ECharsetCode from,									// Convert to utf16
					const char* in, size_t len, std::ostream& out) const;
	bool FromUTF16(ECharsetCode to,
					const unichar_t* in, size_t ulen, std::ostream& out) const;	// Convert from utf8

	bool ToUTF8(ECharsetCode from,									// Convert to utf8
					const char* in, size_t len, std::ostream& out) const;
	bool FromUTF8(const char* in, size_t len, std::ostream& out) const;	// Convert from utf8

	ECharsetCode ScriptToCharset(unsigned long script) const;
	unsigned long CharsetToScript(ECharsetCode charset) const;

	ECharsetCode CharsetForText(const char* p, bool always_unicode = true) const;

protected:
	name_code_map	mNameCode;
	code_name_map	mCodeName;
	code_code_map	mNTOHMap;
	code_code_map	mHTONMap;
	code_font_map	mCodeFont;

	ECharsetCode LookForEuro(ECharsetCode host,						// Look for Euro symbol
								const char* data,
								unsigned long length) const;
};

}
#endif
