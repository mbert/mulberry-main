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


// Source for COptionsMap class

#include "COptionsMap.h"

#include "CACL.h"
#include "CAdbkACL.h"
#include "CAddressAccount.h"
#include "char_stream.h"
#include "CMboxACL.h"
#if __dest_os == __win32_os
#include "CFontMenu.h"
#endif
#include "CNotification.h"
#include "CPreferenceItem.h"
#include "CPreferenceKeys.h"
#include "CStringUtils.h"

#include <stdio.h>
#include <stdlib.h>

// __________________________________________________________________________________________________
// C L A S S __ C O P T I O N S M A P
// __________________________________________________________________________________________________

const char* cValueSpace = ", ";
extern const char* cSpace;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Copy from raw list
COptionsMap::COptionsMap() :
	mSeparator(".")
{
}

// Default destructor
COptionsMap::~COptionsMap(void)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

#pragma mark ____________________________cdstring

// Write key & string
bool COptionsMap::WriteValue(const cdstring& key, const cdstring& value, bool quote, bool convert)
{
	// Quote it and do possible charset/line-end conversion
	cdstring temp = value;
	if (quote)
		temp.quote();
	if (convert)
		temp.ConvertFromOS();
	return WriteKeyValue(key, temp);
}

// Read string from key
bool COptionsMap::ReadValue(const cdstring& key, cdstring& value, NumVersion vers_prefs, bool convert)
{
	bool result = ReadKeyValue(key, value);
	
	// Do possible charset/line-end conversion and unquote
	if (result)
	{
		value.unquote();
		if (convert)
			value.ConvertToOS();
	}
	return result;
}

#pragma mark ____________________________cdstrvect
// Write key & array of strings
bool COptionsMap::WriteValue(const cdstring& key, const cdstrvect& value)
{
#if 0
	cdstring all;
	bool first = true;
	for(cdstrvect::const_iterator iter = value.begin(); iter != value.end(); iter++)
	{
		if (!first)
			all += cValueSpace;
		else
			first = false;

		// Do charset/line-end conversion
		cdstring temp(*iter);
		temp.ConvertFromOS();
		all += temp;
	}
	return WriteKeyValue(key, all);
#endif

	// New way - use S-Expressions
	cdstring sexpression;
	sexpression.CreateSExpression(value);
	sexpression.ConvertFromOS();
	return WriteKeyValue(key, sexpression);
}

// Read key & array of strings
bool COptionsMap::ReadValue(const cdstring& key, cdstrvect& value, NumVersion vers_prefs)
{
	cdstring all;
	bool result = ReadKeyValue(key, all);

	if (result)
	{
		// Clear existing
		value.clear();

		// Look for new style s-expression
		if ((all[0UL] == '(') &&
			(all[all.length() - 1] == ')'))
		{
			all.ConvertToOS();
			all.ParseSExpression(value);
		}
		else
		{
			// Tokenise on commas
			char* p = ::strtok(all, ",");
			while(p)
			{
				// Add to array
				value.push_back(p);

				// Convert charset/line-end
				value.back().ConvertToOS();

				// Get next token and step over leading space
				p = ::strtok(nil, ",");
				if (p && (*p == ' ')) p++;
			}
		}
	}

	return result;
}

#pragma mark ____________________________cdstrmap
// Write key & array of strings
bool COptionsMap::WriteValue(const cdstring& key, const cdstrmap& value)
{
	// Use S-Expressions
	cdstring sexpression;
	sexpression.CreateSExpression(value);
	sexpression.ConvertFromOS();
	return WriteKeyValue(key, sexpression);
}

// Read key & array of strings
bool COptionsMap::ReadValue(const cdstring& key, cdstrmap& value, NumVersion vers_prefs)
{
	cdstring all;
	bool result = ReadKeyValue(key, all);

	if (result)
	{
		// Clear existing
		value.clear();

		// Use S-Expressions
		all.ConvertToOS();
		all.ParseSExpression(value);
	}

	return result;
}

#pragma mark ____________________________cdstruint32map
// Write key & array of strings
bool COptionsMap::WriteValue(const cdstring& key, const cdstruint32map& value)
{
	// Convert map to cdstrvect
	cdstrvect temp;
	for(cdstruint32map::const_iterator iter = value.begin(); iter != value.end(); iter++)
	{
		temp.push_back((*iter).first);
		temp.push_back(cdstring((unsigned long)(*iter).second));
	}

	// Use S-Expressions
	cdstring sexpression;
	sexpression.CreateSExpression(temp);
	sexpression.ConvertFromOS();
	return WriteKeyValue(key, sexpression);
}

