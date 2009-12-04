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


// Header for CCopyToMenu class

#ifndef __CCOPYTOMENU__MULBERRY__
#define __CCOPYTOMENU__MULBERRY__

#include "CListener.h"
#include "CBroadcaster.h"

#include "templs.h"
#include "cdstring.h"
#include "CMenu.h"

// Classes

class CMbox;
class CMboxList;
class CMboxProtocol;
class CMboxRefList;
class CTreeNodeList;

class CCopyToMenu : public CListener, public CBroadcaster
{
	friend class CMailboxPopup;
	friend class CMailboxToolbarPopup;

public:
	enum 
	{
		cCopyToChoose = 1,
		cCopyToSingleINBOX,
		cPopupCopyNone = 0,
		cPopupCopyChoose = 1,
		cPopupCopySingleINBOX,
		cPopupAppendNone = 1,
		cPopupAppendChoose,
		cPopupAppendSingleINBOX
	};

	enum
	{
		eBroadcast_ResetMenuCopy = 'crst',
		eBroadcast_ResetMenuAppend = 'arst'
	};

	// Special class for main copy to menu
	class CCopyToMain : public CMenu, public CListener
	{
	public:
		CCopyToMain(bool copyto, JXMenu* owner, const JIndex itemIndex, JXContainer* enclosure);
		virtual ~CCopyToMain() {}

		virtual void ListenTo_Message(long msg, void* param);	// Respond to list changes

		bool RequiresReset(bool copyto)
			{ return (copyto == mCopyTo) && mRequiresReset; }
		void Reset()
			{ mRequiresReset = false; }

	private:
		bool mCopyTo;
		bool mRequiresReset;
	};

	// Special class for main copy to sub-menu used for dynamic_cast identification only
	class CCopyToSub : public CMenu
	{
	public:
		CCopyToSub(JXMenu* owner, const JIndex itemIndex, JXContainer* enclosure)
			: CMenu(owner, itemIndex, enclosure) {}
		virtual ~CCopyToSub() {}
	};

	static CCopyToMenu*	sMailboxMainMenu;

	CCopyToMenu();
	virtual ~CCopyToMenu();

	virtual void ListenTo_Message(long msg, void* param);	// Respond to list changes

	static void SetMenuOptions(bool use_copyto_cabinet, bool use_appendto_cabinet);
	static void DirtyMenuList(void);
	static void DirtyCopyMenuList(void);
	static void DirtyAppendMenuList(void);
	static void ResetMenuList(JXTextMenu* copyto, JXTextMenu* appendto);

	static bool GetMbox(JXTextMenu* menu, bool copy_to, int item_num, CMbox*& found);

	static cdstring  GetPopupMenu(bool copy_to)
		{ return copy_to ? sCopyToPopupMenu : sAppendToPopupMenu; }
	static bool	GetPopupMboxSend(JXTextMenu *theMenu,bool copy_to, short item_num, CMbox*& found, bool& set_as_default);
	static bool	GetPopupMbox(JXTextMenu *theMenu,bool copy_to, short item_num, CMbox*& found, bool do_choice = true);
	static bool	GetPopupMbox(JXTextMenu *theMenu, bool copy_to, short item_num, bool do_choice, bool sending, CMbox*& found, bool& set_as_default);
	static bool   GetPopupMboxName(JXTextMenu* theMenu, bool copy_to, int item_num, cdstring& found, bool do_choice = true);
	static SInt16  FindPopupMboxPos(JXTextMenu* theMenu, bool copy_to, const char* name);

 private:
	static cdstring sCopyToPopupMenu;
	static cdstring sAppendToPopupMenu;

	static bool sCopyToReset;
	static bool sAppendToReset;
	static bool sUseCopyToCabinet;
	static bool sUseAppendToCabinet;
	static bool	sStripSubscribePrefix;
	static bool	sStripFavouritePrefix;

	static void ChangedList(CTreeNodeList* list);

	static bool	TestPrefix(const CMboxList& list, const cdstring& offset);

	static void ResetAll(JXTextMenu* main_menu, cdstring& popup_menu, bool copy_to);
	static void ResetFavourite(JXTextMenu* main_menu, 
								cdstring& popup_menu, 
								const CMboxRefList& items, 
								const CMboxRefList& mrus,
								bool copy_to);
	static void AddProtocolToMenu(CMboxProtocol* proto, 
									cdstring& menu, 
									bool is_popup,
									int& pos, 
									bool acct_name, 
									bool single = false);
	static void AppendListToMenu(const CMboxList& list, 
									cdstring &menu, 
									bool is_popup,
									int& pos, 
									bool acct_name, 
									const cdstring& noffset);
	static void AppendListToMenu(const CMboxRefList& list, 
									bool dynamic, 
									cdstring& menu, 
									bool is_popup,
									int& pos, 
									bool acct_name, 
									const cdstring& noffset, 
									bool reverse = false);
};

#endif
