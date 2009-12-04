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

#ifndef __MAIN_MENU_H
#define __MAIN_MENU_H

/* This class creates menus for the menubars of each window.  You ask
	 it to create a menubar, which it returns to you.  It also fills in
	 an array with pointers to each of the menus it returns, (including
	 submenus).  Subscripts coresponding to menus that weren't requested
	 will be set to NULL.  Which menus you want are indicated by or'ing
	 together the flags.

	 The enums matching the menu choices are also declared inside this
	 class.

	 Note that any selection events that are not handled in a given
	 class should be passed to CMulberryApp::HandleMainMenu, therefore you
	 should continue to listen to ALL returned menus, not just the ones you are 
	 interested in.

*/
class JXConainter;
class JBroadcaster;
class JXMenu;
class JXTextMenu;
class JXMenuBar;

#include <JXWidget.h>
#include <JPtrArray.h>

struct SMenuBuilder;

class CMainMenu
{
public:
	static CMainMenu *sMMenu;
	static const unsigned short  
		fFile = 1,
		fEdit = 1 << 1,
		fMailboxes = 1 << 2,
		fMessages = 1 << 3,
		fDraft = 1 << 4,
		fAddresses = 1 << 5,
		fCalendar = 1 << 6,
		fWindows = 1 << 7,
		fSpeak = 1 << 8,
		fHelp = 1 << 9;
	enum MenuId {eFile, eEdit, eMailboxes, eDisplayHierarchy,
							 eCabinets, eSortBy, eApplyRules, eMessages, eCopyTo, eFlags, eDecode,
							 eDraft, eNewTextPart, eNewMultipart, eCurrentTextPart, eAppendTo, eNotifications,
							 eFont, eSize, eStyle, eAlignment, eColour,
							 eAddresses, eImportAddresses, eExportAddresses,
							 eCalendar,
							 eWindows, eToolbar, eLayout, eHelp, eNumMenus};
	typedef JXTextMenu *MenusArray[eNumMenus]; //one for each menu
	
	enum {kFileNewMessage = 1, kFileOpenMailbox, kFileOpenMessage,
	      kFileOpenDraft, kFileImport, kFileExport, kFileClose, kFileSave,
	      kFileSaveAs, kFileRevert, kFilePreferences,
	      kFileDisconnect, kFileShowOutgoingQueues, kFilePageSetup, 
		  kFilePrint, kFileExit};
	
	enum {kEditUndo = 1, kEditRedo, kEditCut, kEditCopy, kEditPaste, 
	      kEditPasteAsQuote, kEditDelete, kEditSelectAll, kEditFind,
	      kEditFindNext, kEditFindSel, kEditReplace, kEditReplaceFindNext,
	      kEditReplaceAll, kEditTextMacros, kEditProperties, kEditChangePassword, 
	      kEditCheckSpelling};

	enum {kMailboxCreate = 1, kMailboxRename, kMailboxMarkContentsSeen,
	      kMailboxDelete, kMailboxExpunge, kMailboxSynchronise,
		  kMailboxClearDisconnected, kMailboxDisplayHierarchy, kMailboxCabinets,
	      kMailboxSubscribe, kMailboxUnsubscribe, kMailboxCheckMail,
	      kMailboxCheckFavourites, kMailboxSortBy, kMailboxGotoMessage,
		  kMailboxLoadAllMessages, kMailboxUpdateMatch, kMailboxSearch,
	      kMailboxSearchAgain, kMailboxNextSearch,
	      kMailboxRules, kMailboxApplyRules, kMailboxMakeRule};
	enum {kDispHNew = 1, kDispHEdit, kDispHRemove, kDispHReset, kDispHRefresh};
	enum {kCabNew = 1, kCabEdit, kCabRemove, kCabAddWild,kCabEditWild,
				kCabRemoveItem};
	enum {kSortTo = 1, kSortFrom, kSortReply, kSortSender, kSortCC, kSortSubject, 
				kSortThread, kSortDateSent, kSortDateReceived, kSortSize, kSortFlags,
				kSortNumber, kSortSmartAddress, kSortAttachments, kSortParts,
				kSortMatching, kSortDisconnected};

	enum {kMessagesReadPrev = 1, kMessagesReadNext, kMessagesDelAndReadNext,
				kMessagesCopyAndReadNext, kMessagesReply, kMessagesReplyToSender,
				kMessagesReplyToFrom, kMessagesReplyToAll, kMessagesForward,
				kMessagesBounce, kMessagesReject, kMessagesSendAgain, kMessagesCreateDigest,
				kMessagesCopyTo, kMessagesCopyNow, kMessagesFlags,
				kMessagesViewParts, kMessagesExtractParts, kMessagesDecodeEmbedded,
				kMessagesVerifyDecrypt, kMessagesDelete, kMessagesShowHeader};
	enum {kFlagsSeen = 1, kFlagsImportant, kFlagsAnswered, kFlagsDeleted, 
				kFlagsDraft, kFlagsLabel1, kFlagsLabel2, kFlagsLabel3, kFlagsLabel4, kFlagsLabel5, kFlagsLabel6, kFlagsLabel7, kFlagsLabel8};
	enum {kDecodeBinHex = 1, kDecodeUU};
	