// Read key & array of strings
bool COptionsMap::ReadValue(const cdstring& key, cdstruint32map& value, NumVersion vers_prefs)
{
	cdstring all;
	bool result = ReadKeyValue(key, all);

	if (result)
	{
		// Clear existing
		value.clear();

		// Use S-Expressions
		cdstrvect temp;
		all.ConvertToOS();
		all.ParseSExpression(temp);
		
		// Add each vector pair to map
		for(cdstrvect::const_iterator iter = temp.begin(); iter != temp.end(); iter += 2)
			value.insert(cdstruint32map::value_type(*iter, ::strtoul(*(iter+1), NULL, 10)));
	}

	return result;
}

#pragma mark ____________________________cdstrset
// Write key & array of strings
bool COptionsMap::WriteValue(const cdstring& key, const cdstrset& value)
{
	// Use S-Expressions
	cdstring sexpression;
	sexpression.CreateSExpression(value);
	sexpression.ConvertFromOS();
	return WriteKeyValue(key, sexpression);
}

// Read key & array of strings
bool COptionsMap::ReadValue(const cdstring& key, cdstrset& value, NumVersion vers_prefs)
{
	cdstring all;
	bool result = ReadKeyValue(key, all);

	if (result)
	{
		// Clear existing
		value.clear();

		// Use S-Expressions
		all.ConvertToOS();
		all.ParseSExpression(value);
	}

	return result;
}

#pragma mark ____________________________CPreferenceItem
// Write key & CPreferenceItem
bool COptionsMap::WriteValue(const cdstring& key, const CPreferenceItem& value)
{
	// Get string reprsentation and write it out
	return WriteKeyValue(key, value.GetInfo());
}

bool COptionsMap::ReadValue(const cdstring& key, CPreferenceItem& value, NumVersion vers_prefs)
{
	// Read in string representation
	cdstring all;
	bool result = ReadKeyValue(key, all);

	if (result)
	{
		char_stream stream(all.c_str_mod());
		result = value.SetInfo(stream, vers_prefs);
	}
	return result;
}

#pragma mark ____________________________CStringArray
#if __dest_os == __win32_os
// Write key & array of strings
bool COptionsMap::WriteValue(const cdstring& key, const CStringArray& value)
{
	cdstring all;
	bool first = true;
	for(int i = 0; i < value.GetSize(); i++)
	{
		if (!first)
			all += cValueSpace;
		else
			first = false;

		// Do charset/line-end conversion
		cdstring temp(value[i]);
		temp.ConvertFromOS();
		all += temp;
	}
	return WriteKeyValue(key, all);
}

// Read key & array of strings
bool COptionsMap::ReadValue(const cdstring& key, CStringArray& value, NumVersion vers_prefs)
{
	cdstring all;
	bool result = ReadKeyValue(key, all);

	if (result)
	{
		// Clear existing
		value.RemoveAll();

		// Tokenise on commas
		char* p = ::strtok(all, ",");
		while(p)
		{
			// Do charset/line-end conversion
			cdstring temp(p);
			temp.ConvertToOS();

			// Add to array
			value.Add(temp.win_str());

			// Get next token and step over leading space
			p = ::strtok(nil, ",");
			if (p && (*p == ' ')) p++;
		}
	}

	return result;
}
#endif

#pragma mark ____________________________char
// Write key & char
bool COptionsMap::WriteValue(const cdstring& key, char value)
{
	cdstring str_value = value;
	str_value.ConvertFromOS();
	return WriteKeyValue(key, str_value);
}

// Read char from key
bool COptionsMap::ReadValue(const cdstring& key, char& value, NumVersion vers_prefs)
{
	cdstring str_value;
	bool result = ReadKeyValue(key, str_value);
	if (result)
	{
		str_value.ConvertToOS();
		if (str_value.empty())
			value = 0;
		else
			value = str_value[0UL];
	}
	return result;
}

#pragma mark ____________________________short
// Write key & short
bool COptionsMap::WriteValue(const cdstring& key, short value)
{
	cdstring str_value = (long) value;
	return WriteKeyValue(key, str_value);
}

// Read short from key
bool COptionsMap::ReadValue(const cdstring& key, short& value, NumVersion vers_prefs)
{
	cdstring str_value;
	bool result = ReadKeyValue(key, str_value);
	if (result)
		value = ::atoi(str_value);
	return result;
}

#pragma mark ____________________________long
// Write key & long
bool COptionsMap::WriteValue(const cdstring& key, long value)
{
	cdstring str_value = value;
	return WriteKeyValue(key, str_value);
}

// Read long from key
bool COptionsMap::ReadValue(const cdstring& key, long& value, NumVersion vers_prefs)
{
	cdstring str_value;
	bool result = ReadKeyValue(key, str_value);
	if (result)
		value = ::atol(str_value);
	return result;
}

#pragma mark ____________________________int
// Write key & long
bool COptionsMap::WriteValue(const cdstring& key, int value)
{
	cdstring str_value = (long) value;
	return WriteKeyValue(key, str_value);
}

// Read int from key
bool COptionsMap::ReadValue(const cdstring& key, int& value, NumVersion vers_prefs)
{
	cdstring str_value;
	bool result = ReadKeyValue(key, str_value);
	if (result)
		value = ::atoi(str_value);
	return result;
}

