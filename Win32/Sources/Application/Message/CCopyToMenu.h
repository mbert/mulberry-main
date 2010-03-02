/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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
#include "cdstring.h"

// Classes

class CMbox;
class CMboxList;
class CMboxProtocol;
class CMboxRefList;
class CTreeNode;
class CTreeNodeList;

class CCopyToMenu : public CListener
{
	friend class CMailboxPopup;
	friend class CMailboxToolbarPopup;

public:
	static CCopyToMenu*	sMailboxMainMenu;

						CCopyToMenu();
	virtual 			~CCopyToMenu();

	void				ListenTo_Message(long msg, void* param);	// Respond to list changes

	static void			SetMenuOptions(bool use_copyto_cabinet, bool use_appendto_cabinet);
	static void			DirtyMenuList()
		{ sCopyToReset = true; sAppendToReset = true; }
	static void			ResetMenuList();

	static bool			GetMbox(bool copy_to, int item_num, CMbox*& found);

 	static CMenu*		GetMenu(bool copy_to)
		{ return copy_to ? &sCopyToMenu : &sAppendToMenu; }
 	static CMenu*		GetPopupMenu(bool copy_to)
		{ return copy_to ? &sCopyToPopupMenu : &sAppendToPopupMenu; }
	static bool			GetPopupMboxSend(bool copy_to, short item_num, CMbox*& found, bool& set_as_default);
	static bool			GetPopupMbox(bool copy_to, short item_num, CMbox*& found, bool do_choice = true);
	static bool			GetPopupMbox(bool copy_to, short item_num, bool do_choice, bool sending, CMbox*& found, bool& set_as_default);
	static bool			GetPopupMboxName(bool copy_to, int item_num, cdstring& found, bool do_choice = true);
	static SInt16		FindPopupMboxPos(bool copy_to, const char* name);

private:
	typedef std::vector<CMenu*> CMenuList;

	static CMenu		sCopyToMenu;
	static CMenu		sAppendToMenu;
	static CMenu		sCopyToPopupMenu;
	static ulvector		sCopyToPopupServerBreaks;
	static CMenu		sAppendToPopupMenu;
	static ulvector		sAppendToPopupServerBreaks;
	static CMenuList	sCopyToMenuList;
	static CMenuList	sAppendToMenuList;
	static bool			sCopyToReset;
	static bool			sAppendToReset;
	static bool			sUseCopyToCabinet;
	static bool			sUseAppendToCabinet;
	static bool			sStripSubscribePrefix;
	static bool			sStripFavouritePrefix;

	static void			ChangedList(CTreeNodeList* list);

	static bool			TestPrefix(const CMboxList& list, const cdstring& offset);

	static void			ResetAll(CMenu* main_menu, CMenu* popup_menu, ulvector& breaks, int main_offset, int popup_offset, bool copy_to);
	static void			ResetFavourite(CMenu* main_menu, CMenu* popup_menu, ulvector& breaks,
										const CMboxRefList& items, const CMboxRefList& mrus,
										int main_offset, int popup_offset, bool copy_to);
	static void			AddProtocolToMenu(CMboxProtocol* proto, CMenu* menu, int& menu_row, int& pos, int offset, bool acct_name, bool single = false);
	static void			AppendListToMenu(const CMboxList& list, CMenu* menu, int& menu_row, int& pos, int offset, bool acct_name, const cdstring& noffset);
	static void			AppendListToMenu(const CMboxRefList& list, bool dynamic, CMenu* menu, int& menu_row, int& pos, int offset, bool acct_name, const cdstring& noffset, bool reverse = false);
	static void			AppendMbox(const CTreeNode* mbox, CMenu* menu, int& menu_row, int& pos, int offset, bool acct_name, const cdstring& noffset, size_t noffset_length);
};

#endif
