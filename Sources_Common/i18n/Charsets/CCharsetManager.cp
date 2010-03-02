/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Source for CCharsetManager class

#include "CCharsetManager.h"

using namespace i18n;

#include "CConverterBase.h"
#include "CStringUtils.h"

// Converters
#include "CASCII.h"
#include "CConverterOneByte.h"
#include "COneByteCharsets.h"
#ifdef __MULBERRY
#include "CGB2312.h"
#include "CBig5.h"
#include "CJIS_0201.h"
#include "CJIS_0208.h"
#include "CSJIS.h"
#include "CKSC_5601_1987.h"
#include "CISO2022_CN.h"
#include "CISO2022_JP.h"
#include "CISO2022_KR.h"
#include "CMacJapanese.h"
#include "CMacSimpChinese.h"
#include "CMacTradChinese.h"
#include "CMacKorean.h"
#endif
#include "CUCS2.h"
#include "CUCS4.h"
#include "CUTF8.h"
#include "CUTF16.h"

#include "CCharsetData.cp"

#include <memory>
#include <strstream>

CCharsetManager CCharsetManager::sCharsetManager;

CCharsetManager::CCharsetManager()
{
	// Add charsets we know about
	const SNameCode* item1 = &namecodes[0];
	while(item1->name)
	{
		// Add to forward map
		mNameCode[cdstring(item1->name)] = item1->code;
		
		// Add to reverse map only once
		if (!mCodeName.count(item1->code))
			mCodeName[item1->code] = item1->name;
		item1++;
	}
	
	// Add network to host code map
	for(int i = 0; ntoh_code[i][0] != eNone; i++)
	{
		// Network -> host map uses only the OS-specific mapping
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		mNTOHMap[ntoh_code[i][0]] = ntoh_code[i][1];
#elif __dest_os == __win32_os
		mNTOHMap[ntoh_code[i][0]] = ntoh_code[i][2];
#elif __dest_os == __linux_os
		mNTOHMap[ntoh_code[i][0]] = ntoh_code[i][3];
#endif
	}
	
	// Add host to network code map
	for(int i = 0; hton_code[i][0] != eNone; i++)
	{
		// Host -> network map uses all OS mappings
		mHTONMap[hton_code[i][0]] = hton_code[i][1];
	}
	
	// Add host to network code map
	for(int i = 0; charset2font[i].charset != eNone; i++)
	{
		// Charset -> font mappings
		mCodeFont[charset2font[i].charset] = charset2font[i].font;
	}
}

// Get charset code from charset name
ECharsetCode CCharsetManager::GetCodeFromName(const char* name) const
{
	cdstring cname(name);
	::strlower(cname.c_str_mod());
	name_code_map::const_iterator found = mNameCode.find(cname);
	if (found != mNameCode.end())
		return (*found).second;
	
	return eUSASCII;
}

// Get charset name from charset code
const char* CCharsetManager::GetNameFromCode(ECharsetCode code) const
{
	code_name_map::const_iterator found = mCodeName.find(code);
	if (found != mCodeName.end())
		return (*found).second;
	
	return namecodes[0].name;
}

// Get unicode converter from charset name
CConverterBase* CCharsetManager::GetConverter(const char* name) const
{
	return GetConverter(GetCodeFromName(name));
}