	enum {kDraftIncludeOrig = 1, kDraftInsertSig, kDraftAddCcFile, kDraftAddBccFile, kDraftAttachFile, 
				kDraftNewTextPart, kDraftNewMultipart, kDraftCurrentTextPart, kDraftSend, kDraftAppendTo,
				kDraftAppendNow, kDraftCopyOriginal, kDraftSign, kDraftEncrypt, kDraftNotifications,
				kDraftWrap, kDraftUnwrap, kDraftQuote,
				kDraftUnquote, kDraftRequote, kDraftShiftLeft, kDraftShiftRight,
				kDraftFont, kDraftSize, kDraftStyle, kDraftAlignment, kDraftColour};
	enum {kNewTextPlain = 1, kNewTextEnriched, kNewTextHTML};
	enum {kNewMultiMixed = 1, kNewMultiParallel, kNewMultiDigest, kNewMultiAlt};
	enum {kCurrentTextPlain = 1, kCurrentTextPlainWrapped, kCurrentTextEnriched, kCurrentTextHTML};
	enum {kNotifyReceipt = 1, kNotifySuccess, kNotifyFailure, kNotifyDelay, kNotifyEntireMsg};
	enum {kStylePlain = 1, kStyleBold, kStyleItalic, kStyleUnderline};
	enum {kAlignLeft = 1, kAlignCenter, kAlighRight, kAlighJustified};
	enum {kColourBlack = 1, kColourRed, kColourGreen, kColourBlue,
			kColourYellow, kColourCyan, kColourMagenta, kColourMulberry,
			kColourWhite, kColourOther};

	enum {kAddressesAddBookMan = 1, kAddressesNew, kAddressesOpen, 
				kAddressesRename, kAddressesImport, kAddressesExport, kAddressesDelete,
				kAddressesLogin, kAddressesLogout, kAddessesRefresh,
				kAddressesSynchronise, kAddressesClearDisconnected,
				kAddressesExpandAddresses, kAddressesCaptureAddress, kAddressSearch};

	enum {kCalendarManager = 1, kCalendarSubscribe,
				kCalendarCreate, kCalendarRename, kCalendarCheck, kCalendarRefresh,
				kCalendarDelete,
				kCalendarAddWeb, kCalendarRefreshWeb, kCalendarUploadWeb,
				kCalendarNewEvent, kCalendarNewToDo, kCalendarEditItem, kCalendarDuplicateItem, kCalendarDeleteItem,
				kCalendarGotoToday, kCalendarGotoDate};

	enum {kWindowsCascade = 1, kWindowsTileHor, kWindowsTileVert, 
	      kWindowsSaveDefault, kWindowsResetToDefault, kWindowsExpandHeader, kWindowsShowParts, kWindowsShowQuickSearch,
		  kWindowsToolbars, kWindowsLayout, kWindowsShowList, kWindowsShowItems, kWindowsShowPreview,\
		  kWindowsOptions, kWindowsStatus, kWindowsDynamicStart};
	enum {kToolbarsButtons = 1, kToolbarsSmall, kToolbarsIcons, kToolbarsCaptions, kToolbarsBoth};
	enum {kLayout3Pane = 1, kLayoutSingle, kLayoutListVert, kLayoutListHoriz,
		  kLayoutItemsVert, kLayoutItemsHoriz, kLayoutPreviewVert, kLayoutPreviewHoriz,
		  kLayoutAllVert, kLayoutAllHoriz};

	enum {kHelpHelpTopics = 1, kHelpAboutMul, kHelpAboutPlugins, kHelpDetailedTooltips, kHelpMulberryOnline,
	      kHelpMulberryFAQ, kHelpMulberryUpdates, kHelpMulberrySupport};

	enum EContextMenu
	{
		eContextEdit = 1,
		eContextServerTable,
		eContextMailboxTable,
		eContextMessageAddr,
		eContextMessageSubj,
		eContextMessageParts,
		eContextMessageText,
		eContextMessageTextURL,
		eContextLetterAddr,
		eContextLetterParts,
		eContextLetterText,
		eContextAdbkManager,
		eContextAdbkSearch,
		eContextAdbkTable,
		eContextAdbkGroup,
		eContextCalendarStore,
		eContextCalendarEventTable,
		eContextCalendarToDoTable,
		eContextCalendarEventItem,
		eContextCalendarToDoItem
	};

	const static int cMenuHeight = 20;

	CMainMenu(){}
	JXMenuBar* CreateMenuBar(unsigned short which, JXContainer* enclosure, MenusArray* arrayToFill);

	void CreateContext(EContextMenu context, JXContainer* enclosure, JPtrArray<JXMenu>& menus);

private:
	JXTextMenu* BuildMenu(JXMenuBar* menu_bar, const char* title, const SMenuBuilder* items);
	void BuildMenu(JXTextMenu* menu, const SMenuBuilder* items);
	void CreateSubMenu(JXTextMenu* parent, const char* cmd, const SMenuBuilder* items,
								JXContainer* enclosure, JPtrArray<JXMenu>& menus);
};
	 
#endif
