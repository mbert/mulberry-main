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


// Common source for CMailbox classes

#include "CMulberryCommon.h"

#include "CPreferences.h"
#include "CSoundManager.h"

#include <stdio.h>
#include <TextUtils.h>

#include <LPopupButton.h>

#include <LCFString.h>
#include "MyCFString.h"
#include <CFPreferences.h>

// Do safe insert of text - only allow max. 32767 chars
short SafeTEInsert(const void* text, long length, TEHandle hTE)
{
	const int cTEMaxLimit = 30000;

	SInt16 te_size = (**hTE).teLength;
	SInt16 te_incr = (length + te_size < cTEMaxLimit) ? (length) : (cTEMaxLimit - te_size);
	::TEInsert(text, te_incr, hTE);

	return te_incr;
}

// Draw clipped string if too long
void DrawClippedString(const unsigned char* theTxt, short width, EDrawStringAlignment align, EDrawStringClip clip)
{
	// Always have 4 pixel border on both sides
	width -= 8;
	::Move(4, 0);

	if ((CPreferences::sPrefs != NULL) && true /*CPreferences::sPrefs->mAntiAliasFont.GetValue()*/)
	{
		// Get current pen position
		Point pen;
		::GetPen(&pen);

		// Map alignment to Theme value
		ThemeDrawState	drawState = kThemeStateActive;
		SInt16 alignment;
		switch(align)
		{
		case eDrawString_Left:
		case eDrawStringJustify:
			alignment = teJustLeft;
			break;
		case eDrawString_Center:
			alignment = teJustCenter;
			break;
		case eDrawString_Right:
			alignment = teJustRight;
			break;
		}
		
		// Create CFString of text
		CFMutableStringRef cfstring = ::CFStringCreateMutable(NULL, ::PLstrlen(theTxt));
		if (cfstring == NULL)
			return;

		::CFStringAppendPascalString(cfstring, theTxt, kCFStringEncodingMacRoman);
		
		// Get height of text being drawn
		Point ioBounds;
		SInt16 outBaseline;
		::GetThemeTextDimensions(cfstring, kThemeCurrentPortFont, drawState, false, &ioBounds, &outBaseline);

		// Truncate text using supplied clipping option
		Boolean outTruncated;
		::TruncateThemeText(cfstring, kThemeCurrentPortFont, drawState, width, clip == eClipString_Center ? truncMiddle : truncEnd, &outTruncated); 

		// Determine rect t draw into
		Rect frame;
		frame.left = pen.h;
		frame.right = frame.left + width;
		frame.top = pen.v - ioBounds.v - outBaseline;
		frame.bottom = frame.top + ioBounds.v;
		
		// Draw themed text
		::DrawThemeTextBox(cfstring, kThemeCurrentPortFont, drawState, false, &frame, alignment, NULL);

		::CFRelease(cfstring);
	}
	else
	{
		// Check actual width against allowed width
		short str_width = ::StringWidth(theTxt);
		if ((str_width < width) || (*theTxt < 2))
		{
			switch(align)
			{
			case eDrawString_Left:
			case eDrawStringJustify:
				// Already in right place
				break;
			case eDrawString_Center:
				// Move to center it
				::Move((width + 1 - str_width)/2, 0);
				break;
			case eDrawString_Right:
				// Move to right it
				::Move(width - str_width, 0);
				break;
			}

			// String will fit - draw unmodified
			::DrawString(theTxt);
		}
		else
		{
			// Copy string since it will be modified
			Str255 str;
			::PLstrcpy(str, theTxt);

			// Reduce length of string and recalculate width
			for(str[0]--; str[0] > 0; str[0]--)
			{
				switch(clip)
				{
				case eClipString_Left:
					// Move string one char to left
					::memmove(&str[1], &str[2], str[0]);
					break;
				
				case eClipString_Right:
					// Move string one char to right - i.e. do nothing
					break;

				case eClipString_Center:
					// Move string one char to left at its center
					::memmove(&str[(str[0] + 1)/2 + 1], &str[(str[0] + 1)/2 + 2], str[0] - str[0]/2);
					break;
				}

				if (::StringWidth(str) < width)
					break;
			}

			switch(clip)
			{
			case eClipString_Left:
				// First char must indicate clipping
				str[1] = 'É';
				break;
			
			case eClipString_Right:
				// Last char must indicate clipping
				str[str[0]] = 'É';
				break;

			case eClipString_Center:
				// Center char must indicate clipping
				str[str[0]/2 + 1] = 'É';
				break;
			}

			// Draw clipped string
			::DrawString(str);
		}
	}
}