// Get unicode converter from charset enum
CConverterBase* CCharsetManager::GetConverter(ECharsetCode code) const
{
	switch(code)
	{
	case eUSASCII:
	default:
		return new CASCII;

	// ISO-8859
	case eISO_8859_1:
		return new CConverterOneByte(cISO8859_1);
	case eISO_8859_2:
		return new CConverterOneByte(cISO8859_2);
	case eISO_8859_3:
		return new CConverterOneByte(cISO8859_3);
	case eISO_8859_4:
		return new CConverterOneByte(cISO8859_4);
	case eISO_8859_5:
		return new CConverterOneByte(cISO8859_5);
	case eISO_8859_6:
		return new CConverterOneByte(cISO8859_6);
	case eISO_8859_7:
		return new CConverterOneByte(cISO8859_7);
	case eISO_8859_8:
		return new CConverterOneByte(cISO8859_8);
	case eISO_8859_9:
		return new CConverterOneByte(cISO8859_9);
	case eISO_8859_10:
		return new CConverterOneByte(cISO8859_10);
	case eISO_8859_11:
		return new CConverterOneByte(cISO8859_11);
	case eISO_8859_13:
		return new CConverterOneByte(cISO8859_13);
	case eISO_8859_14:
		return new CConverterOneByte(cISO8859_14);
	case eISO_8859_15:
		return new CConverterOneByte(cISO8859_15);
	case eISO_8859_16:
		return new CConverterOneByte(cISO8859_16);

	// Mac OS
	case eMacRoman:
		return new CConverterOneByte(cMacRoman);
	case eMacCentralEurRoman:
		return new CConverterOneByte(cMacCentralEurRoman);
	case eMacCyrillic:
		return new CConverterOneByte(cMacCyrillic);
	case eMacArabic:
		return new CConverterOneByte(cMacArabic);
	case eMacGreek:
		return new CConverterOneByte(cMacGreek);
	case eMacHebrew:
		return new CConverterOneByte(cMacHebrew);
	case eMacTurkish:
		return new CConverterOneByte(cMacTurkish);
	case eMacThai:
		return new CConverterOneByte(cMacThai);
	case eMacBalticRim:
		return new CConverterOneByte(cMacCentralEurRoman);
//		case eMacCeltic:
//		return new CConverterOneByte(cISO8859_2);
	case eMacLatin9:
		return new CConverterOneByte(cMacRoman);
//	case eMacLatin10:
//		return new CConverterOneByte(cISO8859_2);

#ifdef __MULBERRY
	case eMacJapanese:
		return new CMacJapanese;
	case eMacSimpChinese:
		return new CMacSimpChinese;
	case eMacTradChinese:
		return new CMacTradChinese;
	case eMacKorean:
		return new CMacKorean;
#endif

	// Win32
	case eWindowsLatin1:
		return new CConverterOneByte(cWindowsLatin1);
	case eWindowsLatin2:
		return new CConverterOneByte(cWindowsLatin2);
	case eWindowsCyrillic:
		return new CConverterOneByte(cWindowsCyrillic);
	case eWindowsArabic:
		return new CConverterOneByte(cWindowsArabic);
	case eWindowsGreek:
		return new CConverterOneByte(cWindowsGreek);
	case eWindowsHebrew:
		return new CConverterOneByte(cWindowsHebrew);
	case eWindowsTurkish:
		return new CConverterOneByte(cWindowsTurkish);
	case eWindowsThai:
		return new CConverterOneByte(cWindowsThai);
	case eWindowsBalticRim:
		return new CConverterOneByte(cWindowsBalticRim);
//	case eWindowsLatin8:
//		return new CConverterOneByte(cISO8859_2);
	case eWindowsLatin9:
		return new CConverterOneByte(cWindowsLatin1);
//	case eWindowsLatin10:
//		return new CConverterOneByte(cISO8859_2);

#ifdef __MULBERRY
	case eWindowsJapanese:
		return new CSJIS;
#endif

	// Other one-bytes
	case eKOI8_r:
		return new CConverterOneByte(cKOI8_r);
	case eKOI8_ru:
		return new CConverterOneByte(cKOI8_ru);
	case eKOI8_u:
		return new CConverterOneByte(cKOI8_u);
	
#ifdef __MULBERRY
	// Two-bytes
	case eGB2312:
		return new CGB2312;
	case eBig5:
		return new CBig5;
	case eJIS_0201:
		return new CJIS_0201;
	case eJIS_0208:
		return new CJIS_0208;
	case eSJIS:
		return new CSJIS;
	case eKSC_5601_1987:
		return new CKSC_5601_1987;

		// EUC
	case eEUC_CN:
		return new CGB2312;
	case eEUC_JP:
		return NULL;
	case eEUC_KR:
		return new CKSC_5601_1987;
	case eEUC_TW:
		return NULL;

	// ISO-2022
	case eISO2022_CN:
		return new CISO2022_CN;
	case eISO2022_JP:
		return new CISO2022_JP;
	case eISO2022_KR:
		return new CISO2022_KR;
#endif
	
	// Unicode
	case eUCS2:
		return new CUCS2;
	case eUCS4:
		return new CUCS4;
	case eUTF8:
		return new CUTF8;
	case eUTF16:
		return new CUTF16;
	}
}

// Get corresponding local host charset for network charset
ECharsetCode CCharsetManager::GetHostCharset(ECharsetCode network) const
{
	code_code_map::const_iterator found = mNTOHMap.find(network);
	if (found != mNTOHMap.end())
		return (*found).second;
	else
	{
		// Look for a matching host charset
		ECharsetCode from_host = GetNetworkCharset(network);
		if (from_host != network)
		{
			// Now return try the conversion again
			found = mNTOHMap.find(from_host);
			if (found != mNTOHMap.end())
				return (*found).second;
		}
		return network;
	}
}