#pragma mark ____________________________bool
// Write key & bool
bool COptionsMap::WriteValue(const cdstring& key, bool value)
{
	cdstring str_value = (value ? cValueBoolTrue : cValueBoolFalse);
	return WriteKeyValue(key, str_value);
}

// Read bool from key
bool COptionsMap::ReadValue(const cdstring& key, bool& value, NumVersion vers_prefs)
{
	cdstring str_value;
	bool result = ReadKeyValue(key, str_value);
	if (result)
	{
		::strlower(str_value);
		if (str_value == cValueBoolTrue)
			value = true;
		else
			value = false;
	}

	return result;
}

#pragma mark ____________________________boolvector
// Write key & boolvector
bool COptionsMap::WriteValue(const cdstring& key, boolvector& value)
{
	cdstring str_value;
	bool first = true;
	for(boolvector::const_iterator iter = value.begin(); iter != value.end(); iter++)
	{
		if (first)
			first = false;
		else
			str_value += cValueSpace;

		str_value += (*iter ? cValueBoolTrue : cValueBoolFalse);
	}
	return WriteKeyValue(key, str_value);
}

// Read boolvector from key
bool COptionsMap::ReadValue(const cdstring& key, boolvector& value, NumVersion vers_prefs)
{
	cdstring str_value;
	bool result = ReadKeyValue(key, str_value);
	if (result)
	{
		// Clear existing
		value.clear();

		// Make everything lower case
		::strlower(str_value);

		// Tokenise on comma
		char* p = ::strtok((char*) str_value.c_str(), ",");
		while(p)
		{
			value.push_back(::strcmp(p, cValueBoolTrue) == 0);
			p = ::strtok(nil, ",");
		}
	}

	return result;
}

#pragma mark ____________________________OSType
// Write key & OSType
bool COptionsMap::WriteValue(const cdstring& key, OSType value)
{
	cdstring str_value((char*) &value, 4);
	return WriteKeyValue(key, str_value);
}

// Read OSType from key
bool COptionsMap::ReadValue(const cdstring& key, OSType& value, NumVersion vers_prefs)
{
	cdstring str_value;
	bool result = ReadKeyValue(key, str_value);
	if (result)
	{
		value = *(const OSType*) str_value.c_str();
	}

	return result;
}

#pragma mark ____________________________SStyleTraits
// Write key & SStyleTraits
bool COptionsMap::WriteValue(const cdstring& key, const SStyleTraits& value)
{
	char txt[256];
	int bold_style = (value.style & bold) ? 1 : 0;
	int italic_style = (value.style & italic) ? 1 : 0;
	int underline_style = (value.style & underline) ? 1 : 0;
	int strike_style = (value.style & strike_through) ? 1 : 0;
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
	::snprintf(txt, 256, "%d, %d, %d, %d, %ld, %ld, %ld", bold_style, italic_style, underline_style, strike_style,
				value.color.red >> 8, value.color.green >> 8, value.color.blue >> 8);
#elif __dest_os == __win32_os
	::snprintf(txt, 256, "%d, %d, %d, %d, %d, %d, %d", bold_style, italic_style, underline_style, strike_style,
				GetRValue(value.color), GetGValue(value.color), GetBValue(value.color));
#endif
	return WriteKeyValue(key, txt);
}

// Read SStyleTraits from key
bool COptionsMap::ReadValue(const cdstring& key, SStyleTraits& value, NumVersion vers_prefs)
{
	cdstring str_value;
	bool result = ReadKeyValue(key, str_value);
	if (result)
	{
		int bold_style = 0;
		int italic_style = 0;
		int underline_style = 0;
		int strike_style = 0;
		int red, green, blue;

		// Count items
		char* p = str_value;
		int count = 0;
		while(*p)
		{
			if (*p++ == ',') count++;
		}
		if (count == 5)
			::sscanf(str_value, "%d, %d, %d, %d, %d, %d",
						&bold_style, &italic_style, &strike_style, &red, &green, &blue);
		else
			::sscanf(str_value, "%d, %d, %d, %d, %d, %d, %d",
						&bold_style, &italic_style, &underline_style, &strike_style, &red, &green, &blue);
		value.style = 0;
		if (bold_style)
			value.style |= bold;
		if (italic_style)
			value.style |= italic;
		if (underline_style)
			value.style |= underline;
		if (strike_style)
			value.style |= strike_through;
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
		value.color.red = (red << 8) | red;
		value.color.green = (green << 8) | green;
		value.color.blue = (blue << 8) | blue;
#elif __dest_os == __win32_os
		value.color = RGB(red, green, blue);
#endif
	}

	return result;
}

