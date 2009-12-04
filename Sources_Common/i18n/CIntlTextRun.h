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


// Header for CIntlTextRun class

#ifndef __CINTLTEXTRUN__MULBERRY__
#define __CINTLTEXTRUN__MULBERRY__

#include "CCharsetCodes.h"

#include "cdstring.h"
#include "cdwstring.h"

class CIntlTextRun
{
	class CIntlRun
	{
		friend class CIntlTextRun;

	public:
		NCharsets::ECharsetCode Charset() const
			{ return mCharset; }
		unsigned long Start() const
			{ return mStart; }
		unsigned long Length() const
			{ return mLength; }
		unsigned long TotalLength() const;
		const CIntlRun* Next() const
			{ return mNext; }

	private:
		NCharsets::ECharsetCode mCharset;
		unsigned long mStart;
		unsigned long mLength;
		CIntlRun* mNext;
		
		CIntlRun();
		CIntlRun(CIntlRun* previous, NCharsets::ECharsetCode charset, unsigned long length);
		~CIntlRun();
		
		NCharsets::ECharsetCode& Charset()
			{ return mCharset; }
		unsigned long& Start()
			{ return mStart; }
		unsigned long& Length()
			{ return mLength; }
		CIntlRun* Next()
			{ return mNext; }

		void Append(NCharsets::ECharsetCode charset, unsigned long length);
		void Reset(NCharsets::ECharsetCode charset, unsigned long length);
	
		void steal(CIntlRun& copy);		// Steal the data from one of our own
	};

public:
	CIntlTextRun();
	~CIntlTextRun();

	const char* c_str() const
		{ return cstr ? cstr->c_str() : cdstring::null_str.c_str(); }
	const wchar_t* w_str() const
		{ return wcstr ? wcstr->w_str() : cdwstring::null_str.w_str(); }

	char* c_str_mod()
		{ return cstr ? cstr->c_str_mod() : NULL; }
	wchar_t* w_str_mod()
		{ return wcstr ? wcstr->w_str_mod() : NULL; }

	bool empty() const
		{ return cstr ? cstr->empty() : (wcstr ? wcstr->empty() : true); }
	size_t length() const
		{ return cstr ? cstr->length() : (wcstr ? wcstr->length() : 0); }

	void copyrange(const CIntlTextRun& copy, unsigned long start, unsigned long length);
	void wcopyrange(const CIntlTextRun& copy, unsigned long start, unsigned long length)
		{ copyrange(copy, start * sizeof(wchar_t), length * sizeof(wchar_t)); }

	void steal(CIntlTextRun& copy);		// Steal the data from one of our own
	void steal(char* str);
	void steal(wchar_t* str);

	const CIntlRun* FirstRun() const
		{ return &mRun; }
	const CIntlRun* GetRun(unsigned long pos) const;

	// Convert to/from local encoding
	void ntoh_transcode();
	void hton_transcode();
	
	// Convert to/from unicode
	void to_unicode();
	void from_unicode(NCharsets::ECharsetCode charset);

	// Convert to/from unicode run (preserves original charset details)
	void to_unicode_run();
	void from_unicode_run();

	// Convert to/from utf8
	void to_utf8();
	void from_utf8();

	// Convert to/from most appropriate encoding
	void to_mime(cdstring& out, bool addr_phrase = false, bool wrap = false, unsigned long offset = 0);
	void from_mime(const cdstring& str);

	// Insert item
	void insert(NCharsets::ECharsetCode charset, const char* data, size_t len);
	void insert(const CIntlTextRun& txt);
	
	// Collapse to minimum charset
	void minimise();
	void minimise_one_byte(CIntlRun* run);

protected:
	CIntlRun mRun;
	cdstring* cstr;
	cdwstring* wcstr;
	
	void _tidy();
	void _make_c();
	void _make_wc();
};
#endif