// Get corresponding network charset for local host charset
ECharsetCode CCharsetManager::GetNetworkCharset(ECharsetCode host, const char* data, unsigned long length) const
{
	// Special for some host charsets
	switch(host)
	{
	case eMacRoman:
	case eWindowsLatin1:
		host = LookForEuro(host, data, length);
		break;
	default:;
	}

	code_code_map::const_iterator found = mHTONMap.find(host);
	if (found != mHTONMap.end())
		return (*found).second;
	else
		return host;
}

// Look for Euro symbol
ECharsetCode CCharsetManager::LookForEuro(ECharsetCode host, const char* data, unsigned long length) const
{
	// Ignore empty data
	if (data && *data && length)
	{
		// Determine local code for euro
		unsigned char euro = 0;
		ECharsetCode new_host = host;
		switch(host)
		{
		case eMacRoman:
			euro = 0xDB;
			new_host = eMacLatin9;
			break;
		case eWindowsLatin1:
			euro = 0x80;
			new_host = eWindowsLatin9;
			break;
		default:;
		}
		
		// Scan text looking for euro
		const char* p = data;
		const char* q = p + length;
		while(p < q)
		{
			// If its found, pretend local encoding is of a type to
			// trigger iso-8859-15 when converted to network charset
			if (*p++ == euro)
				return new_host;
		}
	}
	
	// No euro or data - return host encoding without change	
	return host;
}

// Get font map code for charset
EFontMapCode CCharsetManager::GetFontMapCode(ECharsetCode charset) const
{
	code_font_map::const_iterator found = mCodeFont.find(charset);
	if (found != mCodeFont.end())
		return (*found).second;
	else
		return eWesternEuropean;
}

// Get descriptor for font map code
const char* CCharsetManager::GetFontMapDescriptor(EFontMapCode code) const
{
	if (code > eUserDefined)
		code = eWesternEuropean;
	return fontmap_descriptor[code];
}

// Allow merging of common charset subsets
bool CCharsetManager::CanMerge(ECharsetCode code1, ECharsetCode code2) const
{
	// Determine whether code2 can be included in code1
	switch(code1)
	{
	// Only ISO-2022
	case eISO2022_JP:
		switch(code2)
		{
		case eUSASCII:
		case eJIS_0201:
		case eJIS_0208:
			return true;
		default:
			return false;
		}
		break;
	case eISO2022_CN:
		switch(code2)
		{
		case eUSASCII:
		case eGB2312:
			return true;
		default:
			return false;
		}
		break;
	case eISO2022_KR:
		switch(code2)
		{
		case eUSASCII:
		case eKSC_5601_1987:
			return true;
		default:
			return false;
		}
		break;
	default:
		return false;
	}
}

#pragma mark ____________________________charset conversions

// Convert between charsets
bool CCharsetManager::Transcode(ECharsetCode from, ECharsetCode to, const char* in, size_t len, const char*& out) const
{
	// Special case utf-8 -> ucs-2
	if ((from == eUTF8) && (to == eUCS2))
	{
		std::ostrstream sout;
		FromUTF8(in, len, sout);
		sout << std::ends << std::ends;
		out = sout.str();
		return true;
	}

	// Get converters
	std::auto_ptr<CConverterBase> to_unicode(GetConverter(from));
	std::auto_ptr<CConverterBase> from_unicode(GetConverter(to));
	
	// Must have converters
	if (!to_unicode.get() || !from_unicode.get())
		return false;

	// Loop over input
	std::ostrstream sout;
	const unsigned char* p = reinterpret_cast<const unsigned char*>(in);
	const unsigned char* end = p + len;
	from_unicode->init_w_2_c(sout);
	while(p < end)
	{
		wchar_t wc = to_unicode->c_2_w(p);
		
		// Don't allow NULL's (which may come back for illegal chars)
		if (wc)
		{
			char buf[32];
			int buflen = from_unicode->w_2_c(wc, &buf[0]);
			for(int i = 0; i < buflen; i++)
				sout.put(buf[i]);
		}
	}
	from_unicode->finish_w_2_c(sout);
	sout << std::ends;
	out = sout.str();
	return true;
}

