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


// Header for CMIMEFilters classes

#ifndef __CCHARSETS__MULBERRY__
#define __CCHARSETS__MULBERRY__

// Character set descriptors
enum ECharset
{
	eCharsetUnknown = 0,
	eCharsetAutomatic = 0,
	eUSAscii,
	
	// ISO-8859
	eISO8859_1,
	eISO8859_2,
	eISO8859_3,
	eISO8859_4,
	eISO8859_5,
	eISO8859_6,
	eISO8859_7,
	eISO8859_8,
	eISO8859_9,
	eISO8859_10,
	eISO8859_11,
	eISO8859_13,
	eISO8859_14,
	eISO8859_15,
	eISO8859_16,
	eCharsetLast,
	
#if 0
	// Mac encodings
	eMacRoman = 100,
	eMacCentralEurRoman,
	eMacCyrillic,
	eMacArabic,
	eMacGreek,
	eMacHebrew,
	eMacTurkish,
	eMacLatin6,
	eMacThai,
	eMacBalticRim,
	eMacCeltic,
	eMacLatin9,
	eMacLatin10,
	eMacJapanese,
	eMacSimpChinese,
	eMacTradChinese,
	eMacKorean,

	// Windows encodings
	eWindowsLatin1 = 200,
	eWindowsLatin2,
	eWindowsCyrillic,
	eWindowsArabic,
	eWindowsGreek,
	eWindowsHebrew,
	eWindowsTurkish,
	eWindowsLatin6,
	eWindowsThai,
	eWindowsBalticRim,
	eWindowsLatin8,
	eWindowsLatin9,
	eWindowsLatin10,
	eWindowsJapanese,
	eWindowsSimpChinese,
	eWindowsTradChinese,
	eWindowsKorean,
#endif
	
	// Other common one-byte scripts
	eKOI8_r = 300,
	eKOI8_ru,
	eKOI8_u,
	
#if 0
	// Two-bytes
	eGB2312 = 400,
	eBig5,
	eHZ,
	eCNS11643_1,
	eCNS11643_2,
	eCNS11643_3,
	eCNS11643_4,
	eCNS11643_5,
	eCNS11643_6,
	eCNS11643_7,
	eJIS_0201,
	eJIS_0208,
	eJIS_0212,
	eSJIS,
	eKSC_5601,
	
	// EUC
	eEUC_CN = 400,
	eEUC_JP,
	eEUC_KR,
	eEUC_TW,

	// ISO-2022
	eISO2022_CN = 500,
	eISO2022_CNEXT,
	eISO2022_JP,
	eISO2022_JP1,
	eISO2022_JP2,
	eISO2022_KR,
	
	// Unicode - use bit masks
	eUCS2 = 1L << 10,
	eUCS4 = 1L << 11,
#endif
	eUTF8 = 1L << 12,

	// Local ranges
#if 0
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	eFirstLocal = eMacRoman,
	eLastLocal = eMacKorean
#elif __dest_os == __win32_os
	eFirstLocal = eWindowsLatin1,
	eLastLocal = eWindowsKorean
#elif __dest_os == __linux_os
	eFirstLocal = eISO8859_1,
	eLastLocal = eISO8859_16
#else
#error __dest_os
#endif
#else
	eDummy
#endif
};

class cdstring;

// Character mappings
const char* CharsetTo(ECharset charset);
const char* CharsetFrom(ECharset charset);

ECharset GetCurrentCharset();
ECharset GetCharset(const cdstring& charset, ECharset default_charset = eISO8859_1);
const char* GetCharsetFromCode(ECharset charset);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
const char cLocalEuro = 0xDB;
#elif __dest_os == __win32_os
const char cLocalEuro = 0x80;
#elif __dest_os == __linux_os
const char cLocalEuro = 0xA4;
#else
#error __dest_os
#endif

#if __dest_os == __linux_os
void SetCurrentCharset(ECharset cs);
#endif

#ifndef LINUX_NOTYET
ECharset ScriptToCharset(unsigned long script);
unsigned long CharsetToScript(ECharset charset);
#endif
#if __dest_os == __linux_os
cdstring CharsetToXFontSpec(ECharset charset);
#endif
#if __dest_os == __win32_os
ECharset CodePageToCharset(unsigned long cp);
unsigned long CharsetToCodePage(ECharset charset);
#endif

// Allowable characters
extern const char cUSASCIIChar[];		// Allowable ASCII chars
extern const char cCEscapeChar[];		// Characters requiring escape in C
extern const char cCEscape[];			// Escape characters in C
extern const char cINETChar[];			// String chars (IMAP/IMSP?ACAP etc) (0 = OK, 1 = quote, 2 = escape, 3 = literal)
extern const char cINETCharBreak[];		// Atom delimiters (IMAP/IMSP?ACAP etc)
extern const char cQPChar[];			// Allowable quoted-printable chars
extern const char cNoQuoteChar1522[];	// Chars that must be converted for RFC1522
extern const char cNoQuoteChar1522Addr[];	// Chars that must be converted for RFC1522 in address phrase

#endif
