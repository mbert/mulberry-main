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


// Source for CIntlTextRun class

#include "CIntlTextRun.h"

#include "CCharsetManager.h"
#include "CMIMEFilters.h"
#include "CRFC822.h"
#include "CStreamFilter.h"
#include "CURL.h"

#include "charfilters.h"

#include <strstream>
#include <wstring.h>

using namespace NCharsets;

#pragma mark ____________________________CIntlRun

CIntlTextRun::CIntlRun::CIntlRun()
{
	mCharset = eUSASCII;
	mStart = 0;
	mLength = 0;
	mNext = NULL;
}

CIntlTextRun::CIntlRun::CIntlRun(CIntlTextRun::CIntlRun* previous, NCharsets::ECharsetCode charset, unsigned long length)
{
	mCharset = charset;
	mStart = previous->Start() + previous->Length();
	mLength = length;
	previous->mNext = this;
	mNext = NULL;
}

CIntlTextRun::CIntlRun::~CIntlRun()
{
	delete mNext;
	mNext = NULL;
}

unsigned long CIntlTextRun::CIntlRun::TotalLength() const
{
	unsigned long total = mLength;
	CIntlRun* next = mNext;
	while(next)
	{
		total += next->Length();
		next = next->Next();
	}
	
	return total;
}

void CIntlTextRun::CIntlRun::Append(NCharsets::ECharsetCode charset, unsigned long length)
{
	// Scan to end of runs
	CIntlTextRun::CIntlRun* append_to = this;
	while(append_to->mNext)
		append_to = append_to->mNext;
	
	// See if last one is the same as this
	if (append_to->Charset() == charset)
		// Just bump up the length of the last one
		append_to->Length() += length;
	else
		// Add new item
		new CIntlTextRun::CIntlRun(append_to, charset, length);
}

void CIntlTextRun::CIntlRun::Reset(NCharsets::ECharsetCode charset, unsigned long length)
{
	delete mNext;
	mNext = NULL;
	
	mCharset = charset;
	mLength = length;
}

// Steal the data from one of our own
void CIntlTextRun::CIntlRun::steal(CIntlRun& copy)
{
	// Delete our current siblings
	delete mNext;
	mNext = NULL;
	
	// Set our data to the copy
	mCharset = copy.mCharset;
	mStart = copy.mStart;
	mLength = copy.mLength;
	mNext = copy.mNext;
	
	// Make sure copy has lost its siblings
	copy.mNext = NULL;
}

#pragma mark ____________________________CIntlTextRun

CIntlTextRun::CIntlTextRun()
{
	cstr = NULL;
	wcstr = NULL;
}

CIntlTextRun::~CIntlTextRun()
{
	_tidy();
}

void CIntlTextRun::_tidy()
{
	delete cstr;
	delete wcstr;
	cstr = NULL;
	wcstr = NULL;
}

void CIntlTextRun::_make_c()
{
	if (!cstr)
	{
		_tidy();
		cstr = new cdstring;
	}
}

void CIntlTextRun::_make_wc()
{
	if (!wcstr)
	{
		_tidy();
		wcstr = new cdwstring;
	}
}

// Copy part of another run
void CIntlTextRun::copyrange(const CIntlTextRun& copy, unsigned long start, unsigned long length)
{
	// Copy the data
	if (copy.cstr)
	{
		_make_c();
		cstr->copy(copy.c_str() + start, length);
	}
	else if (copy.wcstr)
	{
		_make_wc();
		wcstr->copy(copy.w_str() + start, length);
	}
	
	// Get run at start
	const CIntlTextRun::CIntlRun* run = copy.GetRun(start);
	
	// Set the first run
	unsigned long alength = min(length, run->Length() - (start - run->Start()));
	mRun.Reset(run->Charset(), alength);

	// Bump down remaining length
	length -= alength;
	
	// While somethign remains to be copied...
	while(length)
	{
		// Get next run
		run = run->Next();
		
		// Get next length to add
		alength = min(length, run->Length() - (start - run->Start()));
		
		// Append the run upto the length we need
		mRun.Append(run->Charset(), alength);
		
		// Bump down remaining length
		length -= alength;
	}
}