cdstring CCharsetManager::Transcode(ECharsetCode from, ECharsetCode to, const cdstring& txt) const
{
	cdstring result;
	if (from == to)
		result = txt;
	else
	{
		const char* sout;
		Transcode(from, to, txt, txt.length(), sout);
		result.steal(const_cast<char*>(sout));
	}
	
	return result;
}

#pragma mark ____________________________unicode conversions

// Convert to unicode
bool CCharsetManager::ToUnicode(ECharsetCode from, const char* in, size_t len, std::ostream& out) const
{
	// Get converter
	std::auto_ptr<CConverterBase> to_unicode(GetConverter(from));
	
	// Must have converter
	if (!to_unicode.get())
		return false;

	// Do unicode convert
	to_unicode->ToUnicode(in, len, out);
	return true;
}

// Convert from unicode
bool CCharsetManager::FromUnicode(ECharsetCode to, const wchar_t* in, size_t wlen, std::ostream& out) const
{
	// Get converter
	std::auto_ptr<CConverterBase> from_unicode(GetConverter(to));
	
	// Must have converter
	if (!from_unicode.get())
		return false;

	// Do unicode convert
	from_unicode->FromUnicode(in, wlen, out);
	return true;
}

// cdstring -> cdustring
cdustring CCharsetManager::ToUTF16(ECharsetCode from, const cdstring& txt) const
{
	cdustring result;
	std::ostrstream sout;
	ToUTF16(from, txt, txt.length(), sout);
	sout << std::ends << std::ends;
	result.steal((unichar_t*)sout.str());
	
	return result;
}

// cdustring -> cdstring
cdstring CCharsetManager::FromUTF16(ECharsetCode to, const cdustring& txt) const
{
	cdstring result;
	std::ostrstream sout;
	FromUTF16(to, txt, txt.length(), sout);
	sout << std::ends;
	result.steal(sout.str());
	
	return result;
}

// Convert to utf16
bool CCharsetManager::ToUTF16(ECharsetCode from, const char* in, size_t len, std::ostream& out) const
{
	if (from == eUTF16)
	{
		out.write(in, len);
		return true;
	}
	
	// Get converter
	std::auto_ptr<CConverterBase> to_unicode(GetConverter(from));
	
	// Must have converter
	if (!to_unicode.get())
		return false;

	// Do unicode convert
	to_unicode->ToUTF16(in, len, out);
	return true;
}

// Convert from unicode
bool CCharsetManager::FromUTF16(ECharsetCode to, const unichar_t* in, size_t ulen, std::ostream& out) const
{
	if (to == eUTF16)
	{
		out.write((const char*)in, ulen * sizeof(unichar_t));
		return true;
	}
	
	// Get converter
	std::auto_ptr<CConverterBase> from_unicode(GetConverter(to));
	
	// Must have converter
	if (!from_unicode.get())
		return false;

	// Do unicode convert
	from_unicode->FromUTF16((unsigned short*)in, ulen, out);
	return true;
}

// Convert to utf8
bool CCharsetManager::ToUTF8(ECharsetCode from, const char* in, size_t len, std::ostream& out) const
{
	if (from == eUTF8)
	{
		out.write(in, len);
		return true;
	}
	
	// Get converter
	std::auto_ptr<CConverterBase> to_unicode(GetConverter(from));
	
	// Must have converter
	if (!to_unicode.get())
		return false;

	// Do unicode convert
	to_unicode->ToUTF8(in, len, out);
	return true;
}

// Convert from utf8
bool CCharsetManager::FromUTF8(const char* in, size_t len, std::ostream& out) const
{
	// Convert each character
	const char* p = in;
	const char* q = p + len;
	unsigned long charlen = 0;
	wchar_t wc = 0;
	bool bom_tested = false;
	while(p < q)
	{
		unsigned char mask = 0x3f;
		if (charlen == 0)
		{
			// Determine length of utf8 encoded wchar_t
			if ((*p & 0xf0 ) == 0xe0)
			{
				charlen = 3;
				mask = 0x0f;
			}
			else if ((*p & 0xe0 ) == 0xc0)
			{
				charlen = 2;
				mask = 0x1f;
			}
			else
			{
				charlen = 1;
				mask = 0x7f;
			}

			// Reset char
			wc = 0;
		}

		// Convert the byte
		wc <<= 6;
		wc |= (*p & mask);
		
		// Reduce byte remaining count and write it out if done
		if (!--charlen)
		{
			// Ignore byte order mark
			if (bom_tested)
			{
#ifdef big_endian
				out.put(wc >> 8);
				out.put(wc & 0x00FF);
#else
				out.put(wc & 0x00FF);
				out.put(wc >> 8);
#endif
			}
			else
			{
				if ((wc != 0xFEFF) && (wc != 0xFFFE))
				{
#ifdef big_endian
					out.put(wc >> 8);
					out.put(wc & 0x00FF);
#else
					out.put(wc & 0x00FF);
					out.put(wc >> 8);
#endif
				}
				bom_tested = true;
			}
		}
		
		// Bump ptr
		p++;
	}
	
	return true;
}