// Draw clipped string if too long
void DrawClippedStringUTF8(const char* theTxt, short width, EDrawStringAlignment align, EDrawStringClip clip)
{
	// Get current pen position
	Point pen;
	::GetPen(&pen);

	Rect area = { pen.v - 16, pen.h, pen.v, pen.h + width };
	DrawClippedStringUTF8(NULL, theTxt, area, align, clip);
}

// Draw clipped string if too long
void DrawClippedStringUTF8(CGContextRef inContext, const char* theTxt, Rect area, EDrawStringAlignment align, EDrawStringClip clip)
{
	// Always have 4 pixel border on both sides
	area.left += 4;
	area.right -= 4;

	// Map alignment to Theme value
	ThemeDrawState	drawState = kThemeStateActive;
	SInt16 alignment;
	switch(align)
	{
	case eDrawString_Left:
	case eDrawStringJustify:
		alignment = teJustLeft;
		break;
	case eDrawString_Center:
		alignment = teJustCenter;
		break;
	case eDrawString_Right:
		alignment = teJustRight;
		break;
	}
	
	// Create CFString of text
	MyCFString cfstring(theTxt, kCFStringEncodingUTF8);
	
	// Get height of text being drawn
	Point ioBounds;
	SInt16 outBaseline;
	::GetThemeTextDimensions(cfstring, kThemeCurrentPortFont, drawState, false, &ioBounds, &outBaseline);

	// Truncate text using supplied clipping option
	Boolean outTruncated;
	::TruncateThemeText(cfstring, kThemeCurrentPortFont, drawState, area.right - area.left, clip == eClipString_Center ? truncMiddle : truncEnd, &outTruncated); 

	// Determine rect t draw into
	Rect frame;
	frame.left = area.left;
	frame.right = area.right;
	frame.top = area.bottom - ioBounds.v - outBaseline;
	frame.bottom = frame.top + ioBounds.v;
	
	// Draw themed text
	::DrawThemeTextBox(cfstring, kThemeCurrentPortFont, drawState, false, &frame, alignment, inContext);
}

void AppendItemToMenu(MenuHandle menuH, short pos, const char* text, bool bold, ResIDT icon)
{
	// Insert item
	::AppendMenu(menuH, bold ? "\p?<B" : "\p?");
	if (::strlen(text))
	{
		::SetMenuItemTextUTF8(menuH, pos, cdstring(text));
	}
	if (icon)
		::SetItemIcon(menuH, pos, icon);
}

void SetPopupByName(LPopupButton* popup, const char* name)
{
	MenuHandle menuH = popup->GetMacMenuH();
	short numItems = ::CountMenuItems(menuH);

	for (short i = 1; i <= numItems; i++)
	{
		cdstring menuitem = ::GetMenuItemTextUTF8(menuH, i);
		if (menuitem.compare(name) == 0)
		{
			popup->SetValue(i);
			return;
		}
	}

	popup->SetValue(1);
}

void SetMenuItemTextUTF8(MenuRef menu, MenuItemIndex index, const cdstring& txt)
{
	MyCFString temp(txt, kCFStringEncodingUTF8);
	::SetMenuItemTextWithCFString(menu, index, temp);
}

cdstring GetMenuItemTextUTF8(MenuRef menu, MenuItemIndex index)
{
	if (index > 0)
	{
		CFStringRef menutxt;
		if ((::CopyMenuItemTextAsCFString(menu, index, &menutxt) == noErr) && (menutxt != NULL))
		{
			MyCFString temp(menutxt, false);
			return temp.GetString();
		}
	}

	return cdstring::null_str;
}

cdstring GetPopupMenuItemTextUTF8(LPopupButton* popup)
{
	return GetMenuItemTextUTF8(popup->GetMacMenuH(), popup->GetCurrentMenuItem());
}

void DisableItem(MenuRef theMenu, short item)
{
	::DisableMenuItem(theMenu, (MenuItemIndex) item);
}

void EnableItem(MenuRef theMenu, short item)
{
	::EnableMenuItem(theMenu, (MenuItemIndex) item);
}