// Steal the data from one of our own
void CIntlTextRun::steal(CIntlTextRun& copy)
{
	// Steal the data and its run

	// Check for c/wc-string and steal itw
	if (copy.cstr)
		steal(copy.cstr->grab_c_str());
	else if (copy.wcstr)
		steal(copy.wcstr->grab_w_str());
	else
		_tidy();

	// Copy the entire run
	mRun.steal(copy.mRun);
}

void CIntlTextRun::steal(char* str)
{
	// Make sure we are in char mode
	_make_c();
	
	// Steal the copy
	cstr->steal(str);
}

void CIntlTextRun::steal(wchar_t* str)
{
	// Make sure we are in wchar_t mode
	_make_wc();
	
	// Steal the copy
	wcstr->steal(str);
}

const CIntlTextRun::CIntlRun* CIntlTextRun::GetRun(unsigned long pos) const
{
	// Scan up to the one containing the start position
	const CIntlTextRun::CIntlRun* run = &mRun;
	while(pos > run->Start() + run->Length())
		run = run->Next();
	
	return run;
}

// Convert to local charsets
void CIntlTextRun::ntoh_transcode()
{
	// Must have c-string only
	if (!cstr)
		return;

	// Special for utf-8
	if (mRun.Charset() == eUTF8)
		// Convert to unicode from utf8 and then fall through
		from_utf8();
	
	// Write each run to stream
	std::ostrstream out;
	
	const CIntlTextRun::CIntlRun* run = &mRun;
	CIntlTextRun output;
	unsigned long start = 0;
	while(run)
	{
		// Determine local encoding for this run
		NCharsets::ECharsetCode code_to = CCharsetManager::sCharsetManager.GetHostCharset(run->Charset());
		
		// Transcode to the new one if different
		if (code_to != run->Charset())
		{
			const char* out;
			CCharsetManager::sCharsetManager.Transcode(run->Charset(), code_to,
														c_str() + run->Start(), run->Length(), out);
			output.insert(code_to, out, ::strlen(out));
			delete out;
		}
		else
			// Just insert existing data unchaged
			output.insert(code_to, c_str() + run->Start(), run->Length());
		
		// Get next run
		run = run->Next();
	}
	
	steal(output);
}

void CIntlTextRun::hton_transcode()
{
	// Must have c-string only
	if (!cstr)
		return;

	// Write each run to stream
	std::ostrstream out;
	
	CIntlTextRun::CIntlRun* run = &mRun;
	CIntlTextRun output;
	unsigned long start = 0;
	while(run)
	{
		const char* start = c_str() + run->Start();
		unsigned long length = run->Length();

		// Determine local encoding for this run
		NCharsets::ECharsetCode code_to = CCharsetManager::sCharsetManager.GetNetworkCharset(run->Charset(), start, length);
		
		// Transcode to the new one if different
		if (code_to != run->Charset())
		{
			const char* out;
			CCharsetManager::sCharsetManager.Transcode(run->Charset(), code_to, start, length, out);
			output.insert(code_to, out, ::strlen(out));
			delete out;
		}
		else
			// Just insert existing data unchaged
			output.insert(code_to, start, length);
		
		// Get next run
		run = run->Next();
	}
	
	steal(output);
}

// Convert from charsets to unicode
void CIntlTextRun::to_unicode()
{
	// Must have c-string only
	if (!cstr)
		return;

	// Write each run to stream
	std::ostrstream out;
	
	CIntlTextRun::CIntlRun* run = &mRun;
	unsigned long start = 0;
	while(run)
	{
		// Convert to unicode
		CCharsetManager::sCharsetManager.ToUnicode(run->Charset(), c_str() + run->Start(), run->Length(), out);
		
		// Get next run
		run = run->Next();
	}
	
	// Finish by adding null's
	out << ends << ends;

	// Grab the data
	steal(reinterpret_cast<wchar_t*>(out.str()));

	// Reset runs to single unicode run
	mRun.Reset(eUCS2, length());
}