#pragma mark ____________________________charsets<->scripts/code pages

#if __dest_os == __win32_os
ECharsetCode CodePageToCharset(unsigned long cp);
ECharsetCode CodePageToCharset(unsigned long cp)
{
	// Convert from Code Page into MIME charset
	switch(cp)
	{
	case 1252: // Latin 1 (US, Western Europe)
	default:
		return eWindowsLatin1;
	case 1250: // Central European 
		return eWindowsLatin2;
	case 1251: // Cyrillic
		return eWindowsCyrillic;
	case 1256: // Arabic
		return eWindowsArabic;
	case 1253: // Greek
		return eWindowsGreek;
	case 1255: // Hebrew
		return eWindowsHebrew;
	case 1254: // Turkish
		return eWindowsTurkish;
	case 874: // Thai
		return eWindowsThai;
	case 1257: // Baltic
		return eWindowsBalticRim;

	case 932: // Japan
		return eWindowsJapanese;
	case 936: // Chinese (PRC, Singapore)
		return eWindowsSimpChinese;
	case 950: // Chinese (Taiwan, Hong Kong) 
		return eWindowsTradChinese;
	case 949: // Korean
		return eWindowsKorean;
	}
}

unsigned long CharsetToCodePage(ECharsetCode charset);
unsigned long CharsetToCodePage(ECharsetCode charset)
{
	// Convert from Code Page into MIME charset
	switch(charset)
	{
	case eWindowsLatin1: // Latin 1 (US, Western Europe)
	default:
		return 1252;
	case eWindowsLatin2: // Central European 
		return 1250;
	case eWindowsCyrillic: // Cyrillic
		return 1251;
	case eWindowsArabic: // Arabic
		return 1256;
	case eWindowsGreek: // Greek
		return 1253;
	case eWindowsHebrew: // Hebrew
		return 1255;
	case eWindowsTurkish: // Turkish
		return 1254;
	case eWindowsThai: // Thai
		return 874;
	case eWindowsBalticRim: // Baltic
		return 1257;

	case eWindowsJapanese: // Japan
		return 932;
	case eWindowsSimpChinese: // Chinese (PRC, Singapore)
		return 936;
	case eWindowsTradChinese: // Chinese (Taiwan, Hong Kong) 
		return 950;
	case eWindowsKorean: // Korean
		return 949;
	}
}
#endif

ECharsetCode CCharsetManager::ScriptToCharset(unsigned long script) const
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	switch(script)
	{
	case smRoman:
	default:
		return eMacRoman;
	case smCentralEuroRoman:	/* For Czech, Slovak, Polish, Hungarian, Baltic langs*/
		return eMacCentralEurRoman;
	case smCyrillic:
		return eMacCyrillic;
	case smArabic:
		return eMacArabic;
	case smGreek:
		return eMacGreek;
	case smHebrew:
		return eMacHebrew;
	case smThai:
		return eMacThai;
	case smJapanese:
		return eMacJapanese;
	case smSimpChinese:
		return eMacSimpChinese;
	case smTradChinese:
		return eMacTradChinese;
	case smKorean:
		return eMacKorean;
	}
#elif __dest_os == __win32_os
	// Convert from font charset to Code Page
	unsigned long cp = 0;
	switch(script)
	{
	case ANSI_CHARSET:
	case DEFAULT_CHARSET:
	case SYMBOL_CHARSET:
	default:
		cp = 1252;
		break;
	case EASTEUROPE_CHARSET:
		cp = 1250;
		break;
	case RUSSIAN_CHARSET:
		cp = 1251;
		break;
	case ARABIC_CHARSET:
		cp = 1256;
		break;
	case GREEK_CHARSET:
		cp = 1253;
		break;
	case HEBREW_CHARSET:
		cp = 1255;
		break;
	case TURKISH_CHARSET:
		cp = 1254;
		break;
	case THAI_CHARSET:
		cp = 874;
		break;
	case BALTIC_CHARSET:
		cp = 1257;
		break;
	case SHIFTJIS_CHARSET:
		cp = 932;
		break;
	case GB2312_CHARSET:
		cp = 936;
		break;
	case CHINESEBIG5_CHARSET:
		cp = 950;
		break;
	case HANGUL_CHARSET:
		cp = 949;
		break;
	case JOHAB_CHARSET:
		cp = 1361;
		break;
	}

	// Convert from Code Page into MIME charset
	return ::CodePageToCharset(cp);
