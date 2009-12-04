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


// Common header for classes

#ifndef __CMULBERRYCOMMON__MULBERRY__
#define __CMULBERRYCOMMON__MULBERRY__


// General constants
const	SInt16	kMinMailboxWidth = 460;

const OSType kApplID = 'Mlby';

// DITL items
#define kOK_Btn						1
#define kCancel_Btn					2
#define kOutlineItem				3

// DLOG's
const	ResIDT	DLOG_Get_String = 2000;
enum {
	item_GetStringItem = 5
};

// STR#'s

//const ResIDT	STRx_Standards = 200;
enum {
	str_Delete = 6,
	str_Undelete,
	str_Enable,
	str_Disable,
	str_CopyTo,
	str_MoveTo,
	str_CopyToMailbox,
	str_MoveToMailbox,
	str_CopyNow,
	str_MoveNow,
	str_ShowHeader,
	str_HideHeader,
	str_ExpandHeader,
	str_CollapseHeader,
	str_ShowParts,
	str_HideParts,
	str_ShowQuickSearch,
	str_HideQuickSearch,
	str_ShowList,
	str_HideList,
	str_ShowItems,
	str_HideItems,
	str_ShowPreview,
	str_HidePreview,
	str_ShowToolbar,
	str_HideToolbar,
	str_ExportAddressBookAs,
	str_Logon,
	str_Logoff,
	str_Disconnect,
	str_Connect,
	str_MailboxNone,
	str_MailboxAllowChoice,
	str_MailboxChooseLater,
	str_TooManyForMenu,
	str_PromptColorPick,
	str_PreferencesDialogTitle,
	str_PreferencesDialogRemoteTitle,
	str_PreferencesDialogNoFile,
	str_PreferencesDialogDefaultFile,
	xxstr_ServerPropDialogTitle,
	xxstr_MailboxPropDialogTitle,
	xxstr_MailboxPropDialogTitleMultiple,
	xxstr_AdbkPropDialogTitle,
	xxstr_AdbkPropDialogTitleMultiple,
	str_LoginIMAP,
	str_LoginPOP3,
	str_LoginSMTP,
	str_LoginRemotePrefs,
	str_LoginRemoteAddress,
	str_LoginLDAP,
	str_LoginManageSIEVE,
	str_LoginRemoteCalendar,
	str_ChangePswdIMAP,
	str_ChangePswdPOP3,
	str_ChangePswdSMTP,
	str_ChangePswdRemotePrefs,
	str_ChangePswdRemoteAddress,
	str_ChangePswdLDAP,
	str_ChangePswdManageSIEVE,
	xxstr_Local,
	xxstr_Others,
	xxstr_AddressBooks,
	xxstr_OSAddressBooks,
	_str_LoggingInsecure,
	_str_ReallyQuitSMTPHeldItems,
	_str_ReallyDisconnectSMTPHeldItems,
	_str_ViewAttachmentsStillOpen,
	str_ImportText,
	str_ImportRules,
	str_ExportRules,
	str_SelectAll,
	str_UnselectAll,
	str_ImportCalendar,
	str_ExportCalendar
};

// User configurable'STR 's
const	ResIDT	STR_DefaultPrefsFileName = 256;

// crsr's
const	ResIDT	crsr_StdSpin = 128;
const	ResIDT	crsr_DelaySpin = 129;

// icons

// Macros
#define DISPOSE_HANDLE(ahdl)		if (ahdl) {::DisposeHandle((Handle) (ahdl)); (ahdl) = nil;}
#define DISPOSE_PTR(aptr)			if (aptr) {::DisposePtr((Ptr) (aptr)); (aptr) = nil;}

// Drag type and data
const FlavorType cDragServerType = 'SerL';
const FlavorType cDragWDType = 'WD_L';
const FlavorType cDragMboxType = 'MbxL';
const FlavorType cDragMboxRefType = 'MbrL';
const FlavorType cDragMsgType = 'MsgL';
const FlavorType cDragAddrType = 'Addr';
const FlavorType cDragAdbkAddrTextType = 'AbAd';
const FlavorType cDragAddrGrpType = 'GrpA';
const FlavorType cDragMsgAtchType = 'Mtch';
const FlavorType cDragAtchType = 'Atch';
const FlavorType cDragRulePosType = 'RulP';
const FlavorType cDragRuleType = 'Rule';
const FlavorType cDragCalServerType = 'SerC';
const FlavorType cDragCalendarType = 'CalL';
const FlavorType cDragCalendarItemType = 'CalI';

// Useful routines

class LPopupButton;

short SafeTEInsert(const void* text,
					long length,
					TEHandle hTE);				// Do safe insert of text - only allow max. 32767 chars

enum EDrawStringAlignment
{
	eDrawString_Left,
	eDrawString_Center,
	eDrawString_Right,
	eDrawStringJustify
};

enum EDrawStringClip
{
	eClipString_Left,
	eClipString_Center,
	eClipString_Right
};

void DrawClippedString(const unsigned char* theTxt, short width,
						EDrawStringAlignment align, EDrawStringClip clip = eClipString_Right);	// Draw clipped string if too long

void DrawClippedStringUTF8(const char* theTxt, short width,
						EDrawStringAlignment align, EDrawStringClip clip = eClipString_Right);	// Draw clipped string if too long
void DrawClippedStringUTF8(CGContextRef inContext, const char* theTxt, Rect area,
						EDrawStringAlignment align, EDrawStringClip clip = eClipString_Right);	// Draw clipped string if too long

void AppendItemToMenu(MenuHandle menuH, short pos, const char* text, bool bold = false, ResIDT icon = 0);
void SetPopupByName(LPopupButton* popup, const char* name);

void SetMenuItemTextUTF8(MenuRef menu, MenuItemIndex index, const cdstring& txt);
cdstring GetMenuItemTextUTF8(MenuRef menu, MenuItemIndex index);
cdstring GetPopupMenuItemTextUTF8(LPopupButton* popup);

void DisableItem(MenuRef theMenu, short item);
void EnableItem(MenuRef theMenu, short item);

// Carbon preference access
bool GetCFPreference(const char* key, cdstring& value);
bool SetCFPreference(const char* key, const char* value);

void RectOnScreen(Rect& rect, LWindow* wnd);					// Make sure some portion of title bar is on screen
IconSuiteRef Geticns(SInt16 theResID);
void Ploticns(const Rect* theRect, IconAlignmentType align, IconTransformType transform, SInt16 theResID);

void PlayNamedSound(const char* name);

cdstring GetNumericFormat(unsigned long number);

// Drag & Drop support
OSErr GetDropDirectory(DragReference dragRef, FSRef* fsrOut);

// Stop drawing until done
class StNoRedraw
{
public:
					StNoRedraw(LPane* inPane);
					~StNoRedraw();
private:
	LPane*			mPane;
	ETriState		mSaveState;
};

#endif