// Convert from unicode to specified charset
void CIntlTextRun::from_unicode(ECharsetCode charset)
{
	// Must have wc-string only
	if (!wcstr)
		return;

	// Write each run to stream
	std::ostrstream out;
	
	CIntlTextRun::CIntlRun* run = &mRun;
	unsigned long start = 0;
	while(run)
	{
		// Convert from unicode
		CCharsetManager::sCharsetManager.FromUnicode(charset, w_str() + run->Start(), run->Length(), out);
		
		// Get next run
		run = run->Next();
	}
	
	// Finish by adding null's
	out << ends;

	// Grab the data
	steal(out.str());

	// Reset runs
	mRun.Reset(charset, length());
}

// Convert from charsets to unicode run
void CIntlTextRun::to_unicode_run()
{
	// Must have c-string only
	if (!cstr)
		return;

	// Write each run to stream
	std::ostrstream out;
	
	CIntlTextRun::CIntlRun* run = &mRun;
	while(run)
	{
		// Convert to unicode
		std::ostrstream::pos_type start = out.tellp();
		CCharsetManager::sCharsetManager.ToUnicode(run->Charset(), c_str() + run->Start(), run->Length(), out);
		std::ostrstream::pos_type length = out.tellp() - start;

		// Adjust run info to new range
		run->Charset() = static_cast<ECharsetCode>(run->Charset() | eUCS2);
		run->Start() = start / sizeof(wchar_t);
		run->Length() = length / sizeof(wchar_t);

		// Get next run
		run = run->Next();
	}
	
	// Finish by adding null's
	out << ends << ends;

	// Grab the data
	steal(reinterpret_cast<wchar_t*>(out.str()));
}

// Convert from unicode to specified charset
void CIntlTextRun::from_unicode_run()
{
	// Must have wc-string only
	if (!wcstr)
		return;

	// Write each run to stream
	std::ostrstream out;
	
	CIntlTextRun::CIntlRun* run = &mRun;
	unsigned long start = 0;
	while(run)
	{
		// Convert from unicode
		std::ostrstream::pos_type start = out.tellp();
		ECharsetCode charset = static_cast<ECharsetCode>(run->Charset() & ~eUCS2);
		CCharsetManager::sCharsetManager.FromUnicode(charset, w_str() + run->Start(), run->Length(), out);
		std::ostrstream::pos_type length = out.tellp() - start;

		// Adjust run info to new range
		run->Charset() = charset;
		run->Start() = start;
		run->Length() = length;
		
		// Get next run
		run = run->Next();
	}
	
	// Finish by adding null's
	out << ends;

	// Grab the data
	steal(out.str());
}

// Convert from charsets to utf8
void CIntlTextRun::to_utf8()
{
	// Must have c-string only
	if (!cstr)
		return;

	// Write each run to stream
	std::ostrstream out;
	
	CIntlTextRun::CIntlRun* run = &mRun;
	unsigned long start = 0;
	while(run)
	{
		// Convert to unicode
		CCharsetManager::sCharsetManager.ToUTF8(run->Charset(), c_str() + run->Start(), run->Length(), out);
		
		// Get next run
		run = run->Next();
	}
	
	// Finish by adding null's
	out << ends;

	// Grab the data
	steal(out.str());

	// Reset runs
	mRun.Reset(eUTF8, length());
}

// Convert from utf8 to unicode
void CIntlTextRun::from_utf8()
{
	// Must have c-string only
	if (!cstr)
		return;

	// Write each run to stream
	std::ostrstream out;
	
	CIntlTextRun::CIntlRun* run = &mRun;
	unsigned long start = 0;
	while(run)
	{
		// Convert to unicode
		CCharsetManager::sCharsetManager.FromUTF8(c_str() + run->Start(), run->Length(), out);
		
		// Get next run
		run = run->Next();
	}
	
	// Finish by adding null's
	out << ends << ends;

	// Grab the data
	steal(reinterpret_cast<wchar_t*>(out.str()));

	// Reset runs to single unicode run
	mRun.Reset(eUCS2, length());
}

