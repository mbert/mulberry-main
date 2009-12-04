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


// Header for CToolbarManager class

#ifndef __CTOOLBARMANAGER__MULBERRY__
#define __CTOOLBARMANAGER__MULBERRY__

#include "CListener.h"

#include "CToolbarItem.h"

#include "CScreenDimensions.h"

// Classes
class CToolbarManager : public CListener
{
public:
	enum EToolbarType
	{
		e3PaneMailbox,
		e3PaneContacts,
		e3PaneCalendar,
		e1PaneServer,
		e1PaneMailbox,
		e1PaneSMTP,
		e1PaneMessage,
		e1PaneLetter,
		e1PaneAdbkMgr,
		e1PaneAddressBook,
		e1PaneCalendarStore,
		e1PaneCalendar,
		eLastToolbar
	};

	enum EToolbarItem
	{
		eToolbar_Separator,
		eToolbar_Space,
		eToolbar_ExpandSpace,

		eToolbar_Draft,
		eToolbar_Details,
		eToolbar_Expunge,
		eToolbar_Search,
		eToolbar_Reply,
		eToolbar_ReplyAll,
		eToolbar_Forward,
		eToolbar_Bounce,
		eToolbar_Reject,
		eToolbar_SendAgain,
		eToolbar_Digest,
		eToolbar_CopyTo,
		eToolbar_Delete,
		eToolbar_Header,
		eToolbar_Save,
		eToolbar_Print,

		eToolbar_ShowList,
		eToolbar_ShowItems,
		eToolbar_ShowPreview,

		eToolbar_Connect,
		eToolbar_Login,
		eToolbar_Create,
		eToolbar_Check,
		eToolbar_Cabinet,

		eToolbar_Match,
		eToolbar_Select,
		eToolbar_MailboxType,
		eToolbar_ApplyRules,

		eToolbar_Hold,
		eToolbar_Priority,
		eToolbar_Accounts,
		eToolbar_Enable,

		eToolbar_Flags,
		eToolbar_DeleteNext,
		eToolbar_Previous,
		eToolbar_Next,

		eToolbar_Attach,
		eToolbar_Spell,
		eToolbar_Receipt,
		eToolbar_DSN,
		eToolbar_Signature,
		eToolbar_Sign,
		eToolbar_Encrypt,
		eToolbar_Send,
		eToolbar_AddCC,
		eToolbar_AddBcc,
		eToolbar_AppendNow,
		eToolbar_CopyOriginal,
		eToolbar_ExternalEdit,

		eToolbar_NewAdbk,
		eToolbar_OpenAdbk,
		eToolbar_DeleteAdbk,
		eToolbar_SearchAddress,

		eToolbar_NewAddress,
		eToolbar_NewGroup,
		eToolbar_EditItem,
		eToolbar_DeleteItem,
		
		eToolbar_NewCalendar,
		eToolbar_NewEvent,
		eToolbar_NewToDo,
		eToolbar_ShowToday,
		eToolbar_GoToDate,
		eToolbar_CalendarDay,
		eToolbar_CalendarWorkWeek,
		eToolbar_CalendarWeek,
		eToolbar_CalendarMonth,
		eToolbar_CalendarYear,
		eToolbar_CalendarSummary,
		eToolbar_CalendarToDo,

		eToolbar_CollapseHeader,

		eToolbar_Last
		
	};

	static CToolbarManager	sToolbarManager;

	CToolbarManager()
		{ InitToolbars(); }
	~CToolbarManager() {}

	virtual void		ListenTo_Message(long msg, void* param);	// Respond to list changes

	const CToolbarItem::CToolbarPtrItems& GetAllowedToolbarItems(EToolbarType type)
	{
		return mAllowedCommands[type];
	}

	const CToolbarItem::CToolbarPtrItems& GetToolbarItems(EToolbarType type)
	{
		return mCurrentCommands[type];
	}

	const cdstring& GetTitle(unsigned long item);
	const cdstring& GetDescriptor(unsigned long item);

	void AddItemAt(EToolbarType type, unsigned long allowed_index, unsigned long index, const cdstring& extra = cdstring::null_str);
	void MoveItemAt(EToolbarType type, unsigned long from_index, unsigned long to_index);
	void RemoveItemAt(EToolbarType type, unsigned long index);
	void ResetToolbar(EToolbarType type);

	void PrefsChanged();

	cdstring GetInfo(void) const;
	bool SetInfo(char_stream& txt, NumVersion vers_prefs);

#if __dest_os == __linux_os
	cdstring GetPopupMenuItems(unsigned long index);
#endif

private:
	CToolbarItem::CToolbarItems		mAllToolbarCommands;
	CToolbarItem::CToolbarPtrItems	mAllowedCommands[eLastToolbar];
	CToolbarItem::CToolbarPtrItems	mCurrentCommands[eLastToolbar];
	
	typedef CScreenDimensions<cdstring> CScreenToolbars;
	typedef std::vector<CScreenToolbars> CScreenToolbarList;
	
	CScreenToolbarList				mScreenToolbars;
	
	cdstring GetToolbarInfo(void) const;
	bool SetToolbarInfo(char_stream& txt);
	
	void MergeScreenToolbars();
	void LoadScreenToolbars();

	void InitToolbars();
};

#endif
