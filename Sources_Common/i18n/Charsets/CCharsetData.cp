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


// Source for CCharsetData class

// This file #include'd by CCharsetManager

// Type for name-code map data
struct SNameCode
{
	const char* name;
	ECharsetCode code;
};

// Charset names mapped to codes (these must always be lowercase)
const SNameCode namecodes[] =
{
	{"us-ascii", eUSASCII},

	{"iso-8859-1", eISO_8859_1},
	{"iso_8859-1", eISO_8859_1},
	{"latin1", eISO_8859_1},

	{"iso-8859-2", eISO_8859_2},
	{"iso_8859-2", eISO_8859_2},
	{"latin2", eISO_8859_2},

	{"iso-8859-3", eISO_8859_3},
	{"iso_8859-3", eISO_8859_3},
	{"latin3", eISO_8859_3},

	{"iso-8859-4", eISO_8859_4},
	{"iso_8859-4", eISO_8859_4},
	{"latin4", eISO_8859_4},

	{"iso-8859-5", eISO_8859_5},
	{"iso_8859-5", eISO_8859_5},
	{"cyrillic", eISO_8859_5},

	{"iso-8859-6", eISO_8859_6},
	{"iso_8859-6", eISO_8859_6},
	{"arabic", eISO_8859_6},

	{"iso-8859-7", eISO_8859_7},
	{"iso_8859-7", eISO_8859_7},
	{"greek", eISO_8859_7},

	{"iso-8859-8", eISO_8859_8},
	{"iso_8859-8", eISO_8859_8},
	{"hebrew", eISO_8859_8},

	{"iso-8859-9", eISO_8859_9},
	{"iso_8859-9", eISO_8859_9},
	{"latin5", eISO_8859_9},

	{"iso-8859-10", eISO_8859_10},
	{"iso_8859-10", eISO_8859_10},
	{"latin6", eISO_8859_10},

	{"iso-8859-11", eISO_8859_11},
	{"iso_8859-11", eISO_8859_11},
	{"thai", eISO_8859_11},

	{"iso-8859-13", eISO_8859_13},
	{"iso_8859-13", eISO_8859_13},
	{"latin7", eISO_8859_13},

	{"iso-8859-14", eISO_8859_14},
	{"iso_8859-14", eISO_8859_14},
	{"latin8", eISO_8859_14},

	{"iso-8859-15", eISO_8859_15},
	{"iso_8859-15", eISO_8859_15},
	{"latin9", eISO_8859_15},

	{"iso-8859-16", eISO_8859_16},
	{"iso_8859-16", eISO_8859_16},
	{"latin10", eISO_8859_16},

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
	
	{"koi8-r", eKOI8_r},
	{"koi8-ru", eKOI8_ru},
	{"koi8-u", eKOI8_u},

	// Two bytes
	{"gb2312", eGB2312},
	{"csgb2312", eGB2312},

	{"big5", eBig5},
	{"csbig5", eBig5},

	{"jis_x0201", eJIS_0201},

	{"jis0208", eJIS_0208},
	{"jis_x0208", eJIS_0208},

	{"shift-jis", eSJIS},
	{"shift_jis", eSJIS},
	{"sjis", eSJIS},
	
	{"ks_c_5601-1987", eKSC_5601_1987},
	{"iso-ir-149", eKSC_5601_1987},
	{"ks_c_5601-1989", eKSC_5601_1987},
	{"ksc_5601", eKSC_5601_1987},
	{"korean", eKSC_5601_1987},
	{"csksc56011987", eKSC_5601_1987},
	
	// EUC
	{"euc-cn", eEUC_CN},

	{"euc-jp", eEUC_JP},

	{"euc-kr", eEUC_KR},
	{"cseuckr", eEUC_KR},

	{"euc-tw", eEUC_TW},

	// ISO 2022
	{"iso-2022-jp", eISO2022_JP},
	{"csiso2022jp", eISO2022_JP},

	{"iso-2022-cn", eISO2022_CN},
	
	{"iso-2022-kr", eISO2022_KR},
	{"csiso2022kr", eISO2022_KR},
	
	// Unicode
	{"ucs-2", eUCS2},
	{"ucs-4", eUCS4},
	{"utf-8", i18n::eUTF8},
	{"utf-16", eUTF16},

	// Last one to trigger end of loop
	{NULL, eUSASCII}
};

// Map charset code from network to charset code for local host
// Array: [0] - network code
//        [1] - Mac OS code
//        [2] - Win32 code
//        [3] - unix code