// Convert to/from most appropriate encoding
void CIntlTextRun::to_mime(cdstring& out, bool addr_phrase, bool wrap, unsigned long offset)
{
	// Must have c-string only
	if (!cstr)
		return;

	// First convert to network charsets
	hton_transcode();
	
	// Now minimise the charset specs
	minimise();
	
	// Now do conversion of runs to MIME format
	out = cdstring::null_str;
	CIntlTextRun::CIntlRun* run = &mRun;
	while(run)
	{
		// Shortcut for ASCII
		if (run->Charset() == eUSASCII)
			// Just add run data without conversion
			out.append(c_str() + run->Start(), run->Length());
		else
		{
			ostrstream sout;

			// Create quote start string
			cdstring start;
			start += cRFC1522_QUOTE_START;
			start += CCharsetManager::sCharsetManager.GetNameFromCode(run->Charset());
			start += cRFC1522_QUOTE_MIDDLE;

			// Start with standard RFC1522 info
			sout << start.c_str();
			unsigned long line_length = offset + start.length() + sizeof(cRFC1522_QUOTE_END) - 1 - 3;

			// Add each char and encode
			const char* p = c_str() + run->Start();
			const char* q = p + run->Length();
			while(p < q)
			{
				// Now check for an ordinary unencoded character
				if ((addr_phrase ? cNoQuoteChar1522Addr : cNoQuoteChar1522)[(unsigned char) *p])
				{
					unsigned char d = (unsigned char) *p++;

					// Use underscore for mapped space character
					if (d == ' ')
						d = '_';

					sout << (char) d;
					line_length++;
				}
				
				// Must be an encoded character
				else
				{
					sout << '=';

					// Map to charset
					unsigned char d = (unsigned char) *p++;

					// Do high nibble
					sout << cHexChar[(d >> 4)];

					// Do low nibble
					sout << cHexChar[(d & 0x0F)];
					
					line_length += 3;
				}
				
				// Check for exceed of line length (account for possible encoding of next char
				if (wrap && (line_length >= CRFC822::GetWrapLength()))
				{
					// Must do wrap
					sout << cRFC1522_QUOTE_END << os_endl << " " << start.c_str();
					
					line_length = 1 + start.length() + sizeof(cRFC1522_QUOTE_END) - 1 - 3;
				}
			}

			// Add trailer
			sout << cRFC1522_QUOTE_END;

			// Copy stream back to string
			sout << ends;
			out.append(sout.str());
			sout.freeze(false);
		}
		
		// Get next run
		run = run->Next();
	}
	
}