#else
#ifdef _TODO
#endif
	return eUSASCII;
#endif

}

unsigned long CCharsetManager::CharsetToScript(ECharsetCode charset) const
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	switch(charset)
	{
	case eUSASCII:
	case eMacRoman:
	default:
		return smRoman;
	case eMacCentralEurRoman:
		return smCentralEuroRoman;
	case eMacCyrillic:
		return smCyrillic;
	case eMacArabic:
		return smArabic;
	case eMacGreek:
		return smGreek;
	case eMacHebrew:
		return smHebrew;
	case eMacTurkish:
		return smRoman;
	case eMacJapanese:
		return smJapanese;
	case eMacSimpChinese:
		return smSimpChinese;
	case eMacTradChinese:
		return smTradChinese;
	case eMacKorean:
		return smKorean;
	}
#elif __dest_os == __win32_os
	unsigned long cp = ::CharsetToCodePage(charset);
	switch(cp)
	{
	case 1252:
	default:
		return ANSI_CHARSET;
	case 1250:
		return EASTEUROPE_CHARSET;
	case 1251:
		return RUSSIAN_CHARSET;
	case 1256:
		return ARABIC_CHARSET;
	case 1253:
		return GREEK_CHARSET;
	case 1255:
		return HEBREW_CHARSET;
	case 1254:
		return TURKISH_CHARSET;
	case 874:
		return THAI_CHARSET;
	case 1257:
		return BALTIC_CHARSET;
	case 932:
		return SHIFTJIS_CHARSET;
	case 936:
		return GB2312_CHARSET;
	case 950:
		return CHINESEBIG5_CHARSET;
	case 949:
		return HANGUL_CHARSET;
	case 1361:
		return JOHAB_CHARSET;
	}
#else
#ifdef _TODO
#endif
	return 0;
#endif
}

ECharsetCode CCharsetManager::CharsetForText(const char* txt, bool always_unicode) const
{
	if (txt == NULL)
		return eUSASCII;

	// Determine charset for simple ascii/utf8 case
	if (always_unicode)
	{
		//bool non_ascii = false;
		const unsigned char* p = reinterpret_cast<const unsigned char*>(txt);
		while(*p)
		{
			if (*p++ > 0x7F)
				return eUTF8;
		}
		
		return eUSASCII;
	}
	
	// Do iso-8859-1/15 subset check
	else
	{
		bool non_ascii = false;
		bool have_euro = false;
		const unsigned char* p = reinterpret_cast<const unsigned char*>(txt);
		const unsigned char* q = p + ::strlen(txt);
		unsigned long charlen = 0;
		wchar_t wc = 0;
		while(p < q)
		{
			unsigned char mask = 0x3f;
			if (charlen == 0)
			{
				// Determine length of utf8 encoded wchar_t
				if ((*p & 0xf0 ) == 0xe0)
				{
					charlen = 3;
					mask = 0x0f;
				}
				else if ((*p & 0xe0 ) == 0xc0)
				{
					charlen = 2;
					mask = 0x1f;
				}
				else
				{
					charlen = 1;
					mask = 0x7f;
				}

				// Reset char
				wc = 0;
			}

			// Convert the byte
			wc <<= 6;
			wc |= (*p & mask);

			// Bump ptr
			p++;

			// Reduce byte remaining count and test if done
			if (!--charlen)
			{
				if (wc > 0x007F)
				{
					// Look for non-ISO-8859-15 character
					if ((wc > 0x00FF) && (wc != 0x20AC))
						return eUTF8;
						
					// Have some valid non-ascii ISO-8859-15
					non_ascii = true;
					if (wc == 0x20AC)
						have_euro = true;
				}
			}
		}
		
		return non_ascii ? (have_euro ? eISO_8859_15 : eISO_8859_1) : eUSASCII;
	}
}
