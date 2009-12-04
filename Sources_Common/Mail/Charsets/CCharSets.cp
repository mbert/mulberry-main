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


// Source for CCharSets mappings

#include "CCharSets.h"

#include "cdstring.h"
#include "CStringUtils.h"

const char* cCharsets[] = {"", "us-ascii", "iso-8859-1", "iso-8859-2", "iso-8859-3", "iso-8859-4",
								"iso-8859-5", "iso-8859-6", "iso-8859-7", "iso-8859-8", "iso-8859-9",
								"iso-8859-10", "iso-8859-11", "iso-8859-13", "iso-8859-14", "iso-8859-15"};
// Type for name-code map data
struct SNameCode
{
	const char* name;
	ECharset code;
};

// Charset names mapped to codes
const SNameCode namecodes[] =
{
	{"us-ascii", eUSAscii},

	{"iso-8859-1", eISO8859_1},
	{"iso_8859-1", eISO8859_1},
	{"latin1", eISO8859_1},

	{"iso-8859-2", eISO8859_2},
	{"iso_8859-2", eISO8859_2},
	{"latin2", eISO8859_2},

	{"iso-8859-3", eISO8859_3},
	{"iso_8859-3", eISO8859_3},
	{"latin3", eISO8859_3},

	{"iso-8859-4", eISO8859_4},
	{"iso_8859-4", eISO8859_4},
	{"latin4", eISO8859_4},

	{"iso-8859-5", eISO8859_5},
	{"iso_8859-5", eISO8859_5},
	{"cyrillic", eISO8859_5},

	{"iso-8859-6", eISO8859_6},
	{"iso_8859-6", eISO8859_6},
	{"arabic", eISO8859_6},

	{"iso-8859-7", eISO8859_7},
	{"iso_8859-7", eISO8859_7},
	{"greek", eISO8859_7},

	{"iso-8859-8", eISO8859_8},
	{"iso_8859-8", eISO8859_8},
	{"hebrew", eISO8859_8},

	{"iso-8859-9", eISO8859_9},
	{"iso_8859-9", eISO8859_9},
	{"latin5", eISO8859_9},

	{"iso-8859-10", eISO8859_10},
	{"iso_8859-10", eISO8859_10},
	{"latin6", eISO8859_10},

	{"iso-8859-11", eISO8859_11},
	{"iso_8859-11", eISO8859_11},
	{"thai", eISO8859_11},

	{"iso-8859-13", eISO8859_13},
	{"iso_8859-13", eISO8859_13},
	{"latin7", eISO8859_13},

	{"iso-8859-14", eISO8859_14},
	{"iso_8859-14", eISO8859_14},
	{"latin8", eISO8859_14},

	{"iso-8859-15", eISO8859_15},
	{"iso_8859-15", eISO8859_15},
	{"latin9", eISO8859_15},

	{"iso-8859-16", eISO8859_16},
	{"iso_8859-16", eISO8859_16},
	{"latin10", eISO8859_16},

#if 0
	// mac
	{"macroman", eMacRoman},

	{"maccentraleurope", eMacCentralEurRoman},

	{"maccyrillic", eMacCyrillic},

	{"macarabic", eMacArabic},

	{"macgreek", eMacGreek},

	{"machebrew", eMacHebrew},

	{"macturkish", eMacTurkish},

	{"macthai", eMacThai},

	{"macbaltic", eMacBalticRim},

	{"macceltic", eMacCeltic},

	{"maclatin9", eMacLatin9},

	{"maclatin10", eMacLatin10},

	{"macjapanese", eMacJapanese},

	{"mactradchinese", eMacTradChinese},

	{"mackorean", eMacKorean},

	// Windows
	{"windows-1252", eWindowsLatin1},
	{"cp1252", eWindowsLatin1},
	{"ms-ansi", eWindowsLatin1},

	{"windows-1250", eWindowsLatin2},
	{"cp1250", eWindowsLatin2},
	{"ms-ee", eWindowsLatin2},

	{"windows-1251", eWindowsCyrillic},
	{"cp1251", eWindowsCyrillic},
	{"ms-cyrl", eWindowsCyrillic},

	{"windows-1256", eWindowsArabic},
	{"cp1256", eWindowsArabic},
	{"ms-arab", eWindowsArabic},

	{"windows-1253", eWindowsGreek},
	{"cp1253", eWindowsGreek},
	{"ms-greek", eWindowsGreek},

	{"windows-1255", eWindowsHebrew},
	{"cp1255", eWindowsHebrew},
	{"ms-hebr", eWindowsHebrew},
	
	{"windows-1254", eWindowsTurkish},
	{"cp1254", eWindowsTurkish},
	{"ms-turk", eWindowsTurkish},
	
	{"windows-874", eWindowsThai},
	{"cp874", eWindowsThai},
	{"ms-thai", eWindowsThai},

	{"windows-1257", eWindowsBalticRim},
	{"cp1257", eWindowsBalticRim},
	{"winbaltrim", eWindowsBalticRim},
	
	{"windows-932", eWindowsJapanese},
	{"cp932", eWindowsJapanese},
	
	{"windows-936", eWindowsSimpChinese},
	{"cp936", eWindowsSimpChinese},
	
	{"windows-950", eWindowsTradChinese},
	{"cp950", eWindowsTradChinese},
	
	{"windows-949", eWindowsKorean},
	{"cp949", eWindowsKorean},
#endif

	// Other common one-byte scripts
	{"koi8-r", eKOI8_r},
	{"koi8_r", eKOI8_r},
	{"koi8-ru", eKOI8_ru},
	{"koi8_ru", eKOI8_ru},
	{"koi8-u", eKOI8_u},
	{"koi8_u", eKOI8_u},

#if 0	
	// Two bytes
	{"gb2312", eGB2312},
	{"big5", eBig5},

	{"jis_x0201", eJIS_0201},

	{"jis0208", eJIS_0208},
	{"jis_x0208", eJIS_0208},

	{"shift-jis", eSJIS},
	{"shift_jis", eSJIS},
	{"sjis", eSJIS},
	
	{"ksc_5601", eKSC_5601},
	{"korean", eKSC_5601},
	
	// ISO 2022
	{"iso-2022-jp", eISO2022_JP},

	{"iso-2022-cn", eISO2022_CN},
	
	{"iso-2022-kr", eISO2022_KR},
	
	// Unicode
	{"ucs-2", eUCS2},
	{"ucs-4", eUCS4},
#endif
	{"utf-8", eUTF8},

	// Last one to trigger end of loop
	{NULL, eUSAscii}
};