void CIntlTextRun::from_mime(const cdstring& str)
{
	// Must have c-string only
	_make_c();

	const char* p = str.c_str();
	const char* q = p;
	bool did_decode = false;

	// Check for any encoded
	while(*p)
	{
		// Check for decode
		if ((*p == '=') && (*(p+1) == '?'))
		{
			// Copy previous set of unencoded chars as us-ascii
			if (p != q)
				insert(eUSASCII, q, p - q);

			// Advance and point at charset descriptor
			p += 2;
			const char* char_set = p;

			// Step to end of charset descriptor and terminate
			while(*p && (*p != '?')) p++;
			// Parse error
			if (!*p && !p[1])
				throw -1;

			// Determine charset
			cdstring char_set_name(char_set, p++ - char_set);
			NCharsets::ECharsetCode charset = CCharsetManager::sCharsetManager.GetCodeFromName(char_set_name);

			char cte = *p++;

			// Parse error
			if (!*p || (*p != '?'))
				throw -1;
			p++;
			// Parse error
			if (!*p)
				throw -1;

			// Point at text to decode
			const char* txt = p;

			// Step to end of encoding and terminate, advance
			while(*p && ((*p != '?') || (*(p+1) != '='))) p++;
			// Parse error
			if (!*p)
				throw -1;

			// Filter text according to encoding
			if (p > txt)
			{
				CFilter* filter = NULL;
#if __dest_os == __mac_os
				LHandleStream data;
#elif __dest_os == __win32_os || __dest_os == __linux_os
				LMemFileStream data;
#else
#error __dest_os
#endif

				try
				{
					switch(cte)
					{
					case 'Q':
					case 'q':
						filter = new CQPFilter(eUSAscii, true, false, &data);
						break;
					case 'B':
					case 'b':
						filter = new CBase64Filter(eUSAscii, true, false, &data);
						break;
					default:
						// Parse error
						throw -1;
					}

					// Filter into buffer
					long len = p - txt;
					filter->PutBytes(txt, len);
					delete filter;
					filter = NULL;

					// Copy buffer to stream
					{
#if __dest_os == __mac_os
						StHandleLocker lock(data.GetDataHandle());
#endif

						// Convert from '_' to ' '
						unsigned long actual_len = data.GetLength();
						unsigned long len = actual_len;
#if __dest_os == __mac_os
						char* pp = *data.GetDataHandle();
#elif __dest_os == __win32_os || __dest_os == __linux_os
						char* pp = data.DetachData();
#else
#error __dest_os
#endif
						char* pq = pp;
						while(len--)
						{
							if (*pp == '_') *pp = ' ';
							pp++;
						}

						// Write it out to stream
						insert(charset, pq, actual_len);
					}
				}
				catch(...)
				{
					// Clean up and throw up
					delete filter;
					throw;
				}
			}

			// Bump pointer paste encoding trailer which we found earlier
			q = (p += 2);

			// Set flag for last decode
			did_decode = true;
		}
		
		// Only do the copy after we've started any decoding
		else
		{
			// Sniff to see if encoding run
			bool got_run = false;
			if (did_decode)
			{
				const char* r = p;
				while(*r && !got_run && did_decode)
				{
					switch(*r)
					{
					case ' ':
					case '\t':
						r++;
						break;
					case '=':
						if (*(r+1) == '?')
						{
							// Punt over the spaces
							p = r;
							got_run = true;
						}
						// Fall through
					default:
						// Don't have run of encoded chars => output as normal
						did_decode = false;
						break;
					}
				}
			}

			// Just copy existing char
			if (!got_run)
				p++;
		}
	}

	// Copy remainder as us-ascii
	if (p != q)
		insert(eUSASCII, q, p - q);
}

// Insert item
void CIntlTextRun::insert(NCharsets::ECharsetCode charset, const char* data, size_t len)
{
	// Must have c-string only
	_make_c();

	// Check for empty run
	if (!mRun.Length() && !mRun.Next())
	{
		// Use current run as the first one
		mRun.Charset() = charset;
		mRun.Start() = 0;
		mRun.Length() = len;
	}
	else
		mRun.Append(charset, len);
	cstr->append(data, len);
}

void CIntlTextRun::insert(const CIntlTextRun& txt)
{
	// Must have c-string only
	if (!txt.cstr)
		return;

	// Must have c-string only
	_make_c();

	// Just append the raw data
	cstr->append(txt.c_str());
	
	// Now add each run from the original
	const CIntlRun* run = txt.FirstRun();
	while(run)
	{
		mRun.Append(run->Charset(), run->Length());
		run = run->Next();
	}
}

