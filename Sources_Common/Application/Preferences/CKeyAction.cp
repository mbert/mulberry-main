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


// Source for CKeyAction class

#include "CKeyAction.h"

#include "char_stream.h"
#include "CStringUtils.h"
#include "CURL.h"

#if __dest_os == __linux_os
#include "JXKeyModifiers.h"
#endif

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

#pragma mark ____________________________CKeyModifiers

// Default constructor
CKeyModifiers::CKeyModifiers()
{
	for(int i = 0; i < eNumModifiers; i++)
		mMods[i] = false;
}

CKeyModifiers::CKeyModifiers(int mods)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mMods[eShift] = (mods & shiftKey);
	mMods[eAlt] = (mods & optionKey);
	mMods[eCmd] = (mods & cmdKey);
	mMods[eControl] = (mods & controlKey);
#elif __dest_os == __win32_os
	mMods[eShift] = (::GetKeyState(VK_SHIFT) < 0);
	mMods[eAlt] = (::GetKeyState(VK_MENU) < 0);
	mMods[eCmd] = false;
	mMods[eControl] = (::GetKeyState(VK_CONTROL) < 0);
#elif __dest_os == __linux_os
#endif
}

#if __dest_os == __linux_os
CKeyModifiers::CKeyModifiers(const JXKeyModifiers& mods)
{
	mMods[eShift] = mods.shift();
	mMods[eAlt] = mods.alt();
	mMods[eCmd] = mods.meta();
	mMods[eControl] = mods.control();
}
#endif

void CKeyModifiers::_copy(const CKeyModifiers& copy)
{
	for(int i = 0; i < eNumModifiers; i++)
		mMods[i] = copy.mMods[i];
}

// Compare with same type
int CKeyModifiers::operator==(const CKeyModifiers& comp) const
{
	for(int i = 0; i < eNumModifiers; i++)
	{
		if (mMods[i] != comp.mMods[i])
			return false;
	}

	return true;
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
const char* cModifierNames[] = {"Shift", "Option", "Command", "Control", NULL};
#elif __dest_os == __win32_os
const char* cModifierNames[] = {"Shift", "Alt", "", "Control", NULL};
#elif __dest_os == __linux_os
const char* cModifierNames[] = {"Shift", "Alt", "Meta", "Control", NULL};
#endif

cdstring CKeyModifiers::GetModifiersDescriptor(const CKeyModifiers& mods)
{
	cdstring modname;
	for(int i = 0; i < eNumModifiers; i++)
	{
		if (mods.mMods[i])
		{
			modname += cModifierNames[i];
			modname += "+";
		}
	}
	
	return modname;
}

const char* cModifierDescriptors[] = {"shift", "alt", "command", "control", NULL};

cdstring CKeyModifiers::GetInfo() const
{
	// Add each modifier to array if turned on
	cdstrvect list;
	for(int i = 0; i < eNumModifiers; i++)
	{
		if (mMods[i])
			list.push_back(cModifierDescriptors[i]);
	}
	
	// Convert array to s-expression
	cdstring details;
	details.CreateSExpression(list);
	
	return details;
}

bool CKeyModifiers::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Get items
	cdstrvect list;
	cdstring::ParseSExpression(txt, list, false);
	
	// Clear current set
	for(int i = 0; i < eNumModifiers; i++)
		mMods[i] = false;

	// Set each modifer if descriptor is found
	for(cdstrvect::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		unsigned long index = ::strindexfind(*iter, cModifierDescriptors, eNumModifiers);
		if (index < eNumModifiers)
			mMods[index] = true;
	}
	
	return true;
}

#pragma mark ____________________________CKeyAction

CKeyAction::CKeyAction()
{
	mKey = 0;
}

CKeyAction::CKeyAction(unsigned char key, const CKeyModifiers& mods)
{
	mKey = key;
	mKeyModifiers = mods;
}

void CKeyAction::_copy(const CKeyAction& copy)
{
	mKey = copy.mKey;
	mKeyModifiers = copy.mKeyModifiers;
}

cdstring CKeyAction::GetKeyDescriptor(const CKeyAction& key)
{
	cdstring keyname;
	
	// Get mods descriptor
	keyname += CKeyModifiers::GetModifiersDescriptor(key.mKeyModifiers);

	// alnum goes as-is
	if (isalnum(key.mKey))
		keyname += (char)key.mKey;
	else
		switch(key.mKey)
		{
		case 0:
			return cdstring::null_str;

		case '\t':
			keyname += "tab";
			break;

		case '\r':
			keyname += "return";
			break;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		case kEscapeCharCode:
#elif __dest_os == __win32_os
		case VK_ESCAPE:
#elif __dest_os == __linux_os
#endif
			keyname += "escape";
			break;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		case kLeftArrowCharCode:
#elif __dest_os == __win32_os
		case VK_LEFT:
#elif __dest_os == __linux_os
#endif
			keyname += "left-arrow";
			break;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		case kRightArrowCharCode:
#elif __dest_os == __win32_os
		case VK_RIGHT:
#elif __dest_os == __linux_os
#endif
			keyname += "right-arrow";
			break;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		case kUpArrowCharCode:
#elif __dest_os == __win32_os
		case VK_UP:
#elif __dest_os == __linux_os
#endif
			keyname += "up-arrow";
			break;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		case kDownArrowCharCode:
#elif __dest_os == __win32_os
		case VK_DOWN:
#elif __dest_os == __linux_os
#endif
			keyname += "down-arrow";
			break;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		case kHomeCharCode:
#elif __dest_os == __win32_os
		case VK_HOME:
#elif __dest_os == __linux_os
#endif
			keyname += "home";
			break;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		case kEndCharCode:
#elif __dest_os == __win32_os
		case VK_END:
#elif __dest_os == __linux_os
#endif
			keyname += "end";
			break;

		case ' ':
			keyname += "space";
			break;
		default:
			keyname += (char)key.mKey;
			break;
		}
	
	return keyname;
}

cdstring CKeyAction::GetInfo() const
{
	cdstring details;

	// Use "%xx" encoding for character
	details += "\"";
	details += cURLEscape;
	details += cHexChar[mKey >> 4];
	details += cHexChar[mKey & 0x0F];
	details += "\"";
	details += cSpace;
	details += mKeyModifiers.GetInfo();

	return details;
}

bool CKeyAction::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Decode %xx encoding
	cdstring temp;
	txt.get(temp);
	const char* p = temp.c_str();
	mKey = 0;
	if (*p++ == '%')
	{
		mKey = (unsigned char) (cFromHex[(unsigned char) *p++] << 4);
		mKey |= (unsigned char) cFromHex[(unsigned char) *p++];
	}

	mKeyModifiers.SetInfo(txt, vers_prefs);
	
	return true;
}