typedef map<cdstring, ECharset> name_code_map;
static name_code_map sNameCode;

// USAscii character mappings
const char cToUSASCII[] = // Map to US-ASCII from ASCII
  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,		// 0 - 15
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,		// 16 - 31
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,		// 32 - 47
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,		// 48 - 63
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,		// 64 - 79
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,		// 80 - 95
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,		// 96 - 111
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,		// 112 - 127
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,		// 128 - 143
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,		// 144 - 159
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,		// 160 - 175
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,		// 176 - 191
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,		// 192 - 207
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,		// 208 - 223
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,		// 224 - 239
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF};	// 240 - 255

const char cFromUSASCII[] = // Map from US-ASCII tp ASCII
  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,		// 0 - 15
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,		// 16 - 31
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,		// 32 - 47
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,		// 48 - 63
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,		// 64 - 79
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,		// 80 - 95
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,		// 96 - 111
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,		// 112 - 127
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,		// 128 - 143
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,		// 144 - 159
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,		// 160 - 175
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,		// 176 - 191
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,		// 192 - 207
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,		// 208 - 223
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,		// 224 - 239
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF};	// 240 - 255

// ISO-8859-x character mappings
#include "CISOCharsets.h"

const char cUSASCIIChar[] = // Allowable ASCII chars
						  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,		// 0 - 15
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 16 - 31
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 32 - 47
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 48 - 63
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 64 - 79
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 80 - 95
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 96 - 111
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,		// 112 - 127
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 128 - 143
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 144 - 159
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 160 - 175
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 176 - 191
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 192 - 207
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 208 - 223
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 224 - 239
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	// 240 - 255