// Collapse to minimum charset
void CIntlTextRun::minimise()
{
	// Must have c-string only
	if (!cstr)
		return;

	// Set of unique charsets in this run
	set<ECharsetCode> charsets;

	// Add each unique charset to list
	CIntlTextRun::CIntlRun* run = &mRun;
	while(run)
	{
		// Convert iso-8859-x to ascii if no 8-bit chars present
		switch(run->Charset())
		{
		case eISO_8859_1:
		case eISO_8859_2:
		case eISO_8859_3:
		case eISO_8859_4:
		case eISO_8859_5:
		case eISO_8859_6:
		case eISO_8859_7:
		case eISO_8859_8:
		case eISO_8859_9:
		case eISO_8859_10:
		case eISO_8859_11:
		case eISO_8859_13:
		case eISO_8859_14:
		case eISO_8859_15:
		case eISO_8859_16:
			minimise_one_byte(run);
			break;
		default:;
		}
		charsets.insert(run->Charset());
		run = run->Next();
	}
	
	// If there is only one we're done if its a one byte encoding
	if (charsets.size() == 1)
	{
		switch(*charsets.begin())
		{
		case eUSASCII:
		default:
		case eISO_8859_1:
		case eISO_8859_2:
		case eISO_8859_3:
		case eISO_8859_4:
		case eISO_8859_5:
		case eISO_8859_6:
		case eISO_8859_7:
		case eISO_8859_8:
		case eISO_8859_9:
		case eISO_8859_10:
		case eISO_8859_11:
		case eISO_8859_13:
		case eISO_8859_14:
		case eISO_8859_15:
		case eISO_8859_16:
			return;
		case eGB2312:
		case eBig5:
		case eJIS_0201:
		case eJIS_0208:
		case eSJIS:
			break;
		}
	}

	// Now look for possible combinations:
	//
	// ascii + single iso-8859 => merge
	// ascii + japanese => iso-2022-JP
	// ascii + chinese => iso-2022-CN
	// ascii + korean => iso-2022-KR
	// ...others => utf8
	//
	bool has_ascii = false;
	bool has_iso_8859 = false;
	bool has_japanese = false;
	bool has_chinese = false;
	bool has_korean = false;
	bool has_multiple = false;
	ECharsetCode iso_8859_code = eUSASCII;
	for(set<ECharsetCode>::const_iterator iter = charsets.begin(); iter != charsets.end(); iter++)
	{
		switch(*iter)
		{
		case eUSASCII:
		default:
			has_ascii = true;
			break;
		case eISO_8859_1:
		case eISO_8859_2:
		case eISO_8859_3:
		case eISO_8859_4:
		case eISO_8859_5:
		case eISO_8859_6:
		case eISO_8859_7:
		case eISO_8859_8:
		case eISO_8859_9:
		case eISO_8859_10:
		case eISO_8859_11:
		case eISO_8859_13:
		case eISO_8859_14:
		case eISO_8859_15:
		case eISO_8859_16:
			iso_8859_code = *iter;
			has_multiple = has_multiple | has_iso_8859 | has_japanese | has_chinese | has_korean;
			has_iso_8859 = true;
			break;
		case eGB2312:
		case eBig5:
			has_multiple = has_multiple | has_iso_8859 | has_japanese | has_korean;
			has_chinese = true;
			break;
		case eJIS_0201:
		case eJIS_0208:
		case eSJIS:
			has_multiple = has_multiple | has_iso_8859 | has_chinese | has_korean;
			has_japanese = true;
			break;
		}
	}
	
	// Now determine what overall encoding to use
	if (has_multiple)
	{
		// Convert everything to utf8
		to_utf8();
	}
	else if (has_iso_8859)
	{
		// Merge all into single iso-8859
		mRun.Reset(iso_8859_code, mRun.TotalLength());
	}
	else if (has_japanese)
	{
		// Merge into single iso-2022-JP
		to_unicode();
		from_unicode(eISO2022_JP);
	}
	else if (has_chinese)
	{
		// Merge into single iso-2022-CN
		to_unicode();
		from_unicode(eISO2022_CN);
	}
	else if (has_korean)
	{
		// Merge into single iso-2022-KR
		to_unicode();
		from_unicode(eISO2022_KR);
	}
}


// Collapse to minimum charset
void CIntlTextRun::minimise_one_byte(CIntlRun* run)
{
	// Must have c-string only
	if (!cstr)
		return;

	// Assume we have a single run that is ISO-8859-x
	
	// Check for any non-ascii characters
	const unsigned char* p = reinterpret_cast<const unsigned char*>(c_str() + run->Start());
	const unsigned char* q = p + run->Length();
	while(p < q)
	{
		// Ascii == 7bit only
		if (*p++ >= 0x80)
			return;
	}
	
	// Must be ascii only text so switch charset to ascii
	run->Charset() = eUSASCII;
}