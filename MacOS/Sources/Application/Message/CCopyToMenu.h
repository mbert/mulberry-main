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

#include "templs.h"
//#include "cdstring.h"

// Consts

const ResIDT	MENU_CopyTo = 154;
const ResIDT	MENU_AppendTo = 159;

// Classes

class LMenu;
class CMbox;
class CMboxList;
class CMboxProtocol;
class CMboxRefList;
class CTreeNode;
class CTreeNodeList;

class cdstring;

class CCopyToMenu : public CListener
{
	friend class CMailboxPopup;
	friend class CMailboxToolbarPopup;

private:
	typedef std::vector<LMenu*> CMenuList;

	static LMenu*		sCopyToMenu;
	static LMenu*		sAppendToMenu;
	static CMenuList	sCopyToMenuList;
	static CMenuList	sAppendToMenuList;
	static MenuHandle	sCopyToPopupMenu;
	static ulvector		sCopyToPopupServerBreaks;
	static MenuHandle	sAppendToPopupMenu;
	static ulvector		sAppendToPopupServerBreaks;
	static bool			sCopyToReset;
	static bool			sAppendToReset;
	static bool			sUseCopyToCabinet;
	static bool			sUseAppendToCabinet;
	static bool			sStripSubscribePrefix;
	static bool			sStripFavouritePrefix;

public:
	static CCopyToMenu*	sMailboxMainMenu;

						CCopyToMenu();
	virtual 			~CCopyToMenu();

	void				ListenTo_Message(long msg, void* param);	// Respond to list changes

	static void			SetMenuOptions(bool use_copyto_cabinet, bool use_appendto_cabinet);
	static void			DirtyMenuList(void)
		{ sCopyToReset = true; sAppendToReset = true; }
	static void			ResetMenuList(void);
	static bool			IsCopyToMenu(short menu_id);

	static bool			GetMbox(short menu_id, short item_num, CMbox*& found);

	static MenuHandle	GetPopupMenuHandle(bool copy_to)
		{ return copy_to ? sCopyToPopupMenu : sAppendToPopupMenu; }
	static bool			GetPopupMboxSend(bool copy_to, short item_num, CMbox*& found, bool& set_as_default);
	static bool			GetPopupMbox(bool copy_to, short item_num, CMbox*& found, bool do_choice = true);
	static bool			GetPopupMbox(bool copy_to, short item_num, bool do_choice, bool sending, CMbox*& found, bool& set_as_default);
	static bool			GetPopupMboxName(bool copy_to, short item_num, cdstring& found, bool do_choice = true);
	static SInt16		FindPopupMboxPos(bool copy_to, const char* name);

private:
	static void			ChangedList(CTreeNodeList* list);

	static bool			TestPrefix(const CMboxList& list, const cdstring& offset);

	static void			ResetAll(LMenu* main_menu, MenuHandle popup_menu, ulvector& breaks, bool copy_to);
	static void			ResetFavourite(LMenu* main_menu, MenuHandle popup_menu, ulvector& breaks, const CMboxRefList& items, const CMboxRefList& mrus, bool copy_to);
	static void			AddProtocolToMenu(CMboxProtocol* proto, MenuHandle menuH, short& pos, bool acct_name, bool single = false);
	static void			AppendListToMenu(const CMboxList& list, MenuHandle menuH, short& pos, bool acct_name, const cdstring& offset);
	static void			AppendListToMenu(const CMboxRefList& list, bool dynamic, MenuHandle menuH, short& pos, bool acct_name, const cdstring& offset, bool reverse = false);
	static void			AppendMbox(const CTreeNode* mbox, MenuHandle menuH, short& pos, bool acct_name, const cdstring& offset, size_t offset_length);
};

#endif