const char cCEscapeChar[] = // String chars for C (0 = OK, 1 = quote, 2 = escape, 3 = literal)
						  { 2, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 3, 3,		// 0 - 15
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 16 - 31
							1, 0, 2, 0, 0, 1, 0, 2, 1, 1, 1, 0, 0, 0, 0, 0,		// 32 - 47
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 48 - 63
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 64 - 79
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0,		// 80 - 95
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 96 - 111
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 3,		// 112 - 127
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 128 - 143
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 144 - 159
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 160 - 175
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 176 - 191
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 192 - 207
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 208 - 223
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 224 - 239
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };	// 240 - 255

const char cCEscape[] = // String chars for C
						  { 'O', 0, 0, 0, 0, 0, 0, 'a', 'b', 't', 'n', 'v', 'f', 'r', 0, 0,		// 0 - 15
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,						// 16 - 31
							0, 0, '\"', 0, 0, 0, 0, '\'', 0, 0, 0, 0, 0, 0, 0, 0,				// 32 - 47
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,						// 48 - 63
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,						// 64 - 79
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\\', 0, 0, 0,					// 80 - 95
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,						// 96 - 111
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,						// 112 - 127
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,						// 128 - 143
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,						// 144 - 159
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,						// 160 - 175
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,						// 176 - 191
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,						// 192 - 207
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,						// 208 - 223
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,						// 224 - 239
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };					// 240 - 255

const char cINETChar[] = // String chars (IMAP/IMSP?ACAP etc) (0 = OK, 1 = quote, 2 = escape, 3 = literal)
						  { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 0 - 15
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 16 - 31
							1, 0, 2, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,		// 32 - 47
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 48 - 63
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 64 - 79
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0,		// 80 - 95
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 96 - 111
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 3,		// 112 - 127
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 128 - 143
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 144 - 159
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 160 - 175
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 176 - 191
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 192 - 207
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 208 - 223
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// 224 - 239
							3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };	// 240 - 255

const char cINETCharBreak[] = " \"%()\\{}";	// NB Exclude '*' here

const char cQPChar[] = // Allowable quoted-printable chars
						  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,		// 0 - 15
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 16 - 31
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 32 - 47
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,		// 48 - 63
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 64 - 79
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 80 - 95
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 96 - 111
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,		// 112 - 127
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 128 - 143
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 144 - 159
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 160 - 175
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 176 - 191
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 192 - 207
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 208 - 223
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 224 - 239
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	// 240 - 255

const char cNoQuoteChar1522[] = // Chars that must be converted for RFC1522
						  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 0 - 15
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 16 - 31
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 32 - 47
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0,		// 48 - 63
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 64 - 79
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,		// 80 - 95
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 96 - 111
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,		// 112 - 127
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 128 - 143
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 144 - 159
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 160 - 175
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 176 - 191
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 192 - 207
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 208 - 223
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 224 - 239
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	// 240 - 255

const char cNoQuoteChar1522Addr[] = // Chars that must be converted for RFC1522 used in address phrase
						  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 0 - 15
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 16 - 31
							1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,		// 32 - 47
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,		// 48 - 63
							0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 64 - 79
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0,		// 80 - 95
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		// 96 - 111
							1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,		// 112 - 127
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 128 - 143
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 144 - 159
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 160 - 175
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 176 - 191
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 192 - 207
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 208 - 223
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		// 224 - 239
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	// 240 - 255

const char* CharsetTo(ECharset charset)
{
	switch(charset)
	{
	case eUSAscii:
		return cToUSASCII;
	case eISO8859_1:
	case eCharsetUnknown:
	default:
		return cToISO8859_1;
	case eISO8859_2:
		return cToISO8859_2;
	//case eISO8859_3:
	//	return cToISO8859_3;
	//case eISO8859_4:
	//	return cToISO8859_4;
	case eISO8859_5:
		return cToISO8859_5;
	case eISO8859_6:
		return cToISO8859_6;
	case eISO8859_7:
		return cToISO8859_7;
	case eISO8859_8:
		return cToISO8859_8;
	case eISO8859_9:
		return cToISO8859_9;
	case eISO8859_15:
		return cToISO8859_15;
	}
}