#pragma mark ____________________________RGBColour
// Write key & SStyleTraits
bool COptionsMap::WriteValue(const cdstring& key, const _RGBColor& value)
{
	cdstring all;
	all += '(';
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
	all += cdstring(static_cast<long>(value.red >> 8));
#elif __dest_os == __win32_os
	all += cdstring(static_cast<long>(GetRValue(value._c)));
#endif
	all += cSpace;
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
	all += cdstring(static_cast<long>(value.green >> 8));
#elif __dest_os == __win32_os
	all += cdstring(static_cast<long>(GetGValue(value._c)));
#endif
	all += cSpace;
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
	all += cdstring(static_cast<long>(value.blue >> 8));
#elif __dest_os == __win32_os
	all += cdstring(static_cast<long>(GetBValue(value._c)));
#endif
	all += ')';

	return WriteKeyValue(key, all);
}

// Read SStyleTraits from key
bool COptionsMap::ReadValue(const cdstring& key, _RGBColor& value, NumVersion vers_prefs)
{
	cdstring all;
	bool result = ReadKeyValue(key, all);
	if (result)
	{
		// Parse S-Expression list
		char_stream txt(all.c_str_mod());
		if (txt.start_sexpression())
		{
			long red;
			txt.get(red);
			long green;
			txt.get(green);
			long blue;
			txt.get(blue);

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
			value.red = (red << 8) | red;
			value.green = (green << 8) | green;
			value.blue = (blue << 8) | blue;
#elif __dest_os == __win32_os
			value._c = RGB(red, green, blue);
#endif

			txt.end_sexpression();
		}
	}

	return result;
}

#pragma mark ____________________________CColourList
// Write key & SStyleTraits
bool COptionsMap::WriteValue(const cdstring& key, const CColourList& value)
{
	cdstring all;
	all += '(';
	for(CColourList::const_iterator iter = value.begin(); iter != value.end(); iter++)
	{
		all += '(';
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
		all += cdstring(static_cast<long>((*iter).red >> 8));
#elif __dest_os == __win32_os
		all += cdstring(static_cast<long>(GetRValue(*iter)));
#endif
		all += cSpace;
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
		all += cdstring(static_cast<long>((*iter).green >> 8));
#elif __dest_os == __win32_os
		all += cdstring(static_cast<long>(GetGValue(*iter)));
#endif
		all += cSpace;
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
		all += cdstring(static_cast<long>((*iter).blue >> 8));
#elif __dest_os == __win32_os
		all += cdstring(static_cast<long>(GetBValue(*iter)));
#endif
		all += ')';
	}
	all += ')';

	return WriteKeyValue(key, all);
}

// Read SStyleTraits from key
bool COptionsMap::ReadValue(const cdstring& key, CColourList& value, NumVersion vers_prefs)
{
	cdstring all;
	bool result = ReadKeyValue(key, all);
	if (result)
	{
		// Clear existing
		value.clear();

		// Parse S-Expression list
		char_stream txt(all.c_str_mod());
		if (txt.start_sexpression())
		{
			while(txt.start_sexpression())
			{
				long red;
				txt.get(red);
				long green;
				txt.get(green);
				long blue;
				txt.get(blue);

				RGBColor color;
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
				color.red = (red << 8) | red;
				color.green = (green << 8) | green;
				color.blue = (blue << 8) | blue;
#elif __dest_os == __win32_os
				color = RGB(red, green, blue);
#endif
				value.push_back(color);

				txt.end_sexpression();
			}

			txt.end_sexpression();
		}
	}

	return result;
}

#pragma mark ____________________________CINETAccountList
// Write key & CINETAccountList
bool COptionsMap::WriteValue(const cdstring& key, const CINETAccountList& value)
{
	// Use S_expression lists
	cdstring all = '(';
	for(CINETAccountList::const_iterator iter = value.begin(); iter != value.end(); iter++)
	{
		all += '(';
		all += (*iter)->GetInfo();
		all += ')';
	}
	all += ')';
	return WriteKeyValue(key, all);
}

// Read CINETAccountList from key
bool COptionsMap::ReadValue(const cdstring& key, CINETAccountList& value, NumVersion vers_prefs)
{
	cdstring all;
	bool result = ReadKeyValue(key, all);

	if (result)
	{
		// Clear existing
		value.clear();

		// Parse S-Expression list
		char_stream txt(all.c_str_mod());
		if (txt.start_sexpression())
		{
			while(txt.start_sexpression())
			{
				CINETAccount* acct = nil;

				if (key.compare_end(cMailAccountsKey_1_4) ||
					key.compare_end(cMailAccountsKey_2_0))
					// Create mail account
					acct = new CMailAccount;
				else if (key.compare_end(cSMTPAccountsKey_1_4) ||
							key.compare_end(cSMTPAccountsKey_2_0))
					// Create SMTP account
					acct = new CSMTPAccount;
				else if (key.compare_end(cRemoteAccountsKey_1_4) ||
							key.compare_end(cRemoteAccountsKey_2_0))
					// Create remote account
					acct = new COptionsAccount;
				else if (key.compare_end(cAddressAccountsKey_1_3) ||
							key.compare_end(cAddressAccountsKey_1_4) ||
							key.compare_end(cAddressAccountsKey_2_0) ||
							key.compare_end(cLocalAdbkAccount) ||
							key.compare_end(cOSAdbkAccount))
					// Create address account
					acct = new CAddressAccount;
				else if (key.compare_end(cSIEVEAccountsKey))
					// Create SIEVE account
					acct = new CManageSIEVEAccount;
				else if (key.compare_end(cCalendarAccountsKey) || key.compare_end(cWebCalendarAccountKey))
					// Create calendar account
					acct = new CCalendarAccount;

				// Parse into account and add to list
				if (acct != NULL)
				{
					result = acct->SetInfo(txt, vers_prefs) && result;
					value.push_back(acct);
				}

				txt.end_sexpression();
			}

			txt.end_sexpression();
		}
	}

	return result;
}