const ECharsetCode ntoh_code[][4] =
{
	{eUSASCII, eUSASCII, eUSASCII, eUSASCII},
	{eISO_8859_1, eMacRoman, eWindowsLatin1, eISO_8859_1},
	{eISO_8859_2, eMacCentralEurRoman, eWindowsLatin2, eISO_8859_2},
	//{"iso_8859_3", eISO_8859_3},
	//{"iso_8859_4", eISO_8859_4},
	{eISO_8859_5, eMacCyrillic, eWindowsCyrillic, eISO_8859_5},
	{eISO_8859_6, eMacArabic, eWindowsArabic, eISO_8859_6},
	{eISO_8859_7, eMacGreek, eWindowsGreek, eISO_8859_7},
	{eISO_8859_8, eMacHebrew, eWindowsHebrew, eISO_8859_8},
	{eISO_8859_9, eMacTurkish, eWindowsTurkish, eISO_8859_9},
	{eISO_8859_10, eISO_8859_10, eWindowsLatin6, eISO_8859_10},
	{eISO_8859_11, eMacThai, eWindowsThai, eISO_8859_11},
	{eISO_8859_13, eMacBalticRim, eWindowsBalticRim, eISO_8859_13},
	{eISO_8859_14, eMacCeltic, eWindowsLatin8, eISO_8859_14},
	{eISO_8859_15, eMacLatin9, eWindowsLatin9, eISO_8859_15},
	{eISO_8859_16, eMacLatin10, eWindowsLatin10, eISO_8859_16},
	
	{i18n::eKOI8_r, eMacCyrillic, eWindowsCyrillic, eISO_8859_5},
	{i18n::eKOI8_ru, eMacCyrillic, eWindowsCyrillic, eISO_8859_5},
	{i18n::eKOI8_u, eMacCyrillic, eWindowsCyrillic, eISO_8859_5},
	
	{eGB2312, eMacSimpChinese, eWindowsSimpChinese, eGB2312},
	{eBig5, eMacTradChinese, eWindowsTradChinese, eBig5},
	{eJIS_0201, eMacJapanese, eWindowsJapanese, eJIS_0201},
	{eJIS_0208, eMacJapanese, eWindowsJapanese, eJIS_0208},
	{eSJIS, eMacJapanese, eWindowsJapanese, eSJIS},
	{eKSC_5601_1987, eMacKorean, eWindowsKorean, eKSC_5601_1987},
	
	{eISO2022_JP, eMacJapanese, eWindowsJapanese, eSJIS},
	{eISO2022_CN, eMacSimpChinese, eWindowsSimpChinese, eGB2312},
	{eISO2022_KR, eMacKorean, eWindowsKorean, eISO2022_KR},
	
	// Force conversion of utf8 into unicode
	{i18n::eUTF8, eUCS2, eUCS2, eUCS2},
	
	// Last one to trigger end of loop
	{eNone, eNone, eNone, eNone}
};

// Map charset code from host to charset code for network
// Array: [0] - local host code
//        [1] - network code

const ECharsetCode hton_code[][2] =
{
	{eMacRoman, eISO_8859_1},
	{eMacCentralEurRoman, eISO_8859_2},
	{eMacCyrillic, eISO_8859_5},
	{eMacArabic, eISO_8859_6},
	{eMacGreek, eISO_8859_7},
	{eMacHebrew, eISO_8859_8},
	{eMacTurkish, eISO_8859_9},
	{eMacThai, eISO_8859_11},
	{eMacBalticRim, eISO_8859_13},
	{eMacCeltic, eISO_8859_14},
	{eMacLatin9, eISO_8859_15},
	{eMacLatin10, eISO_8859_16},
	
	{eMacJapanese, eISO2022_JP},
	{eMacSimpChinese, eGB2312},
	{eMacTradChinese, eBig5},
	{eMacKorean, eISO2022_KR},
	
	{eWindowsLatin1, eISO_8859_1},
	{eWindowsLatin2, eISO_8859_2},
	{eWindowsCyrillic, eISO_8859_5},
	{eWindowsArabic, eISO_8859_6},
	{eWindowsGreek, eISO_8859_7},
	{eWindowsHebrew, eISO_8859_8},
	{eWindowsTurkish, eISO_8859_9},
	{eWindowsLatin6, eISO_8859_10},
	{eWindowsThai, eISO_8859_11},
	{eWindowsBalticRim, eISO_8859_13},
	{eWindowsLatin8, eISO_8859_14},
	{eWindowsLatin9, eISO_8859_15},
	{eWindowsLatin10, eISO_8859_16},
	
	{eWindowsJapanese, eISO2022_JP},
	{eWindowsSimpChinese, eGB2312},
	{eWindowsTradChinese, eBig5},
	{eWindowsKorean, eISO2022_KR},

	// Last one to trigger end of loop
	{eNone, eNone}
};

