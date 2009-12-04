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


// Header for CCharsetCodes

#ifndef __CCHARSETCODES__MULBERRY__
#define __CCHARSETCODES__MULBERRY__

namespace i18n 
{
	enum ECharsetCode
	{
		eNone = 0,
		eUnknown = 0,
		eAutomatic = 0,
		eUSASCII,
		
		// ISO-8859
		eISO_8859_1,
		eISO_8859_2,
		eISO_8859_3,
		eISO_8859_4,
		eISO_8859_5,
		eISO_8859_6,
		eISO_8859_7,
		eISO_8859_8,
		eISO_8859_9,
		eISO_8859_10,
		eISO_8859_11,
		eISO_8859_13,
		eISO_8859_14,
		eISO_8859_15,
		eISO_8859_16,
		
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
		
		// Other common one-byte scripts
		eKOI8_r = 300,
		eKOI8_ru,
		eKOI8_u,
		
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
		eKSC_5601_1987,
		
		// EUC
		eEUC_CN = 450,
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
		eUTF8 = 1L << 12,
		eUTF16 = 1L << 13,

		// Local ranges
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		eFirstLocal = eMacRoman,
		eLastLocal = eMacKorean
#elif __dest_os == __win32_os
		eFirstLocal = eWindowsLatin1,
		eLastLocal = eWindowsKorean
#elif __dest_os == __linux_os
		eFirstLocal = eISO_8859_1,
		eLastLocal = eISO_8859_16
#endif
	};
	
	enum EFontMapCode
	{
		eWesternEuropean = 0,
		eCentralEuropean,
		eCyrillic,
		eArabic,
		eGreek,
		eHebrew,
		eTurkish,
		eJapanese,
		eSimpChinese,
		eTradChinese,
		eKorean,
		eUnicode,
		eUserDefined
	};
}
#endif