#pragma mark ____________________________CNotification
// Write key & CNotification
bool COptionsMap::WriteValue(const cdstring& key, const CNotification& value)
{
	// Use S_expression lists
	cdstring sexpression = value.GetInfo();
	return WriteKeyValue(key, sexpression);
}

// Read CNotification from key
bool COptionsMap::ReadValue(const cdstring& key, CNotification& value, NumVersion vers_prefs)
{
	cdstring sexpression;
	bool result = ReadKeyValue(key, sexpression);

	if (result)
	{
		// Parse S-Expression list
		char_stream p(sexpression.c_str_mod());
		result = value.SetInfo(p, vers_prefs);
	}

	return result;
}

#pragma mark ____________________________CMailNotificationList
// Write key & CMailNotificationList
bool COptionsMap::WriteValue(const cdstring& key, const CMailNotificationList& value)
{
	// Use S_expression lists
	cdstring sexpression = value.GetInfo();
	return WriteKeyValue(key, sexpression);
}

// Read CMailNotificationList from key
bool COptionsMap::ReadValue(const cdstring& key, CMailNotificationList& value, NumVersion vers_prefs)
{
	cdstring sexpression;
	bool result = ReadKeyValue(key, sexpression);

	if (result)
	{
		// Parse S-Expression list
		char_stream p(sexpression.c_str_mod());
		result = value.SetInfo(p, vers_prefs);
	}

	return result;
}

#pragma mark ____________________________CFavouriteItemList
// Write key & CFavouriteItemList
bool COptionsMap::WriteValue(const cdstring& key, const CFavouriteItemList& value)
{
	// Use S_expression lists
	return WriteKeyValue(key, value.GetInfo());
}

// Read CFavouriteItemList from key
bool COptionsMap::ReadValue(const cdstring& key, CFavouriteItemList& value, NumVersion vers_prefs)
{
	cdstring sexpression;
	bool result = ReadKeyValue(key, sexpression);

	if (result)
	{
		// Parse S-Expression list
		char_stream p(sexpression.c_str_mod());
		result = value.SetInfo(p, vers_prefs);
	}

	return result;
}

#pragma mark ____________________________CSearchStyleList
// Write key & CSearchStyleList
bool COptionsMap::WriteValue(const cdstring& key, const CSearchStyleList& value)
{
	// Use S_expression lists
	cdstring sexpression = value.GetInfo();
	return WriteKeyValue(key, sexpression);
}

// Read CSearchStyleList from key
bool COptionsMap::ReadValue(const cdstring& key, CSearchStyleList& value, NumVersion vers_prefs)
{
	cdstring sexpression;
	bool result = ReadKeyValue(key, sexpression);

	if (result)
	{
		// Parse S-Expression list
		char_stream p(sexpression.c_str_mod());
		result = value.SetInfo(p, vers_prefs);
	}

	return result;
}

#pragma mark ____________________________CIdentityList
// Write key & CIdentityList
bool COptionsMap::WriteValue(const cdstring& key, const CIdentityList& value)
{
	// Use S_expression lists
	cdstring sexpression = value.GetInfo();
	return WriteKeyValue(key, sexpression);
}

// Read CIdentityList from key
bool COptionsMap::ReadValue(const cdstring& key, CIdentityList& value, NumVersion vers_prefs)
{
	cdstring sexpression;
	bool result = ReadKeyValue(key, sexpression);

	if (result)
	{
		// Parse S-Expression list
		char_stream p(sexpression.c_str_mod());
		value.SetInfo(p, vers_prefs);
	}

	return result;
}

// Write key & CTiedIdentityList
bool COptionsMap::WriteValue(const cdstring& key, const CTiedIdentityList& value)
{
	// Use S_expression lists
	cdstring sexpression = value.GetInfo();
	return WriteKeyValue(key, sexpression);
}