bool GetCFPreference(const char* key, cdstring& value)
{
	// Read the preference.
	CFStringRef keystr = ::CFStringCreateWithCString(NULL, key, ::CFStringGetSystemEncoding());
	CFStringRef appstr = ::CFStringCreateWithCString(NULL, "Mulberry", ::CFStringGetSystemEncoding());
	CFStringRef valuestr = reinterpret_cast<CFStringRef>(::CFPreferencesCopyValue(keystr, appstr, kCFPreferencesCurrentUser, kCFPreferencesAnyHost));

	// Put into buffer
	bool result = false;
	if (valuestr)
	{
		value.reserve(256);
		result = ::CFStringGetCString(valuestr, value.c_str_mod(), 256, ::CFStringGetSystemEncoding());
	}
	
	if (keystr)
		::CFRelease(keystr);
	if (appstr)
		::CFRelease(appstr);
	if (valuestr)
		::CFRelease(valuestr);
	
	return result;
}

bool SetCFPreference(const char* key, const char* value)
{
	// Read the preference.
	CFStringRef keystr = ::CFStringCreateWithCString(NULL, key, ::CFStringGetSystemEncoding());
	CFStringRef appstr = ::CFStringCreateWithCString(NULL, "Mulberry", ::CFStringGetSystemEncoding());
	CFStringRef valuestr = (value ? ::CFStringCreateWithCString(NULL, value, ::CFStringGetSystemEncoding()) : NULL);
	::CFPreferencesSetValue(keystr, valuestr, appstr, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);

	bool result = ::CFPreferencesSynchronize(appstr, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
	
	if (keystr)
		::CFRelease(keystr);
	if (appstr)
		::CFRelease(appstr);
	if (valuestr)
		::CFRelease(valuestr);

	return result;
}

// Make sure some portion of title bar is on screen
void RectOnScreen(Rect& rect, LWindow* wnd)
{
	// Determine the offset between the content and structure regions of the window
	Rect	structRect;
	::GetWindowBounds(wnd->GetMacWindow(), kWindowStructureRgn, &structRect);
	Rect	contentRect;
	::GetWindowBounds(wnd->GetMacWindow(), kWindowContentRgn, &contentRect);
	
	Point	contentOffset;
	contentOffset.h = (SInt16) (contentRect.left - structRect.left);
	contentOffset.v = (SInt16) (contentRect.top - structRect.top);

	// Find GDevice containing largest portion of requested rect
	GDHandle	dominantDevice = UWindows::FindDominantDevice(rect);

	if (dominantDevice == NULL) {	// Window is offscreen, so use the
									//   main scren
		dominantDevice = ::GetMainDevice();
	}
	Rect screenRect = (**dominantDevice).gdRect;
	
									// Must compensate for MenuBar on the
									//   main screen
	if (dominantDevice == ::GetMainDevice()) {
		screenRect.top += ::GetMBarHeight();
	}
	
	#if TARGET_API_MAC_CARBON		// CarbonLib 1.3 or later has a call
									//   to get the available screen rect,
									//   which accounts for the menu bar
									//   and the dock
	
		if (CFM_AddressIsResolved_(GetAvailableWindowPositioningBounds)) {
		
			::GetAvailableWindowPositioningBounds(dominantDevice, &screenRect);
		}
		
	#endif

	// Adjust for content/structure offset
	Rect deskr = screenRect;
	deskr.top += contentOffset.v;
	deskr.left += contentOffset.h;

	if (!::PtInRect(topLeft(rect), &deskr))
	{
		Point diff = {0, 0};

		if (rect.left < deskr.left)
			diff.h = deskr.left - rect.left;
		else if (rect.left > deskr.right)
			diff.h = deskr.right - rect.left - 100;

		if (rect.top < deskr.top)
			diff.v = deskr.top - rect.top;
		else if (rect.top > deskr.bottom)
			diff.v = deskr.bottom - rect.top - 100;

		::OffsetRect(&rect, diff.h, diff.v);
	}
	
	// Add sanity check for window height and width
	if (rect.bottom > deskr.bottom)
		rect.bottom = deskr.bottom;
	if (rect.right > deskr.right)
		rect.right = deskr.right;
}

IconSuiteRef Geticns(SInt16 theResID)
{
	// Look for icon suite in cache
	typedef std::map<SInt16, IconSuiteRef> CIconSuiteCache;
	static CIconSuiteCache cache;
	CIconSuiteCache::const_iterator found = cache.find(theResID);
	if (found != cache.end())
	{
		// Use cached icon suite handle
		return (*found).second;
	}
	else
	{
		StResource icns('icns', theResID, false);
		if (icns.IsValid())
		{
			IconSuiteRef iref;
			OSErr err = ::IconFamilyToIconSuite((IconFamilyHandle)icns.Get(), kSelectorAllAvailableData, &iref);
			if (err == noErr)
			{
				// Cache the handle
				cache[theResID] = iref;

				return iref;
			}
		}
	}
	return NULL;
}

void Ploticns(const Rect* theRect, IconAlignmentType align, IconTransformType transform, SInt16 theResID)
{
	// Look for icon suite in cache
	IconSuiteRef iref = ::Geticns(theResID);
	if (iref != NULL)
		::PlotIconSuite(theRect, align, transform, iref);
	else
		// Fall back to plain icon suite drawing
		::PlotIconID(theRect, align, transform, theResID);
}

cdstring GetNumericFormat(unsigned long number)
{
	cdstring result;
	result.reserve(64);
	if (number >= 10000000UL)
		::snprintf(result.c_str_mod(), 64, "%d M", (number >> 20) + (number & (1L << 19) ? 1 : 0));
	else if (number >= 10000UL)
		::snprintf(result.c_str_mod(), 64, "%d K", (number >> 10) + (number & (1L << 9) ? 1 : 0));
	else
		::snprintf(result.c_str_mod(), 64, "%d", number);
	
	return result;
}

void PlayNamedSound(const char* name)
{
	// Only if named
	if (!CSoundManager::sSoundManager.PlaySound(name))
		::SysBeep(1);
}

OSErr GetDropDirectory(DragReference dragRef, FSRef* fsrOut)
{
	OSErr err = noErr;

	StAEDescriptor dropLocAlias;

	if (!(err = ::GetDropLocation(dragRef,dropLocAlias)))
	{
		if (dropLocAlias.DescriptorType() != typeAlias)
			err = paramErr;
		else
		{
			StAEDescriptor dropLocFSR;

			if (!(err = ::AECoerceDesc(dropLocAlias, typeFSRef, dropLocFSR)))
			{

				err = ::AEGetDescData(dropLocFSR, fsrOut, sizeof(FSRef));
				ThrowIfOSErr_(err);
			}
		}
	}

	return err;
}

// Need this because some versions of the InterfaceLib do not
// a have PBXGetVolInfoSync method

#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM || TARGET_API_MAC_CARBON
#else
#include <FSM.h>
#include <Traps.h>
#endif

// Define the ProcInfoType value for the PBXGetVolInfo routines.
         
enum {
    uppXGetVolInfoProcInfo = kRegisterBased |
        RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
        REGISTER_RESULT_LOCATION(kRegisterD0) |
        REGISTER_ROUTINE_PARAMETER(1, kRegisterD0, kFourByteCode) |
        REGISTER_ROUTINE_PARAMETER(2, kRegisterD1, kFourByteCode) |
        REGISTER_ROUTINE_PARAMETER(3, kRegisterA0, SIZE_CODE(sizeof(XVolumeParamPtr)))
};
         
// Glue for the sync routine.
         
extern pascal OSErr MyPBXGetVolInfoSync(XVolumeParamPtr paramBlock);
pascal OSErr MyPBXGetVolInfoSync(XVolumeParamPtr paramBlock)
{
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM || TARGET_API_MAC_CARBON
	return ::PBXGetVolInfoSync(paramBlock);
#else
    return CallOSTrapUniversalProc(GetOSTrapAddress(_FSDispatch), uppXGetVolInfoProcInfo,
        kFSMXGetVolInfo,            // selector   in D0
        _FSDispatch,                // trap word  in D1
        paramBlock                  // paramBlock in A0
    );
#endif
}

StNoRedraw::StNoRedraw(
	LPane*		inPane)
{
	mPane		= inPane;
	mSaveState	= triState_Off;

	if (inPane != NULL) {
		mSaveState  = inPane->GetVisibleState();
		inPane->SetVisibleState(triState_Off);
	}
}


StNoRedraw::~StNoRedraw()
{
	if (mPane != NULL) {
		mPane->SetVisibleState(mSaveState);
		if (mSaveState == triState_On)
			mPane->Refresh();
	}
}