const char* CharsetFrom(ECharset charset)
{
	switch(charset)
	{
	case eUSAscii:
	case eUTF8:
		return cFromUSASCII;
	case eISO8859_1:
	case eCharsetUnknown:
	default:
		return cFromISO8859_1;
	case eISO8859_2:
		return cFromISO8859_2;
	//case eISO8859_3:
	//	return cFromISO8859_3;
	//case eISO8859_4:
	//	return cFromISO8859_4;
	case eISO8859_5:
		return cFromISO8859_5;
	case eISO8859_6:
		return cFromISO8859_6;
	case eISO8859_7:
		return cFromISO8859_7;
	case eISO8859_8:
		return cFromISO8859_8;
	case eISO8859_9:
		return cFromISO8859_9;
	case eISO8859_15:
		return cFromISO8859_15;
	case eKOI8_r:
		return cFromKOI8_R;
	case eKOI8_ru:
		return cFromKOI8_RU;
	case eKOI8_u:
		return cFromKOI8_U;
	}
}

#if __dest_os == __linux_os
static ECharset currentCharSet = eISO8859_1;

void SetCurrentCharset(ECharset cs)
{
	currentCharSet = cs;
}
#endif

ECharset GetCurrentCharset()
{

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	return ::ScriptToCharset(::FontScript());
#elif __dest_os == __win32_os
	return CodePageToCharset(::GetACP());
#elif __dest_os == __linux_os
	return currentCharSet; 
#else
#error __dest_os
#endif
}

ECharset GetCharset(const cdstring& charset, ECharset default_charset)
{
	// Add charsets we know about
	if (sNameCode.empty())
	{
		const SNameCode* item1 = &namecodes[0];
		while(item1->name)
		{
			// Add to forward map
			sNameCode[cdstring(item1->name)] = item1->code;
			item1++;
		}
	}

	// Determine charset
	cdstring cname(charset);
	::strlower(cname.c_str_mod());
	name_code_map::const_iterator found = sNameCode.find(cname);
	if (found != sNameCode.end())
		return (*found).second;
	
	// Default unknown is ISO-8859-1
	return default_charset;
}

const char* GetCharsetFromCode(ECharset charset)
{
	if (charset < eCharsetLast)
		return cCharsets[charset];
	
	switch(charset)
	{
	case eKOI8_r:
		return "koi8-r";
	case eKOI8_ru:
		return "koi8-ru";
	case eKOI8_u:
		return "koi8-u";
	case eUTF8:
		return "utf-8";
	default:
		return cCharsets[eUSAscii];
	}
}

#ifndef LINUX_NOTYET
ECharset ScriptToCharset(unsigned long script)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	switch(script)
	{
	case smRoman:
	default:
		return eISO8859_1;
	//case smJapanese:
	//	return eISO8859_1;
	//case smTradChinese:	/* Traditional Chinese*/
	//	return eISO8859_1;
	//case smKorean:
	//	return eISO8859_1;
	case smArabic:
		return eISO8859_6;
	case smHebrew:
		return eISO8859_8;
	case smGreek:
		return eISO8859_7;
	case smCyrillic:
		return eISO8859_5;
	//case smThai:
	//	return eISO8859_11;
	case smCentralEuroRoman:	/* For Czech, Slovak, Polish, Hungarian, Baltic langs*/
		return eISO8859_2;
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
	case SHIFTJIS_CHARSET:
		cp = 932;
		break;
	case HANGUL_CHARSET:
		cp = 949;
		break;
	case GB2312_CHARSET:
		cp = 936;
		break;
	case CHINESEBIG5_CHARSET:
		cp = 950;
		break;
	case JOHAB_CHARSET:
		cp = 1361;
		break;
	case HEBREW_CHARSET:
		cp = 1255;
		break;
	case ARABIC_CHARSET:
		cp = 1256;
		break;
	case GREEK_CHARSET:
		cp = 1253;
		break;
	case TURKISH_CHARSET:
		cp = 1254;
		break;
	case THAI_CHARSET:
		cp = 874;
		break;
	case EASTEUROPE_CHARSET:
		cp = 1250;
		break;
	case RUSSIAN_CHARSET:
		cp = 1251;
		break;
	case BALTIC_CHARSET:
		cp = 1257;
		break;
	}

	// Convert from Code Page into MIME charset
	return ::CodePageToCharset(cp);
#else
#error __dest_os
#endif

}