// Descriptive names for scripts
const char* fontmap_descriptor[] =
{
	"Western European",
	"Central European",
	"Cyrillic",
	"Arabic",
	"Greek",
	"Hebrew",
	"Turkish",
	"Japanese",
	"Simplified Chinese",
	"Traditional Chinese",
	"Korean",
	"Unicode",
	"User Defined"
};

// Type for name-code map data
struct SCharsetFontMap
{
	ECharsetCode charset;
	EFontMapCode font;
};

// Charset names mapped to codes
const SCharsetFontMap charset2font[] =
{
	{eUSASCII, eWesternEuropean},
	
	// ISO-8859
	{eISO_8859_1, eWesternEuropean},
	{eISO_8859_2, eCentralEuropean},
	{eISO_8859_3, eWesternEuropean},
	{eISO_8859_4, eWesternEuropean},
	{eISO_8859_5, eCyrillic},
	{eISO_8859_6, eArabic},
	{eISO_8859_7, eGreek},
	{eISO_8859_8, eHebrew},
	{eISO_8859_9, eTurkish},
	{eISO_8859_10, eWesternEuropean},
	{eISO_8859_11, eWesternEuropean},
	{eISO_8859_13, eCentralEuropean},
	{eISO_8859_14, eWesternEuropean},
	{eISO_8859_15, eWesternEuropean},
	{eISO_8859_16, eWesternEuropean},
	
	// Mac encodings
	{eMacRoman, eWesternEuropean},
	{eMacCentralEurRoman, eCentralEuropean},
	{eMacCyrillic, eCyrillic},
	{eMacArabic, eArabic},
	{eMacGreek, eGreek},
	{eMacHebrew, eHebrew},
	{eMacTurkish, eTurkish},
	{eMacLatin6, eWesternEuropean},
	{eMacThai, eWesternEuropean},
	{eMacBalticRim, eWesternEuropean},
	{eMacCeltic, eWesternEuropean},
	{eMacLatin9, eWesternEuropean},
	{eMacLatin10, eWesternEuropean},
	{eMacJapanese, eJapanese},
	{eMacSimpChinese, eSimpChinese},
	{eMacTradChinese, eTradChinese},
	{eMacKorean, eKorean},

	// Windows encodings
	{eWindowsLatin1, eWesternEuropean},
	{eWindowsLatin2, eCentralEuropean},
	{eWindowsCyrillic, eCyrillic},
	{eWindowsArabic, eArabic},
	{eWindowsGreek, eGreek},
	{eWindowsHebrew, eHebrew},
	{eWindowsTurkish, eTurkish},
	{eWindowsLatin6, eWesternEuropean},
	{eWindowsThai, eWesternEuropean},
	{eWindowsBalticRim, eCentralEuropean},
	{eWindowsLatin8, eWesternEuropean},
	{eWindowsLatin9, eWesternEuropean},
	{eWindowsLatin10, eWesternEuropean},
	{eWindowsJapanese, eJapanese},
	{eWindowsSimpChinese, eSimpChinese},
	{eWindowsTradChinese, eTradChinese},
	{eWindowsKorean, eKorean},
	
	// Other common one-byte scripts
	{i18n::eKOI8_r, eCyrillic},
	{i18n::eKOI8_ru, eCyrillic},
	{i18n::eKOI8_u, eCyrillic},
	
	// Two-bytes
	{eGB2312, eWesternEuropean},
	{eBig5, eWesternEuropean},
	{eJIS_0201, eJapanese},
	{eJIS_0208, eJapanese},
	{eSJIS, eJapanese},
	{eKSC_5601_1987, eKorean},
	
	// ISO-2022
	{eISO2022_JP, eJapanese},
	{eISO2022_CN, eSimpChinese},
	{eISO2022_KR, eKorean},
	
	// Unicode
	{eUCS2, eUnicode},
	{eUCS4, eUnicode},
	{i18n::eUTF8, eUnicode},
	
	// Last item to terminate loop
	{eNone, eWesternEuropean}
};
