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


// Source for CUserAction class

#include "CUserAction.h"

#include "char_stream.h"
#include "CStringUtils.h"
#include "CURL.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

#pragma mark ____________________________CUserAction

CUserAction::CUserAction()
{
	mSelection = false;
	mSingleClick = false;
	mDoubleClick = false;

	mKey = 0;
}

void CUserAction::_copy(const CUserAction& copy)
{
	mSelection = copy.mSelection;

	mSingleClick = copy.mSingleClick;
	mClickModifiers = copy.mClickModifiers;

	mDoubleClick = copy.mDoubleClick;
	mDoubleClickModifiers = copy.mDoubleClickModifiers;

	mKey = copy.mKey;
}

cdstring CUserAction::GetKeyDescriptor(unsigned char key, const CKeyModifiers& mods)
{
	cdstring keyname;
	
	// Get mods descriptor
	keyname += CKeyModifiers::GetModifiersDescriptor(mods);

	// alnum goes as-is
	if (isalnum(key))
		keyname += (char)key;
	else
		switch(key)
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
			keyname += (char)key;
			break;
		}
	
	return keyname;
}

cdstring CUserAction::GetInfo() const
{
	cdstring details;

	details += mSelection ? cValueBoolTrue : cValueBoolFalse;
	details += cSpace;

	details += mSingleClick ? cValueBoolTrue : cValueBoolFalse;
	details += cSpace;
	details += mClickModifiers.GetInfo();
	details += cSpace;

	details += mDoubleClick ? cValueBoolTrue : cValueBoolFalse;
	details += cSpace;
	details += mDoubleClickModifiers.GetInfo();
	details += cSpace;

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

bool CUserAction::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	txt.get(mSelection);

	txt.get(mSingleClick);
	mClickModifiers.SetInfo(txt, vers_prefs);

	txt.get(mDoubleClick);
	mDoubleClickModifiers.SetInfo(txt, vers_prefs);

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