unsigned long CharsetToScript(ECharset charset)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	switch(charset)
	{
	case eUSAscii:    // ascii
	case eISO8859_1:  // Latin 1 (US, Western Europe)
	case eISO8859_15: // Latin 1 with Euro
	default:
		return smRoman;
	case eISO8859_2: // Central European 
		return smCentralEuroRoman;
	case eISO8859_5: // Cyrillic
	case eKOI8_r:
	case eKOI8_ru:
	case eKOI8_u:
		return smCyrillic;
	case eISO8859_6: // Arabic
		return smArabic;
	case eISO8859_7: // Greek
		return smGreek;
	case eISO8859_8: // Hebrew
		return smHebrew;
	case eISO8859_9: // Turkish
		return smRoman;
	}
#elif __dest_os == __win32_os
	unsigned long cp = ::CharsetToCodePage(charset);
	switch(cp)
	{
	case 874:
		return THAI_CHARSET;
	case 932:
		return SHIFTJIS_CHARSET;
	case 936:
		return GB2312_CHARSET;
	case 949:
		return HANGUL_CHARSET;
	case 950:
		return CHINESEBIG5_CHARSET;
	case 1250:
		return EASTEUROPE_CHARSET;
	case 1251:
		return RUSSIAN_CHARSET;
	case 1252:
	default:
		return ANSI_CHARSET;
	case 1253:
		return GREEK_CHARSET;
	case 1254:
		return TURKISH_CHARSET;
	case 1255:
		return HEBREW_CHARSET;
	case 1256:
		return ARABIC_CHARSET;
	case 1257:
		return BALTIC_CHARSET;
	case 1361:
		return JOHAB_CHARSET;
	}
#else
#error __dest_os
#endif
}
#endif

#if __dest_os == __linux_os
const char* cXCharsets[] = {"", "", "iso8859-1", "iso8859-2", "iso8859-3", "iso8859-4",
								"iso8859-5", "iso8859-6", "iso8859-7", "iso8859-8", "iso8859-9",
								"iso8859-10", "iso8859-11", "iso8859-12", "iso8859-13", "iso8859-14", "iso8859-15"};

cdstring CharsetToXFontSpec(ECharset charset)
{
	if (charset <= eISO8859_15)
		return cXCharsets[charset];
	else
	{
		switch(charset)
		{
		case eKOI8_r:
		case eKOI8_ru:
		case eKOI8_u:
			return cXCharsets[eISO8859_5];
		default:;
		}
	}

	return cXCharsets[eUSAscii];
}
#endif

#if __dest_os == __win32_os
ECharset CodePageToCharset(unsigned long cp)
{
	// Convert from Code Page into MIME charset
	switch(cp)
	{
	//case 874: // Thai
	//	return eISO8859_11;
	//case 932: // Japan
	//	return eISO8859_1;
	//case 936: // Chinese (PRC, Singapore)
	//	return eISO8859_1;
	//case 949: // Korean
	//	return eISO8859_1;
	//case 950: // Chinese (Taiwan, Hong Kong) 
	//	return eISO8859_1;
	//case 1200: // Unicode (BMP of ISO 10646)
	//	return eISO8859_1;

	case 1250: // Central European 
		return eISO8859_2;
	case 1251: // Cyrillic
		return eISO8859_5;
	case 1252: // Latin 1 (US, Western Europe)
	default:
		return eISO8859_1;
	case 1253: // Greek
		return eISO8859_7;
	case 1254: // Turkish
		return eISO8859_9;
	case 1255: // Hebrew
		return eISO8859_8;
	case 1256: // Arabic
		return eISO8859_6;
	case 1257: // Baltic
		return eISO8859_4;
	}
}

unsigned long CharsetToCodePage(ECharset charset)
{
	// Convert from Code Page into MIME charset
	switch(charset)
	{
	case eUSAscii:    // ascii
	case eISO8859_1:  // Latin 1 (US, Western Europe)
	case eISO8859_15: // Latin 1 with Euro
	default:
		return 1252;
	case eISO8859_2: // Central European 
		return 1250;
	case eISO8859_4: // Baltic
		return 1257;
	case eISO8859_5: // Cyrillic
	case eKOI8_r:
	case eKOI8_ru:
	case eKOI8_u:
		return 1251;
	case eISO8859_6: // Arabic
		return 1256;
	case eISO8859_7: // Greek
		return 1253;
	case eISO8859_8: // Hebrew
		return 1255;
	case eISO8859_9: // Turkish
		return 1254;
	//case eISO8859_11: // Thai
	//	return 874;
	}
}
#endif