// Read CTiedIdentityList from key
bool COptionsMap::ReadValue(const cdstring& key, CTiedIdentityList& value, NumVersion vers_prefs)
{
	cdstring sexpression;
	bool result = ReadKeyValue(key, sexpression);

	if (result)
	{
		// Parse S-Expression list
		char_stream p(sexpression.c_str_mod());
		value.SetInfo(p, vers_prefs);
	}

	return result;
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x

#pragma mark ____________________________TextTraits
// Write key & TextTraits
bool COptionsMap::WriteValue(const cdstring& key, const STextTraitsRecord& value)
{
	char txt[512];
	cdstring temp(value.traits.fontName);
	::snprintf(txt, 512, "%d, %s", value.traits.size, temp.c_str());
	return WriteKeyValue(key, txt);
}

bool COptionsMap::ReadValue(const cdstring& key, STextTraitsRecord& value, NumVersion vers_prefs)
{
	cdstring str_value;
	bool result = ReadKeyValue(key, str_value);
	if (result)
	{
		int size = 10;
		::sscanf(str_value, "%d, %[^,]", &size, &value.traits.fontName);
		value.traits.size = size;
		c2pstr((char*) value.traits.fontName);

		// Init others
		value.traits.style = normal;
		value.traits.justification = teFlushLeft;
		value.traits.mode = srcOr;
		value.traits.color.red = 0;
		value.traits.color.green = 0;
		value.traits.color.blue = 0;
		value.traits.fontNumber = UTextTraits::fontNumber_Unknown;
	}
	return result;
}

#elif __dest_os == __win32_os

#pragma mark ____________________________LOGFONT
// Write key & LOGFONT
bool COptionsMap::WriteValue(const cdstring& key, const SLogFont& value)
{
	char txt[512];
	::snprintf(txt, 512, "%d, %s", -value.logfont.lfHeight, cdstring(value.logfont.lfFaceName));
	return WriteKeyValue(key, txt);
}

// Read key & LOGFONT
bool COptionsMap::ReadValue(const cdstring& key, SLogFont& value, NumVersion vers_prefs)
{
	cdstring str_value;
	bool result = ReadKeyValue(key, str_value);
	if (result)
	{
		// Count items
		char* p = str_value;
		int count = 0;
		while(*p)
		{
			if (*p++ == ',') count++;
		}

		// Check for new style format of prefs
		if (count == 1)
		{
			LOGFONTA _temp;
			_temp.lfHeight = 0;
			_temp.lfWidth = 0;
			_temp.lfEscapement = 0;
			_temp.lfOrientation = 0;
			_temp.lfWeight = 400;
			_temp.lfItalic = 0;
			_temp.lfUnderline = 0;
			_temp.lfStrikeOut = 0;
			_temp.lfCharSet = 0;
			_temp.lfOutPrecision = 1;
			_temp.lfClipPrecision = 2;
			_temp.lfQuality = 1;
			_temp.lfPitchAndFamily = 0;
			*_temp.lfFaceName = 0;

			LOGFONT temp;
			temp.lfHeight = 0;
			temp.lfWidth = 0;
			temp.lfEscapement = 0;
			temp.lfOrientation = 0;
			temp.lfWeight = 400;
			temp.lfItalic = 0;
			temp.lfUnderline = 0;
			temp.lfStrikeOut = 0;
			temp.lfCharSet = 0;
			temp.lfOutPrecision = 1;
			temp.lfClipPrecision = 2;
			temp.lfQuality = 1;
			temp.lfPitchAndFamily = 0;
			*temp.lfFaceName = 0;

			::sscanf(str_value, "%d, %[^,]",
						&temp.lfHeight, &_temp.lfFaceName);

#ifdef __MULBERRY
			// Look for font in global list and match characteristics
			if (CFontPopup::GetInfo(_temp.lfFaceName, &temp.lfCharSet, &temp.lfPitchAndFamily))
#endif
			{
				::lstrcpy(temp.lfFaceName, cdstring(_temp.lfFaceName).win_str());
				temp.lfHeight *= -1;
				value.logfont = temp;
			}
		}
	}
	return result;
}

#elif __dest_os == __linux_os
bool COptionsMap::WriteValue(const cdstring& key, const SFontInfo& value)
{
	char txt[512];
	::snprintf(txt, 512, "%ld, %s", value.size, value.fontname.c_str());
	return WriteKeyValue(key, txt);
}

bool COptionsMap::ReadValue(const cdstring& key, SFontInfo& value, NumVersion vers_prefs)
{
	cdstring str_value;
	bool result = ReadKeyValue(key, str_value);
	if (result)
	{
		char fontname[512];
		sscanf(str_value, "%ld, %[^,]", &value.size, &fontname);
		value.fontname = fontname;
	}
	return result;
}

#else
#error __dest_os
#endif

#ifdef __use_speech
#pragma mark ____________________________CMessageSpeakVector
// Write key & CMessageSpeakVector
bool COptionsMap::WriteValue(const cdstring& key, const CMessageSpeakVector& value)
{
	cdstring all;
	bool first = true;
	for(CMessageSpeakVector::const_iterator iter = value.begin(); iter != value.end(); iter++)
	{
		// Add to total
		if (!first)
			all += cValueSpace;
		else
			first = false;

		all += cdstring((long) (*iter).mItem);
		all += cValueSpace;
		
		// Convert charset/line-end
		cdstring temp((*iter).mItemText);
		temp.ConvertFromOS();
		all += temp;
	}
	return WriteKeyValue(key, all);
}

bool COptionsMap::ReadValue(const cdstring& key, CMessageSpeakVector& value, NumVersion vers_prefs)
{
	cdstring all;
	bool result = ReadKeyValue(key, all);

	if (result)
	{
		// Clear existing
		value.clear();

		// Tokenise on commas
		char* p = ::strtok(all, ",");
		while(p)
		{
			CMessageSpeak item;

			// Get item type
			item.mItem = (EMessageSpeakItem) ::atoi(p);

			// Get next token and step over leading space
			p = ::strtok(nil, ",");
			if (p && (*p == ' ')) p++;

			// Get string and convert charset/line-end
			item.mItemText = p;
			item.mItemText.ConvertToOS();

			// Add to array
			value.push_back(item);

			// Get next token and step over leading space
			p = ::strtok(nil, ",");
			if (p && (*p == ' ')) p++;
		}
	}

	return result;
}
#endif

#pragma mark ____________________________CMIMEMap
// Write key & CMIMEMap
bool COptionsMap::WriteValue(const cdstring& key, const CMIMEMapVector& value)
{
	// Use S_expression lists
	cdstring all = '(';
	for(CMIMEMapVector::const_iterator iter = value.begin(); iter != value.end(); iter++)
	{
		all += '(';
		all += (*iter).GetInfo();
		all += ')';
	}
	all += ')';
	return WriteKeyValue(key, all);
}

// Read key & array of strings
bool COptionsMap::ReadValue(const cdstring& key, CMIMEMapVector& value, NumVersion vers_prefs)
{
	cdstring all;
	bool result = ReadKeyValue(key, all);

	if (result)
	{
		// Clear existing
		value.clear();

		// Parse S-Expression list
		char_stream txt(all.c_str_mod());

		if (txt.start_sexpression())
		{
			while(txt.start_sexpression())
			{
				// Create address account
				CMIMEMap map;

				// Parse into account and add to list
				map.SetInfo(txt);
				value.push_back(map);

				txt.end_sexpression();
			}

			txt.end_sexpression();
		}
	}

	return result;
}

#pragma mark ____________________________SACLStyleList
// Write key & SACLStyleList
bool COptionsMap::WriteValue(const cdstring& key, const SACLStyleList& value)
{
	cdstring all;
	bool first = true;
	for(SACLStyleList::const_iterator iter = value.begin(); iter != value.end(); iter++)
	{
		if (!first)
			all += cValueSpace;
		else
			first = false;

		// Write style name, converting charset/line-ends
		cdstring temp((*iter).first);
		temp.ConvertFromOS();
		all += temp;
		all += cValueSpace;

		if (key == cMboxACLStyleListKey)
		{
			// Create dummy ACL and parse into string
			CMboxACL acl;
			acl.SetRights((*iter).second);
			all += acl.GetTextRights();
		}
		else
		{
			// Create dummy ACL and parse into string
			CAdbkACL acl;
			acl.SetRights((*iter).second);
			all += acl.GetTextRights();
		}
	}

	return WriteKeyValue(key, all);
}

// Read key & SACLStyleList
bool COptionsMap::ReadValue(const cdstring& key, SACLStyleList& value, NumVersion vers_prefs)
{
	cdstring all;
	bool result = ReadKeyValue(key, all);

	if (result)
	{
		// Clear existing
		value.clear();

		// Tokenise on commas
		char* p = ::strtok(all, ",");
		while(p)
		{
			// Get Style name and convert charset/line-ends
			cdstring aStr = p;
			aStr.ConvertToOS();

			// Get next token and step over leading space
			p = ::strtok(nil, ",");
			if (p && (*p == ' ')) p++;

			// Get style ACL
			cdstring txt_acl = p;

			if (key == cMboxACLStyleListKey)
			{
				// Create dummy ACL for parse
				CMboxACL acl;
				acl.SetRights(txt_acl);

				// Add to array
				value.push_back(std::make_pair(aStr, acl.GetRights()));
			}
			else
			{
				// Create dummy ACL for parse
				CAdbkACL acl;
				acl.SetRights(txt_acl);

				// Add to array
				value.push_back(std::make_pair(aStr, acl.GetRights()));
			}

			// Get next token and step over leading space
			p = ::strtok(nil, ",");
			if (p && (*p == ' ')) p++;
		}
	}

	return result;
}

#pragma mark ____________________________CWindowState
// Write key & CWindowStateArray
bool COptionsMap::WriteValue(const cdstring& key, const CWindowState& value)
{
	// Use S_expression lists
	cdstring sexpression = value.GetInfo();
	return WriteKeyValue(key, sexpression);
}

bool COptionsMap::ReadValue(const cdstring& key, CWindowState& value, NumVersion vers_prefs)
{
	cdstring all;
	bool result = ReadKeyValue(key, all);

	if (result)
	{
		char_stream stream(all.c_str_mod());
		result = value.SetInfo(stream, vers_prefs);
	}
	return result;
}

#pragma mark ____________________________CWindowStateArray
// Write key & CWindowStateArray
bool COptionsMap::WriteValue(const cdstring& key, const CWindowStateArray& value)
{
	// Use S_expression lists
	cdstring all = '(';
	for(CWindowStateArray::const_iterator iter = value.begin(); iter != value.end(); iter++)
	{
		all += '(';
		all += (*iter)->GetInfo();
		all += ')';
	}
	all += ')';
	return WriteKeyValue(key, all);
}

bool COptionsMap::ReadValue(const cdstring& key, CWindowStateArray& value, NumVersion vers_prefs)
{
	cdstring all;
	bool result = ReadKeyValue(key, all);

	if (result)
	{
		// Clear existing - must delete each item
		for(CWindowStateArray::iterator iter = value.begin(); iter != value.end(); iter++)
			delete *iter;
		value.clear();

		// Parse S-Expression list
		char* txt = all;
		char* p = ::strgetbrastr(&txt);
		txt = p;

		while(p && *p)
		{
			p = ::strgetbrastr(&txt);
			if (p)
			{
				// Create new info
				CWindowState* state = nil;
				if ((key == cServerInfoSection_1_3) || (key == cServerInfoSection_1_4) || (key == cServerInfoSection_2_0))
					state = new CServerWindowState;
				else if ((key == cMailboxInfoSection_1_3) || (key == cMailboxInfoSection_1_4) || (key == cMailboxInfoSection_2_0))
					state = new CMailboxWindowState;
				else if ((key == cAddressBookInfoSection_1_0) || (key == cAddressBookInfoSection_2_0))
					state = new CAddressBookWindowState;
				else if (key == cAddressBookInfoSection_2_1)
					state = new CNewAddressBookWindowState;
				else if (key == cAdbkManagerInfoSection_1_3)
					state = new CAdbkManagerWindowState;
				else if (key == cAdbkSearchInfoSection_1_3)
					state = new CAdbkSearchWindowState;
				else if (key == cCalendarInfoSection_4_0)
					state = new CCalendarWindowState;

				if (state)
				{
					char_stream stream(p);
					bool temp_result = state->SetInfo(stream, vers_prefs);
					result = result && temp_result;
					value.push_back(state);
				}
			}
		}

	}

	return result;
}

#pragma mark ____________________________CConnectionManager::CConnectOptions
// Write key & CConnectionManager::CConnectOptions
bool COptionsMap::WriteValue(const cdstring& key, const CConnectionManager::CConnectOptions& value)
{
	// Use S_expression lists
	cdstring sexpression = value.GetInfo();
	return WriteKeyValue(key, sexpression);
}

// Read CConnectionManager::CConnectOptions from key
bool COptionsMap::ReadValue(const cdstring& key, CConnectionManager::CConnectOptions& value, NumVersion vers_prefs)
{
	cdstring sexpression;
	bool result = ReadKeyValue(key, sexpression);

	if (result)
	{
		// Parse S-Expression list
		char_stream p(sexpression.c_str_mod());
		result = value.SetInfo(p, vers_prefs);
	}

	return result;
}

#pragma mark ____________________________CConnectionManager::CDisconnectOptions
// Write key & CConnectionManager::CDisconnectOptions
bool COptionsMap::WriteValue(const cdstring& key, const CConnectionManager::CDisconnectOptions& value)
{
	// Use S_expression lists
	cdstring sexpression = value.GetInfo();
	return WriteKeyValue(key, sexpression);
}

// Read CConnectionManager::CDisconnectOptions from key
bool COptionsMap::ReadValue(const cdstring& key, CConnectionManager::CDisconnectOptions& value, NumVersion vers_prefs)
{
	cdstring sexpression;
	bool result = ReadKeyValue(key, sexpression);

	if (result)
	{
		// Parse S-Expression list
		char_stream p(sexpression.c_str_mod());
		result = value.SetInfo(p, vers_prefs);
	}

	return result;
}

#pragma mark ____________________________Sections

// Add sub-section
void COptionsMap::PushSection(const cdstring& sub_section)
{
	// Form key from two
	if (!sub_section.empty())
	{
		if (!mSection.empty())
			mSection += mSeparator;
		mSection += sub_section;

		mSectionStack.push_back(sub_section);
	}
}

// Remove last sub-section
void COptionsMap::PopSection(void)
{
	// Pop section off stack
	mSectionStack.pop_back();

	// Re-form section string
	mSection = cdstring::null_str;
	for(cdstrvect::const_iterator iter = mSectionStack.begin(); iter != mSectionStack.end(); iter++)
	{
		if (!mSection.empty())
			mSection += mSeparator;
		mSection += *iter;
	}
}
