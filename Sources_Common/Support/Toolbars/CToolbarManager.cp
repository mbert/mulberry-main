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


// Source for CToolbarManager class

#include "CToolbarManager.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
#include "CCommands.h"
#endif
#include "char_stream.h"
#include "CFilterManager.h"
#include "CPreferences.h"
#include "CStringUtils.h"
#include "CToolbar.h"
#include "CXStringResources.h"

#include <algorithm>

CToolbarManager	CToolbarManager::sToolbarManager;

// Classes

#if __dest_os == __linux_os
enum
{
	ePopupMenu_Ignore = 0,
	ePopupMenu_Select,
	ePopupMenu_Flags
};
#endif

CToolbarItem::SToolbarItem sItems[] =
{
	// Common items
#if __dest_os == __mac_os || __dest_os == __mac_os_x

	{ CToolbarItem::eSeparator, CToolbarManager::eToolbar_Separator, false, 1750, false, 0, 0, 0, 0, 0 }, 																// Separator bar
	{ CToolbarItem::eSpace, CToolbarManager::eToolbar_Space, false, 1751, false, 0, 0, 0, 0, 0 }, 																		// Space
	{ CToolbarItem::eExpandSpace, CToolbarManager::eToolbar_ExpandSpace, false, 1752, false, 0, 0, 0, 0, 0 }, 															// Expandable Space

	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Draft, false, 9002, false, cmd_ToolbarNewLetter, cmd_ToolbarNewLetterOption, 0, 0, 0 }, 						// Draft
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Details, false, 4010, false, cmd_ToolbarDetailsBtn, 0, 0, 0, 0 }, 											// Details
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Expunge, false, 1002, false, cmd_ToolbarMailboxExpungeBtn, 0, 0, 0, 0 }, 									// Expunge
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Search, false, 1015, false, cmd_ToolbarSearchMailboxBtn, 0, 0, 0, 0 }, 										// Search
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Reply, false, 2000, false, cmd_ToolbarMessageReplyBtn, cmd_ToolbarMessageReplyOptionBtn, 0, 0, 0 }, 			// Reply
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ReplyAll, false, 2020, false, cmd_ToolbarMessageReplyAllBtn, 0, 0, 0, 0 }, 									// Reply All
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Forward, false, 2002, false, cmd_ToolbarMessageForwardBtn, cmd_ToolbarMessageForwardOptionBtn, 0, 0, 0 }, 	// Forward
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Bounce, false, 2013, false, cmd_ToolbarMessageBounceBtn, 0, 0, 0, 0 }, 										// Bounce
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Reject, false, 2017, false, cmd_ToolbarMessageRejectBtn, 0, 0, 0, 0 }, 										// Reject
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_SendAgain, false, 2018, false, cmd_ToolbarMessageSendAgainBtn, 0, 0, 0, 0 }, 								// Send Again
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Digest, false, 2019, false, cmd_ToolbarMessageDigestBtn, 0, 0, 0, 0 }, 										// Digest
	{ CToolbarItem::eCopyToButton, CToolbarManager::eToolbar_CopyTo, true, 3009, false, cmd_ToolbarMessageCopyBtn, cmd_ToolbarMessageCopyOptionBtn,
										cmd_ToolbarMessageCopyPopup, cmd_ToolbarMessageCopyOptionPopup, 2000 }, 														// Copyto
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Delete, true, 1000, false, cmd_ToolbarMessageDeleteBtn, 0, 0, 0, 0 }, 										// Delete
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Header, false, 2021, false, cmd_ToolbarMessageHeaderBtn, 0, 0, 0, 0 }, 										// Header
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Save, false, 1151, false, cmd_ToolbarFileSaveBtn, 0, 0, 0, 0 }, 												// Save
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Print, false, 2014, false, cmd_ToolbarMessagePrintBtn, 0, 0, 0, 0 }, 										// Print
	
	// 3pane window items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ShowList, false, 1740, false, cmd_ToolbarShowList, 0, 0, 0, 0 }, 											// Show List
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ShowItems, false, 1741, false, cmd_ToolbarShowItems, 0, 0, 0, 0 }, 											// Show Items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ShowPreview, false, 1742, false, cmd_ToolbarShowPreview, 0, 0, 0, 0 }, 										// Show Preview

	// Server pane items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Connect, true, 4000, true, cmd_ToolbarServerLoginBtn, 0, 0, 0, 0 }, 											// Connect
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Login, true, 4000, true, cmd_ToolbarServerLoginBtn, 0, 0, 0, 0 }, 											// Login
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Create, false, 4020, false, cmd_ToolbarCreateMailboxBtn, 0, 0, 0, 0 }, 										// Create
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Check, false, 4019, false, cmd_ToolbarCheckMailboxBtn, 0, 0, 0, 0 }, 										// Check
	{ CToolbarItem::eCabinetButton, CToolbarManager::eToolbar_Cabinet, false, 4023, false, cmd_ToolbarCabinetPopup, 0, 0, 0, 4002 }, 									// Cabinet
	
	// Mailbox pane items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Match, false, 1017, false, cmd_ToolbarMailboxMatchBtn, cmd_ToolbarMailboxMatchOptionBtn, 0, 0, 0 }, 			// Match
	{ CToolbarItem::eSelectButton, CToolbarManager::eToolbar_Select, false, 1016, false, cmd_ToolbarMailboxSelectPopup, 0, 0, 0, 1003 }, 								// Select
	{ CToolbarItem::eStaticIcon, CToolbarManager::eToolbar_MailboxType, false, 1150, false, 0, 0, 0, 0, 0 },							 								// Mailbox type
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ApplyRules, false, 1202, false, cmd_ToolbarMailboxApplyRules, 0, 0, 0, 0 },							 		// Apply Rules
	
	// SMTP Items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Hold, false, 1501, false, cmd_ToolbarSMTPHoldBtn, 0, 0, 0, 0 }, 												// Hold
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Priority, false, 1503, false, cmd_ToolbarSMTPPriorityBtn, 0, 0, 0, 0 }, 										// Priority
	{ CToolbarItem::eSMTPAccountPopup, CToolbarManager::eToolbar_Accounts, false, 3000, false, cmd_ToolbarSMTPAccountsPopup, 0, 0, 0, 5016 }, 							// Accounts
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Enable, true, 1500, false, cmd_ToolbarSMTPEnableBtn, 0, 0, 0, 0 }, 											// Enable
	
	// Message
	{ CToolbarItem::ePopupButton, CToolbarManager::eToolbar_Flags, false, 1018, false, cmd_ToolbarMessageFlagsBtn, 0, 0, 0, 171 }, 										// Flags
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_DeleteNext, false, 2005, false, cmd_ToolbarMessageDeleteNextBtn, 0, 0, 0, 0 }, 								// Delete and Next
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Previous, false, 2004, false, cmd_ToolbarMessagePreviousBtn, 0, 0, 0, 0 }, 									// Previous
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Next, false, 2003, false, cmd_ToolbarMessageNextBtn, 0, 0, 0, 0 }, 											// Next

	// Letter
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Attach, false, 3002, false, cmd_ToolbarLetterAttachBtn, 0, 0, 0, 0 }, 										// Attach
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Spell, false, 3011, false, cmd_ToolbarLetterSpellBtn, 0, 0, 0, 0 }, 											// Spell
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Receipt, false, 3012, false, cmd_ToolbarLetterReceiptBtn, 0, 0, 0, 0 }, 										// Receipt
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_DSN, false, 3012, false, cmd_ToolbarLetterDSNBtn, 0, 0, 0, 0 }, 												// DSN
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Signature, false, 3010, false, cmd_ToolbarLetterSignatureBtn, 0, 0, 0, 0 }, 									// Signature
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Sign, false, 3013, false, cmd_ToolbarLetterSignPopup, 0, 0, 0, 0 }, 											// Sign
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Encrypt, false, 3014, false, cmd_ToolbarLetterEncryptPopup, 0, 0, 0, 0 }, 									// Encrypt
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Send, false, 3000, false, cmd_ToolbarLetterSendBtn, 0, 0, 0, 0 }, 											// Send
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_AddCC, false, 3016, false, cmd_ToolbarLetterAddCCBtn, 0, 0, 0, 0 }, 											// Add CC
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_AddBcc, false, 3017, false, cmd_ToolbarLetterAddBccBtn, 0, 0, 0, 0 }, 										// Add Bcc
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_AppendNow, false, 3009, false, cmd_ToolbarLetterAppendNowBtn, cmd_ToolbarLetterAppendNowOptionBtn, 0, 0, 0 },// Append Now
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CopyOriginal, false, 3018, false, cmd_ToolbarLetterCopyOriginalBtn, 0, 0, 0, 0 }, 							// Copy Original
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ExternalEdit, false, 3000, false, cmd_ToolbarLetterSpellBtn, 0, 0, 0, 0 }, 									// Edit

	// Address Book Manager
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewAdbk, false, 9500, false, cmd_ToolbarAdbkMgrNewBtn, 0, 0, 0, 0 }, 										// New Adbk
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_OpenAdbk, false, 9501, false, cmd_ToolbarAdbkMgrOpenBtn, 0, 0, 0, 0 }, 										// Open Adbk
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_DeleteAdbk, false, 9502, false, cmd_ToolbarAdbkMgrDeleteBtn, 0, 0, 0, 0 }, 									// Delete Adbk
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_SearchAddress, false, 9550, false, cmd_ToolbarAdbkMgrSearchBtn, 0, 0, 0, 0 }, 								// Search Address

	// Address Book
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewAddress, false, 9000, false, cmd_ToolbarAddressBookAddressBtn, 0, 0, 0, 0 }, 								// New Address
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewGroup, false, 9001, false, cmd_ToolbarAddressBookGroupBtn, 0, 0, 0, 0 }, 									// New Group
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_EditItem, false, 9003, false, cmd_ToolbarAddressBookEditBtn, 0, 0, 0, 0 }, 									// Edit Item
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_DeleteItem, false, 9004, false, cmd_ToolbarAddressBookDeleteBtn, 0, 0, 0, 0 }, 								// Delete Item

	// Calendar
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewCalendar, false, 1808, false, cmd_ToolbarNewCalendarBtn, 0, 0, 0, 0 }, 									// New Calendar
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewEvent, false, 1810, false, cmd_ToolbarNewEventBtn, 0, 0, 0, 0 }, 											// New Event
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewToDo, false, 1811, false, cmd_ToolbarNewToDoBtn, 0, 0, 0, 0 }, 											// New To Do
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ShowToday, false, 1812, false, cmd_ToolbarShowTodayBtn, 0, 0, 0, 0 }, 										// Show Today
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_GoToDate, false, 1813, false, cmd_ToolbarGoToDateBtn, 0, 0, 0, 0 }, 											// Go to date
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarDay, false, 1800, false, cmd_ToolbarCalendarDayBtn, 0, 0, 0, 0 }, 									// Day View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarWorkWeek, false, 1801, false, cmd_ToolbarCalendarWorkWeekBtn, 0, 0, 0, 0 }, 							// Work Week View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarWeek, false, 1802, false, cmd_ToolbarCalendarWeekBtn, 0, 0, 0, 0 }, 									// Week View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarMonth, false, 1803, false, cmd_ToolbarCalendarMonthBtn, 0, 0, 0, 0 }, 								// Month View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarYear, false, 1804, false, cmd_ToolbarCalendarYearBtn, 0, 0, 0, 0 }, 									// Year View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarSummary, false, 1805, false, cmd_ToolbarCalendarSummaryBtn, 0, 0, 0, 0 }, 							// Summary View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarToDo, false, 1806, false, cmd_ToolbarCalendarToDoBtn, 0, 0, 0, 0 }, 									// To Do View

	// Window
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CollapseHeader, false, 3019, false, cmd_ToolbarCollapseHeader, 0, 0, 0, 0 }, 								// Collapse Header
	
	{ CToolbarItem::eNone, 0, 0, 0, 0, 0, 0, 0 }, 																														// Last item

#elif __dest_os == __win32_os

	{ CToolbarItem::eSeparator, CToolbarManager::eToolbar_Separator, false, IDI_TOOLBAR_DIVIDER, false, 0, 0, 0, 0, 0 }, 												// Separator bar
	{ CToolbarItem::eSpace, CToolbarManager::eToolbar_Space, false, IDI_TOOLBAR_SPACE, false, 0, 0, 0, 0, 0 }, 															// Space
	{ CToolbarItem::eExpandSpace, CToolbarManager::eToolbar_ExpandSpace, false, IDI_TOOLBAR_EXPAND, false, 0, 0, 0, 0, 0 }, 											// Expandable Space

	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Draft, false, IDI_MAILBOXNEWMSG, false, IDC_TOOLBARNEWLETTER, IDC_TOOLBARNEWLETTEROPTION, 0, 0, 0 }, 		// Draft
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Details, false, IDI_SERVERPROP, false, IDC_TOOLBARDETAILSBTN, 0, 0, 0, 0 }, 									// Details
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Expunge, false, IDI_MAILBOXEXPUNGE, false, IDC_TOOLBARMAILBOXEXPUNGEBTN, 0, 0, 0, 0 }, 							// Expunge
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Search, false, IDI_MAILBOXSEARCH, false, IDC_TOOLBARSEARCHMAILBOXBTN, 0, 0, 0, 0 }, 								// Search
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Reply, false, IDI_MAILBOXREPLY, false, IDC_TOOLBARMESSAGEREPLYBTN, IDC_TOOLBARMESSAGEREPLYOPTIONBTN, 0, 0, 0 }, 	// Reply
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ReplyAll, false, IDI_MAILBOXREPLYALL, false, IDC_TOOLBARMESSAGEREPLYALLBTN, 0, 0, 0, 0 }, 						// Reply All
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Forward, false, IDI_MAILBOXFORWARD, false, IDC_TOOLBARMESSAGEFORWARDBTN, IDC_TOOLBARMESSAGEFORWARDOPTIONBTN, 0, 0, 0 }, 	// Forward
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Bounce, false, IDI_MAILBOXBOUNCE, false, IDC_TOOLBARMESSAGEBOUNCEBTN, 0, 0, 0, 0 }, 										// Bounce
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Reject, false, IDI_MAILBOXREJECT, false, IDC_TOOLBARMESSAGEREJECTBTN, 0, 0, 0, 0 }, 										// Reject
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_SendAgain, false, IDI_MAILBOXSENDAGAIN, false, IDC_TOOLBARMESSAGESENDAGAINBTN, 0, 0, 0, 0 }, 							// Send Again
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Digest, false, IDI_MAILBOXDIGEST, false, IDC_TOOLBARMESSAGEDIGESTBTN, 0, 0, 0, 0 }, 										// Digest
	{ CToolbarItem::eCopyToButton, CToolbarManager::eToolbar_CopyTo, true, IDI_MAILBOXCOPY, false, IDC_TOOLBARMESSAGECOPYBTN, IDC_TOOLBARMESSAGECOPYOPTIONBTN,
										IDC_TOOLBARMESSAGECOPYPOPUP, IDC_TOOLBARMESSAGECOPYOPTIONPOPUP, 0 }, 																		// Copyto
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Delete, true, IDI_MAILBOXDELETE, false, IDC_TOOLBARMESSAGEDELETEBTN, 0, 0, 0, 0 }, 										// Delete
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Header, false, IDI_HEADERMSG, false, IDC_TOOLBARMESSAGEHEADERBTN, 0, 0, 0, 0 }, 											// Header
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Save, false, IDI_SAVEFILE, false, IDC_TOOLBARFILESAVEBTN, 0, 0, 0, 0 }, 													// Save
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Print, false, IDI_PRINT, false, IDC_TOOLBARMESSAGEPRINTBTN, 0, 0, 0, 0 }, 												// Print
	
	// 3pane window items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ShowList, false, IDI_SHOWLIST, false, IDC_TOOLBARSHOWLIST, 0, 0, 0, 0 }, 										// Show List
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ShowItems, false, IDI_SHOWITEM, false, IDC_TOOLBARSHOWITEMS, 0, 0, 0, 0 }, 										// Show Items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ShowPreview, false, IDI_SHOWPREVIEW, false, IDC_TOOLBARSHOWPREVIEW, 0, 0, 0, 0 }, 								// Show Preview

	// Server pane items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Connect, true, IDI_LOGIN, true, IDC_TOOLBARSERVERLOGINBTN, 0, 0, 0, 0 }, 										// Connect
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Login, true, IDI_LOGIN, true, IDC_TOOLBARSERVERLOGINBTN, 0, 0, 0, 0 }, 											// Login
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Create, false, IDI_SERVER_CREATEBTN, false, IDC_TOOLBARCREATEMAILBOXBTN, 0, 0, 0, 0 }, 							// Create
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Check, false, IDI_SERVER_CHECKBTN, false, IDC_TOOLBARCHECKMAILBOXBTN, 0, 0, 0, 0 }, 								// Check
	{ CToolbarItem::eCabinetButton, CToolbarManager::eToolbar_Cabinet, false, IDI_SERVER_VIEWBTN, false, IDC_TOOLBARCABINETPOPUP, 0, 0, 0, 0 }, 							// Cabinet
	
	// Mailbox pane items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Match, false, IDI_MAILBOXMATCH, false, IDC_TOOLBARMAILBOXMATCHBTN, IDC_TOOLBARMAILBOXMATCHOPTIONBTN, 0, 0, 0 }, 	// Match
	{ CToolbarItem::eSelectButton, CToolbarManager::eToolbar_Select, false, IDI_MAILBOXSELECTFLAG, false, IDC_TOOLBARMAILBOXSELECTPOPUP, 0, 0, 0, IDR_POPUP_MAILBOX_SELECTBY }, 								// Select
	{ CToolbarItem::eStaticIcon, CToolbarManager::eToolbar_MailboxType, false, IDI_MAILBOX_REMOTE, false, 0, 0, 0, 0, 0 },							 						// Mailbox type
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ApplyRules, false, IDI_RULESICON, false, IDC_TOOLBARMAILBOXAPPLYRULES, 0, 0, 0, 0 },							 	// Apply Rules
	
	// SMTP Items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Hold, false, IDI_SMTPHOLD, false, IDC_TOOLBARSMTPHOLDBTN, 0, 0, 0, 0 }, 											// Hold
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Priority, false, IDI_SMTPPRIORITY, false, IDC_TOOLBARSMTPPRIORITYBTN, 0, 0, 0, 0 }, 								// Priority
	{ CToolbarItem::eSMTPAccountPopup, CToolbarManager::eToolbar_Accounts, false, IDI_SENDLETTER, false, IDC_TOOLBARSMTPACCOUNTSPOPUP, 0, 0, 0, IDR_POPUP_SMTP_ACCOUNTS }, 	// Accounts
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Enable, true, IDI_SMTPENABLE, false, IDC_TOOLBARSMTPENABLEBTN, 0, 0, 0, 0 }, 									// Enable
	
	// Message
	{ CToolbarItem::ePopupButton, CToolbarManager::eToolbar_Flags, false, IDI_TITLE_FLAG_TITLE, false, IDC_TOOLBARMESSAGEFLAGSBTN, 0, 0, 0, IDR_POPUP_MAILBOX_FLAGS }, 		// Flags
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_DeleteNext, false, IDI_DELETENEXT, false, IDC_TOOLBARMESSAGEDELETENEXTBTN, 0, 0, 0, 0 }, 						// Delete and Next
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Previous, false, IDI_PREVMSG, false, IDC_TOOLBARMESSAGEPREVIOUSBTN, 0, 0, 0, 0 }, 								// Previous
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Next, false, IDI_NEXTMSG, false, IDC_TOOLBARMESSAGENEXTBTN, 0, 0, 0, 0 }, 										// Next

	// Letter
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Attach, false, IDI_ATTACHFILE, false, IDC_TOOLBARLETTERATTACHBTN, 0, 0, 0, 0 }, 									// Attach
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Spell, false, IDI_SPELLING, false, IDC_TOOLBARLETTERSPELLBTN, 0, 0, 0, 0 }, 										// Spell
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Receipt, false, IDI_RECEIPT, false, IDC_TOOLBARLETTERRECEIPTBTN, 0, 0, 0, 0 }, 									// Receipt
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_DSN, false, IDI_RECEIPT, false, IDC_TOOLBARLETTERDSNBTN, 0, 0, 0, 0 }, 											// DSN
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Signature, false, IDI_INSERTSIGNATURE, false, IDC_TOOLBARLETTERSIGNATUREBTN, 0, 0, 0, 0 }, 						// Signature
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Sign, false, IDI_SIGN, false, IDC_TOOLBARLETTERSIGNPOPUP, 0, 0, 0, 0 }, 											// Sign
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Encrypt, false, IDI_ENCRYPT, false, IDC_TOOLBARLETTERENCRYPTPOPUP, 0, 0, 0, 0 }, 								// Encrypt
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Send, false, IDI_SENDLETTER, false, IDC_TOOLBARLETTERSENDBTN, 0, 0, 0, 0 }, 										// Send
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_AddCC, false, IDI_ADD_CC, false, IDC_TOOLBARLETTERADDCCBTN, 0, 0, 0, 0 }, 											// Add CC
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_AddBcc, false, IDI_ADD_BCC, false, IDC_TOOLBARLETTERADDBCCBTN, 0, 0, 0, 0 }, 										// Add Bcc
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_AppendNow, false, IDI_APPEND_NOW, false, IDC_TOOLBARLETTERAPPENDNOWBTN, IDC_TOOLBARLETTERAPPENDNOWOPTIONBTN, 0, 0, 0 },// Append Now
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CopyOriginal, false, IDI_COPY_ORIGINAL, false, IDC_TOOLBARLETTERCOPYORIGINALBTN, 0, 0, 0, 0 }, 							// Copy Original
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ExternalEdit, false, IDI_SENDLETTER, false, IDC_TOOLBARLETTERSPELLBTN, 0, 0, 0, 0 }, 							// Edit

	// Address Book Manager
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewAdbk, false, IDI_ADBK_NEW_BTN, false, IDC_TOOLBARADBKMGRNEWBTN, 0, 0, 0, 0 }, 								// New Adbk
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_OpenAdbk, false, IDI_ADBK_OPEN_BTN, false, IDC_TOOLBARADBKMGROPENBTN, 0, 0, 0, 0 }, 								// Open Adbk
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_DeleteAdbk, false, IDI_ADBK_DELETE_BTN, false, IDC_TOOLBARADBKMGRDELETEBTN, 0, 0, 0, 0 }, 						// Delete Adbk
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_SearchAddress, false, IDI_ADDRESS_SEARCH, false, IDC_TOOLBARADBKMGRSEARCHBTN, 0, 0, 0, 0 }, 						// Search Address

	// Address Book
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewAddress, false, IDI_ADDRESS_NEW_SINGLE, false, IDC_TOOLBARADDRESSBOOKADDRESSBTN, 0, 0, 0, 0 }, 				// New Address
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewGroup, false, IDI_ADDRESS_NEW_GROUP, false, IDC_TOOLBARADDRESSBOOKGROUPBTN, 0, 0, 0, 0 }, 					// New Group
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_EditItem, false, IDI_ADDRESS_EDIT, false, IDC_TOOLBARADDRESSBOOKEDITBTN, 0, 0, 0, 0 }, 							// Edit Item
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_DeleteItem, false, IDI_ADDRESS_DELETE, false, IDC_TOOLBARADDRESSBOOKDELETEBTN, 0, 0, 0, 0 }, 					// Delete Item

	// Calendar
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewCalendar, false, IDI_CALENDAR_NEWCALENDAR, false, IDC_TOOLBAR_CALENDAR_NEWCALENDAR, 0, 0, 0, 0 }, 			// New Calendar
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewEvent, false, IDI_CALENDAR_NEWEVENT, false, IDC_TOOLBAR_CALENDAR_NEWEVENT, 0, 0, 0, 0 }, 						// New Event
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewToDo, false, IDI_CALENDAR_NEWTODO, false, IDC_TOOLBAR_CALENDAR_NEWTODO, 0, 0, 0, 0 }, 						// New To Do
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ShowToday, false, IDI_CALENDAR_TODAY, false, IDC_TOOLBAR_CALENDAR_TODAY, 0, 0, 0, 0 }, 							// Show Today
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_GoToDate, false, IDI_CALENDAR_GOTO, false, IDC_TOOLBAR_CALENDAR_GOTO, 0, 0, 0, 0 }, 								// Go to date
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarDay, false, IDI_CALENDAR_DAY, false, IDC_TOOLBAR_CALENDAR_DAY, 0, 0, 0, 0 }, 							// Day View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarWorkWeek, false, IDI_CALENDAR_WORK, false, IDC_TOOLBAR_CALENDAR_WORK, 0, 0, 0, 0 }, 						// Work Week View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarWeek, false, IDI_CALENDAR_WEEK, false, IDC_TOOLBAR_CALENDAR_WEEK, 0, 0, 0, 0 }, 							// Week View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarMonth, false, IDI_CALENDAR_MONTH, false, IDC_TOOLBAR_CALENDAR_MONTH, 0, 0, 0, 0 }, 						// Month View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarYear, false, IDI_CALENDAR_YEAR, false, IDC_TOOLBAR_CALENDAR_YEAR, 0, 0, 0, 0 }, 							// Year View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarSummary, false, IDI_CALENDAR_SUMMARY, false, IDC_TOOLBAR_CALENDAR_SUMMARY, 0, 0, 0, 0 }, 				// Summary View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarToDo, false, IDI_CALENDAR_TASKS, false, IDC_TOOLBAR_CALENDAR_TASKS, 0, 0, 0, 0 }, 						// To Do View

	// Window
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CollapseHeader, false, IDI_COLLAPSEHEADER, false, IDC_TOOLBARWINDOWCOLLAPSEHEADERBTN, 0, 0, 0, 0 }, 				// Collapse Header

	{ CToolbarItem::eNone, 0, 0, 0, 0, 0, 0, 0 }, 																															// Last item

#elif __dest_os == __linux_os

	{ CToolbarItem::eSeparator, CToolbarManager::eToolbar_Separator, false, IDI_TOOLBAR_DIVIDER, false, 0, 0, 0, 0, 0 }, 												// Separator bar
	{ CToolbarItem::eSpace, CToolbarManager::eToolbar_Space, false, IDI_TOOLBAR_SPACE, false, 0, 0, 0, 0, 0 }, 															// Space
	{ CToolbarItem::eExpandSpace, CToolbarManager::eToolbar_ExpandSpace, false, IDI_TOOLBAR_EXPAND, false, 0, 0, 0, 0, 0 }, 											// Expandable Space

	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Draft, false, IDI_MAILBOXNEWMSG, false, CCommand::eToolbarNewLetter, CCommand::eToolbarNewLetterOption, 0, 0, 0 }, 		// Draft
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Details, false, IDI_SERVERPROP, false, CCommand::eToolbarDetailsBtn, 0, 0, 0, 0 }, 									// Details
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Expunge, false, IDI_MAILBOXEXPUNGE, false, CCommand::eToolbarMailboxExpungeBtn, 0, 0, 0, 0 }, 							// Expunge
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Search, false, IDI_MAILBOXSEARCH, false, CCommand::eToolbarSearchMailboxBtn, 0, 0, 0, 0 }, 								// Search
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Reply, false, IDI_MAILBOXREPLY, false, CCommand::eToolbarMessageReplyBtn, CCommand::eToolbarMessageReplyOptionBtn, 0, 0, 0 }, 	// Reply
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ReplyAll, false, IDI_MAILBOXREPLYALL, false, CCommand::eToolbarMessageReplyAllBtn, 0, 0, 0, 0 }, 						// Reply All
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Forward, false, IDI_MAILBOXFORWARD, false, CCommand::eToolbarMessageForwardBtn, CCommand::eToolbarMessageForwardOptionBtn, 0, 0, 0 }, 	// Forward
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Bounce, false, IDI_MAILBOXBOUNCE, false, CCommand::eToolbarMessageBounceBtn, 0, 0, 0, 0 }, 										// Bounce
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Reject, false, IDI_MAILBOXREJECT, false, CCommand::eToolbarMessageRejectBtn, 0, 0, 0, 0 }, 										// Reject
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_SendAgain, false, IDI_MAILBOXSENDAGAIN, false, CCommand::eToolbarMessageSendAgainBtn, 0, 0, 0, 0 }, 							// Send Again
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Digest, false, IDI_MAILBOXDIGEST, false, CCommand::eToolbarMessageDigestBtn, 0, 0, 0, 0 }, 										// Digest
	{ CToolbarItem::eCopyToButton, CToolbarManager::eToolbar_CopyTo, true, IDI_MAILBOXCOPY, false, CCommand::eToolbarMessageCopyBtn, CCommand::eToolbarMessageCopyOptionBtn,
										CCommand::eToolbarMessageCopyPopup, CCommand::eToolbarMessageCopyOptionPopup, 0 }, 																		// Copyto
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Delete, true, IDI_MAILBOXDELETE, false, CCommand::eToolbarMessageDeleteBtn, 0, 0, 0, 0 }, 										// Delete
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Header, false, IDI_HEADERMSG, false, CCommand::eToolbarMessageHeaderBtn, 0, 0, 0, 0 }, 											// Header
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Save, false, IDI_SAVEFILE, false, CCommand::eToolbarFileSaveBtn, 0, 0, 0, 0 }, 													// Save
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Print, false, IDI_PRINT, false, CCommand::eToolbarMessagePrintBtn, 0, 0, 0, 0 }, 												// Print
	
	// 3pane window items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ShowList, false, IDI_SHOWLIST, false, CCommand::eToolbarShowList, 0, 0, 0, 0 }, 										// Show List
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ShowItems, false, IDI_SHOWITEM, false, CCommand::eToolbarShowItems, 0, 0, 0, 0 }, 										// Show Items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ShowPreview, false, IDI_SHOWPREVIEW, false, CCommand::eToolbarShowPreview, 0, 0, 0, 0 }, 								// Show Preview

	// Server pane items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Connect, true, IDI_LOGIN, true, CCommand::eToolbarServerLoginBtn, 0, 0, 0, 0 }, 										// Connect
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Login, true, IDI_LOGIN, true, CCommand::eToolbarServerLoginBtn, 0, 0, 0, 0 }, 											// Login
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Create, false, IDI_SERVER_CREATEBTN, false, CCommand::eToolbarCreateMailboxBtn, 0, 0, 0, 0 }, 							// Create
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Check, false, IDI_SERVER_CHECKBTN, false, CCommand::eToolbarCheckMailboxBtn, 0, 0, 0, 0 }, 							// Check
	{ CToolbarItem::eCabinetButton, CToolbarManager::eToolbar_Cabinet, false, IDI_SERVER_VIEWBTN, false, CCommand::eToolbarCabinetPopup, 0, 0, 0, 0 }, 							// Cabinet
	
	// Mailbox pane items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Match, false, IDI_MAILBOXMATCH, false, CCommand::eToolbarMailboxMatchBtn, CCommand::eToolbarMailboxMatchOptionBtn, 0, 0, 0 }, 	// Match
	{ CToolbarItem::eSelectButton, CToolbarManager::eToolbar_Select, false, IDI_MAILBOXSELECTFLAG, false, CCommand::eToolbarMailboxSelectPopup, 0, 0, 0, ePopupMenu_Select }, 								// Select
	{ CToolbarItem::eStaticIcon, CToolbarManager::eToolbar_MailboxType, false, IDI_MAILBOX_REMOTE, false, 0, 0, 0, 0, 0 },							 						// Mailbox type
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ApplyRules, false, IDI_RULESICON, false, CCommand::eToolbarMailboxApplyRules, 0, 0, 0, 0 },							 	// Apply Rules
	
	// SMTP Items
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Hold, false, IDI_SMTPHOLD, false, CCommand::eToolbarSMTPHoldBtn, 0, 0, 0, 0 }, 											// Hold
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Priority, false, IDI_SMTPPRIORITY, false, CCommand::eToolbarSMTPPriorityBtn, 0, 0, 0, 0 }, 								// Priority
	{ CToolbarItem::eSMTPAccountPopup, CToolbarManager::eToolbar_Accounts, false, IDI_SENDLETTER, false, CCommand::eToolbarSMTPAccountsPopup, 0, 0, 0, 0 }, 	// Accounts
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Enable, true, IDI_SMTPENABLE, false, CCommand::eToolbarSMTPEnableBtn, 0, 0, 0, 0 }, 									// Enable
	
	// Message
	{ CToolbarItem::ePopupButton, CToolbarManager::eToolbar_Flags, false, IDI_TITLE_FLAG_TITLE, false, CCommand::eToolbarMessageFlagsBtn, 0, 0, 0, ePopupMenu_Flags }, 		// Flags
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_DeleteNext, false, IDI_DELETENEXT, false, CCommand::eToolbarMessageDeleteNextBtn, 0, 0, 0, 0 }, 						// Delete and Next
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Previous, false, IDI_PREVMSG, false, CCommand::eToolbarMessagePreviousBtn, 0, 0, 0, 0 }, 								// Previous
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Next, false, IDI_NEXTMSG, false, CCommand::eToolbarMessageNextBtn, 0, 0, 0, 0 }, 										// Next

	// Letter
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Attach, false, IDI_ATTACHFILE, false, CCommand::eToolbarLetterAttachBtn, 0, 0, 0, 0 }, 									// Attach
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Spell, false, IDI_SPELLING, false, CCommand::eToolbarLetterSpellBtn, 0, 0, 0, 0 }, 										// Spell
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Receipt, false, IDI_RECEIPT, false, CCommand::eToolbarLetterReceiptBtn, 0, 0, 0, 0 }, 									// Receipt
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_DSN, false, IDI_RECEIPT, false, CCommand::eToolbarLetterDSNBtn, 0, 0, 0, 0 }, 											// DSN
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Signature, false, IDI_INSERTSIGNATURE, false, CCommand::eToolbarLetterSignatureBtn, 0, 0, 0, 0 }, 						// Signature
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Sign, false, IDI_SIGN, false, CCommand::eToolbarLetterSignBtn, 0, 0, 0, 0 }, 											// Sign
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Encrypt, false, IDI_ENCRYPT, false, CCommand::eToolbarLetterEncryptBtn, 0, 0, 0, 0 }, 								// Encrypt
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_Send, false, IDI_SENDLETTER, false, CCommand::eToolbarLetterSendBtn, 0, 0, 0, 0 }, 										// Send
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_AddCC, false, IDI_ADD_CC, false, CCommand::eToolbarLetterAddCcBtn, 0, 0, 0, 0 }, 											// Add CC
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_AddBcc, false, IDI_ADD_BCC, false, CCommand::eToolbarLetterAddBccBtn, 0, 0, 0, 0 }, 										// Add Bcc
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_AppendNow, false, IDI_APPEND_NOW, false, CCommand::eToolbarLetterAppendNowBtn, CCommand::eToolbarLetterAppendNowBtn, 0, 0, 0 },// Append Now
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CopyOriginal, false, IDI_COPY_ORIGINAL, false, CCommand::eToolbarLetterCopyOriginalBtn, 0, 0, 0, 0 }, 							// Copy Original
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ExternalEdit, false, IDI_EXTERNALEDIT, false, CCommand::eToolbarLetterExternalBtn, 0, 0, 0, 0 }, 										// Send

	// Address Book Manager
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewAdbk, false, IDI_ADBK_NEW_BTN, false, CCommand::eToolbarAdbkMgrNewBtn, 0, 0, 0, 0 }, 								// New Adbk
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_OpenAdbk, false, IDI_ADBK_OPEN_BTN, false, CCommand::eToolbarAdbkMgrOpenBtn, 0, 0, 0, 0 }, 								// Open Adbk
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_DeleteAdbk, false, IDI_ADBK_DELETE_BTN, false, CCommand::eToolbarAdbkMgrDeleteBtn, 0, 0, 0, 0 }, 						// Delete Adbk
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_SearchAddress, false, IDI_ADDRESS_SEARCH, false, CCommand::eToolbarAdbkMgrSearchBtn, 0, 0, 0, 0 }, 						// Search Address

	// Address Book
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewAddress, false, IDI_ADDRESS_NEW_SINGLE, false, CCommand::eToolbarAddressBookAddressBtn, 0, 0, 0, 0 }, 				// New Address
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewGroup, false, IDI_ADDRESS_NEW_GROUP, false, CCommand::eToolbarAddressBookGroupBtn, 0, 0, 0, 0 }, 					// New Group
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_EditItem, false, IDI_ADDRESS_EDIT, false, CCommand::eToolbarAddressBookEditBtn, 0, 0, 0, 0 }, 							// Edit Item
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_DeleteItem, false, IDI_ADDRESS_DELETE, false, CCommand::eToolbarAddressBookDeleteBtn, 0, 0, 0, 0 }, 					// Delete Item

	// Calendar
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewCalendar, false, IDI_CALENDAR_NEWCALENDAR, false, CCommand::eToolbarCalendarCreateBtn, 0, 0, 0, 0 }, 			// New Calendar
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewEvent, false, IDI_CALENDAR_NEWEVENT, false, CCommand::eToolbarCalendarNewEventBtn, 0, 0, 0, 0 }, 				// New Event
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_NewToDo, false, IDI_CALENDAR_NEWTODO, false, CCommand::eToolbarCalendarNewToDoBtn, 0, 0, 0, 0 }, 				// New To Do
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_ShowToday, false, IDI_CALENDAR_TODAY, false, CCommand::eToolbarCalendarTodayBtn, 0, 0, 0, 0 }, 					// Show Today
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_GoToDate, false, IDI_CALENDAR_GOTO, false, CCommand::eToolbarCalendarGotoBtn, 0, 0, 0, 0 }, 						// Go to date
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarDay, false, IDI_CALENDAR_DAY, false, CCommand::eToolbarCalendarDayBtn, 0, 0, 0, 0 }, 					// Day View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarWorkWeek, false, IDI_CALENDAR_WORK, false, CCommand::eToolbarCalendarWorkBtn, 0, 0, 0, 0 }, 				// Work Week View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarWeek, false, IDI_CALENDAR_WEEK, false, CCommand::eToolbarCalendarWeekBtn, 0, 0, 0, 0 }, 					// Week View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarMonth, false, IDI_CALENDAR_MONTH, false, CCommand::eToolbarCalendarMonthBtn, 0, 0, 0, 0 }, 				// Month View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarYear, false, IDI_CALENDAR_YEAR, false, CCommand::eToolbarCalendarYearBtn, 0, 0, 0, 0 }, 					// Year View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarSummary, false, IDI_CALENDAR_SUMMARY, false, CCommand::eToolbarCalendarSummaryBtn, 0, 0, 0, 0 }, 		// Summary View
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CalendarToDo, false, IDI_CALENDAR_TASKS, false, CCommand::eToolbarCalendarTasksBtn, 0, 0, 0, 0 }, 				// To Do View

	// Window
	{ CToolbarItem::ePushButton, CToolbarManager::eToolbar_CollapseHeader, false, IDI_COLLAPSEHEADER, false, CCommand::eToolbarWindowsExpandHeaderBtn, 0, 0, 0, 0 }, 			// Collapse Header

	{ CToolbarItem::eNone, 0, 0, 0, 0, 0, 0, 0 }, 																															// Last item

#endif

};

// Allowed items

// 3Pane Mailbox
int sAllow_3PaneMailbox[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Login,
	CToolbarManager::eToolbar_Create,
	CToolbarManager::eToolbar_Cabinet,
	CToolbarManager::eToolbar_Check,
	CToolbarManager::eToolbar_Search,
	CToolbarManager::eToolbar_Flags,
	CToolbarManager::eToolbar_Reply,
	CToolbarManager::eToolbar_ReplyAll,
	CToolbarManager::eToolbar_Forward,
	CToolbarManager::eToolbar_Bounce,
	CToolbarManager::eToolbar_Reject,
	CToolbarManager::eToolbar_SendAgain,
	CToolbarManager::eToolbar_Digest,
	CToolbarManager::eToolbar_CopyTo,
	CToolbarManager::eToolbar_Delete,
	CToolbarManager::eToolbar_Expunge,
	CToolbarManager::eToolbar_Match,
	CToolbarManager::eToolbar_Select,
	CToolbarManager::eToolbar_ApplyRules,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Save,
	CToolbarManager::eToolbar_Print,
	CToolbarManager::eToolbar_DeleteNext,
	CToolbarManager::eToolbar_Previous,
	CToolbarManager::eToolbar_Next,
	CToolbarManager::eToolbar_MailboxType,
	CToolbarManager::eToolbar_ShowList,
	CToolbarManager::eToolbar_ShowItems,
	CToolbarManager::eToolbar_ShowPreview,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Space,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Last
};

// 3Pane Contacts
int sAllow_3PaneContacts[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Login,
	CToolbarManager::eToolbar_NewAdbk,
	CToolbarManager::eToolbar_OpenAdbk,
	CToolbarManager::eToolbar_DeleteAdbk,
	CToolbarManager::eToolbar_NewAddress,
	CToolbarManager::eToolbar_NewGroup,
	CToolbarManager::eToolbar_DeleteItem,
	CToolbarManager::eToolbar_SearchAddress,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_ShowList,
	CToolbarManager::eToolbar_ShowItems,
	CToolbarManager::eToolbar_ShowPreview,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Space,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Last
};

// 3Pane Calendar
int sAllow_3PaneCalendar[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Login,
	CToolbarManager::eToolbar_NewCalendar,
	CToolbarManager::eToolbar_NewEvent,
	CToolbarManager::eToolbar_NewToDo,
	CToolbarManager::eToolbar_ShowToday,
	CToolbarManager::eToolbar_GoToDate,
	CToolbarManager::eToolbar_CalendarDay,
	CToolbarManager::eToolbar_CalendarWorkWeek,
	CToolbarManager::eToolbar_CalendarWeek,
	CToolbarManager::eToolbar_CalendarMonth,
	CToolbarManager::eToolbar_CalendarYear,
	CToolbarManager::eToolbar_CalendarSummary,
	CToolbarManager::eToolbar_CalendarToDo,
	CToolbarManager::eToolbar_Check,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Print,
	CToolbarManager::eToolbar_ShowList,
	CToolbarManager::eToolbar_ShowItems,
	CToolbarManager::eToolbar_ShowPreview,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Space,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Last
};

// 1Pane Server
int sAllow_1PaneServer[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Login,
	CToolbarManager::eToolbar_Create,
	CToolbarManager::eToolbar_Cabinet,
	CToolbarManager::eToolbar_Check,
	CToolbarManager::eToolbar_Search,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Space,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Last
};

// 1Pane Mailbox
int sAllow_1PaneMailbox[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Flags,
	CToolbarManager::eToolbar_Reply,
	CToolbarManager::eToolbar_ReplyAll,
	CToolbarManager::eToolbar_Forward,
	CToolbarManager::eToolbar_Bounce,
	CToolbarManager::eToolbar_Reject,
	CToolbarManager::eToolbar_SendAgain,
	CToolbarManager::eToolbar_Digest,
	CToolbarManager::eToolbar_CopyTo,
	CToolbarManager::eToolbar_Delete,
	CToolbarManager::eToolbar_Expunge,
	CToolbarManager::eToolbar_Match,
	CToolbarManager::eToolbar_Select,
	CToolbarManager::eToolbar_ApplyRules,
	CToolbarManager::eToolbar_Check,
	CToolbarManager::eToolbar_Search,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Save,
	CToolbarManager::eToolbar_Print,
	CToolbarManager::eToolbar_DeleteNext,
	CToolbarManager::eToolbar_Previous,
	CToolbarManager::eToolbar_Next,
	CToolbarManager::eToolbar_MailboxType,
	CToolbarManager::eToolbar_ShowPreview,
	CToolbarManager::eToolbar_Space,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Last
};

// 1Pane SMTP
int sAllow_1PaneSMTP[] = 
{
	CToolbarManager::eToolbar_Hold,
	CToolbarManager::eToolbar_Priority,
	CToolbarManager::eToolbar_Delete,
	CToolbarManager::eToolbar_Expunge,
	CToolbarManager::eToolbar_Accounts,
	CToolbarManager::eToolbar_Enable,
	CToolbarManager::eToolbar_ShowPreview,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Space,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Last
};

// 1Pane Message
int sAllow_1PaneMessage[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Flags,
	CToolbarManager::eToolbar_Reply,
	CToolbarManager::eToolbar_ReplyAll,
	CToolbarManager::eToolbar_Forward,
	CToolbarManager::eToolbar_Bounce,
	CToolbarManager::eToolbar_Reject,
	CToolbarManager::eToolbar_SendAgain,
	CToolbarManager::eToolbar_CopyTo,
	CToolbarManager::eToolbar_Delete,
	CToolbarManager::eToolbar_Save,
	CToolbarManager::eToolbar_Print,
	CToolbarManager::eToolbar_DeleteNext,
	CToolbarManager::eToolbar_Previous,
	CToolbarManager::eToolbar_Next,
	CToolbarManager::eToolbar_CollapseHeader,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Space,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Last
};

// 1Pane Letter
int sAllow_1PaneLetter[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Save,
	CToolbarManager::eToolbar_Print,
	CToolbarManager::eToolbar_Attach,
	CToolbarManager::eToolbar_Spell,
	CToolbarManager::eToolbar_Receipt,
	CToolbarManager::eToolbar_DSN,
	CToolbarManager::eToolbar_Signature,
	CToolbarManager::eToolbar_Sign,
	CToolbarManager::eToolbar_Encrypt,
	CToolbarManager::eToolbar_Send,
	CToolbarManager::eToolbar_AddCC,
	CToolbarManager::eToolbar_AddBcc,
	CToolbarManager::eToolbar_AppendNow,
	CToolbarManager::eToolbar_CopyOriginal,
#if __dest_os == __linux_os
	CToolbarManager::eToolbar_ExternalEdit,
#endif
	CToolbarManager::eToolbar_CollapseHeader,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Space,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Last
};

// 1Pane Address Book Manager
int sAllow_1PaneAdbkMgr[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Login,
	CToolbarManager::eToolbar_NewAdbk,
	CToolbarManager::eToolbar_OpenAdbk,
	CToolbarManager::eToolbar_DeleteAdbk,
	CToolbarManager::eToolbar_SearchAddress,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Space,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Last
};

// 1Pane Address Book
int sAllow_1PaneAddress[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_NewAddress,
	CToolbarManager::eToolbar_NewGroup,
	CToolbarManager::eToolbar_EditItem,
	CToolbarManager::eToolbar_DeleteItem,
	CToolbarManager::eToolbar_ShowPreview,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Space,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Last
};

// 1Pane Calendar Store
int sAllow_1PaneCalendarStore[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Login,
	CToolbarManager::eToolbar_NewCalendar,
	CToolbarManager::eToolbar_Check,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Print,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Space,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Last
};

// 1Pane Calendar
int sAllow_1PaneCalendar[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_NewEvent,
	CToolbarManager::eToolbar_NewToDo,
	CToolbarManager::eToolbar_ShowToday,
	CToolbarManager::eToolbar_GoToDate,
	CToolbarManager::eToolbar_CalendarDay,
	CToolbarManager::eToolbar_CalendarWorkWeek,
	CToolbarManager::eToolbar_CalendarWeek,
	CToolbarManager::eToolbar_CalendarMonth,
	CToolbarManager::eToolbar_CalendarYear,
	CToolbarManager::eToolbar_CalendarSummary,
	CToolbarManager::eToolbar_CalendarToDo,
	CToolbarManager::eToolbar_Check,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Print,
	CToolbarManager::eToolbar_ShowPreview,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Space,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Last
};

// All allowed
int* sAllowedItems[] = 
{
	sAllow_3PaneMailbox,
	sAllow_3PaneContacts,
	sAllow_3PaneCalendar,
	sAllow_1PaneServer,
	sAllow_1PaneMailbox,
	sAllow_1PaneSMTP,
	sAllow_1PaneMessage,
	sAllow_1PaneLetter,
	sAllow_1PaneAdbkMgr,
	sAllow_1PaneAddress,
	sAllow_1PaneCalendarStore,
	sAllow_1PaneCalendar
};

// Default items

// 3Pane Mailbox
int sDefault_3PaneMailbox[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Login,
	CToolbarManager::eToolbar_Create,
	CToolbarManager::eToolbar_Cabinet,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Reply,
	CToolbarManager::eToolbar_Forward,
	CToolbarManager::eToolbar_CopyTo,
	CToolbarManager::eToolbar_Delete,
	CToolbarManager::eToolbar_Expunge,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Match,
	CToolbarManager::eToolbar_Select,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Check,
	CToolbarManager::eToolbar_Search,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Last
};

// 3Pane Contacts
int sDefault_3PaneContacts[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Login,
	CToolbarManager::eToolbar_NewAdbk,
	CToolbarManager::eToolbar_OpenAdbk,
	CToolbarManager::eToolbar_DeleteAdbk,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_NewAddress,
	CToolbarManager::eToolbar_NewGroup,
	CToolbarManager::eToolbar_DeleteItem,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_SearchAddress,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Last
};

// 3Pane Calendar
int sDefault_3PaneCalendar[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Login,
	CToolbarManager::eToolbar_NewCalendar,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_NewEvent,
	CToolbarManager::eToolbar_NewToDo,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_CalendarDay,
	CToolbarManager::eToolbar_CalendarWorkWeek,
	CToolbarManager::eToolbar_CalendarWeek,
	CToolbarManager::eToolbar_CalendarMonth,
	CToolbarManager::eToolbar_CalendarYear,
	CToolbarManager::eToolbar_CalendarSummary,
	CToolbarManager::eToolbar_CalendarToDo,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Check,
	CToolbarManager::eToolbar_ShowToday,
	CToolbarManager::eToolbar_GoToDate,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Last
};

// 1Pane Server
int sDefault_1PaneServer[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Login,
	CToolbarManager::eToolbar_Create,
	CToolbarManager::eToolbar_Cabinet,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Check,
	CToolbarManager::eToolbar_Search,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Last
};

// 1Pane Mailbox
int sDefault_1PaneMailbox[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Reply,
	CToolbarManager::eToolbar_Forward,
	CToolbarManager::eToolbar_CopyTo,
	CToolbarManager::eToolbar_Delete,
	CToolbarManager::eToolbar_Expunge,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Match,
	CToolbarManager::eToolbar_Select,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Check,
	CToolbarManager::eToolbar_Search,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_MailboxType,
	CToolbarManager::eToolbar_Last
};

// 1Pane SMTP
int sDefault_1PaneSMTP[] = 
{
	CToolbarManager::eToolbar_Hold,
	CToolbarManager::eToolbar_Priority,
	CToolbarManager::eToolbar_Delete,
	CToolbarManager::eToolbar_Expunge,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Accounts,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Enable,
	CToolbarManager::eToolbar_Last
};

// 1Pane Message
int sDefault_1PaneMessage[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Reply,
	CToolbarManager::eToolbar_ReplyAll,
	CToolbarManager::eToolbar_Forward,
	CToolbarManager::eToolbar_CopyTo,
	CToolbarManager::eToolbar_Delete,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Print,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Previous,
	CToolbarManager::eToolbar_Next,
	CToolbarManager::eToolbar_DeleteNext,
	CToolbarManager::eToolbar_Last
};

// 1Pane Letter
int sDefault_1PaneLetter[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Save,
	CToolbarManager::eToolbar_Attach,
	CToolbarManager::eToolbar_Spell,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_AddCC,
	CToolbarManager::eToolbar_AddBcc,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Sign,
	CToolbarManager::eToolbar_Encrypt,
	CToolbarManager::eToolbar_ExpandSpace,
	CToolbarManager::eToolbar_Send,
	CToolbarManager::eToolbar_Last
};

// 1Pane Address Book Manager
int sDefault_1PaneAdbkMgr[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Login,
	CToolbarManager::eToolbar_NewAdbk,
	CToolbarManager::eToolbar_OpenAdbk,
	CToolbarManager::eToolbar_DeleteAdbk,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_SearchAddress,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Last
};

// 1Pane Address Book
int sDefault_1PaneAddress[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_NewAddress,
	CToolbarManager::eToolbar_NewGroup,
	CToolbarManager::eToolbar_EditItem,
	CToolbarManager::eToolbar_DeleteItem,
	CToolbarManager::eToolbar_Last
};

// 1Pane Calendar Store
int sDefault_1PaneCalendarStore[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Login,
	CToolbarManager::eToolbar_NewCalendar,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Check,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Last
};

// 1Pane Calendar
int sDefault_1PaneCalendar[] = 
{
	CToolbarManager::eToolbar_Draft,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_NewEvent,
	CToolbarManager::eToolbar_NewToDo,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_CalendarDay,
	CToolbarManager::eToolbar_CalendarWorkWeek,
	CToolbarManager::eToolbar_CalendarWeek,
	CToolbarManager::eToolbar_CalendarMonth,
	CToolbarManager::eToolbar_CalendarYear,
	CToolbarManager::eToolbar_CalendarSummary,
	CToolbarManager::eToolbar_CalendarToDo,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Check,
	CToolbarManager::eToolbar_ShowToday,
	CToolbarManager::eToolbar_GoToDate,
	CToolbarManager::eToolbar_Separator,
	CToolbarManager::eToolbar_Details,
	CToolbarManager::eToolbar_Last
};

int* sDefaultItems[] = 
{
	sDefault_3PaneMailbox,
	sDefault_3PaneContacts,
	sDefault_3PaneCalendar,
	sDefault_1PaneServer,
	sDefault_1PaneMailbox,
	sDefault_1PaneSMTP,
	sDefault_1PaneMessage,
	sDefault_1PaneLetter,
	sDefault_1PaneAdbkMgr,
	sDefault_1PaneAddress,
	sDefault_1PaneCalendarStore,
	sDefault_1PaneCalendar
};

void CToolbarManager::InitToolbars()
{
	// Add all available toolbar items
	CToolbarItem::SToolbarItem* p = sItems;
	while(p->mType != CToolbarItem::eNone)
	{
		mAllToolbarCommands.push_back(new CToolbarItem(*p++));
	}
	
	// Add allowed items for 3-pane mailbox
	for(int i = e3PaneMailbox; i < eLastToolbar; i++)
	{
		for(int j = 0; sAllowedItems[i][j] != CToolbarManager::eToolbar_Last; j++)
			mAllowedCommands[i].push_back(mAllToolbarCommands.at(sAllowedItems[i][j]));
	}
	
	// Add default items for 3-pane mailbox
	for(int i = e3PaneMailbox; i < eLastToolbar; i++)
	{
		for(int j = 0; sDefaultItems[i][j] != CToolbarManager::eToolbar_Last; j++)
			mCurrentCommands[i].push_back(mAllToolbarCommands.at(sDefaultItems[i][j]));
	}
}

void CToolbarManager::AddItemAt(EToolbarType type, unsigned long allowed_index, unsigned long index, const cdstring& extra)
{
	if (allowed_index >= mAllowedCommands[type].size())
		return;

	if (index < mCurrentCommands[type].size())
		mCurrentCommands[type].insert(mCurrentCommands[type].begin() + index, mAllowedCommands[type][allowed_index]);
	else
	{
		mCurrentCommands[type].push_back(mAllowedCommands[type][allowed_index]);
		index = mCurrentCommands[type].size() - 1;
	}
	mCurrentCommands[type][index].SetExtraInfo(extra);
	
	// Sync with prefs
	if (CPreferences::sPrefs != NULL)
		CPreferences::sPrefs->ToolbarChanged();
}

void CToolbarManager::MoveItemAt(EToolbarType type, unsigned long from_index, unsigned long to_index)
{
	// Adjust to_index if after from_index
	CToolbarItem::CToolbarItemInfo move_item = mCurrentCommands[type][from_index];
	mCurrentCommands[type].erase(mCurrentCommands[type].begin() + from_index);
	mCurrentCommands[type].insert(mCurrentCommands[type].begin() + to_index, move_item);
	
	// Sync with prefs
	if (CPreferences::sPrefs != NULL)
		CPreferences::sPrefs->ToolbarChanged();
}

void CToolbarManager::RemoveItemAt(EToolbarType type, unsigned long index)
{
	mCurrentCommands[type].erase(mCurrentCommands[type].begin() + index);
	
	// Sync with prefs
	if (CPreferences::sPrefs != NULL)
		CPreferences::sPrefs->ToolbarChanged();
}

void CToolbarManager::ResetToolbar(EToolbarType type)
{
	// Clear all existing items
	mCurrentCommands[type].clear();

	// Add default items back in
	for(int j = 0; sDefaultItems[type][j] != CToolbarManager::eToolbar_Last; j++)
		mCurrentCommands[type].push_back(mAllToolbarCommands.at(sDefaultItems[type][j]));
	
	// Sync with prefs
	if (CPreferences::sPrefs != NULL)
		CPreferences::sPrefs->ToolbarChanged();
}

void CToolbarManager::PrefsChanged()
{
	// Change data
	cdstring temp = CPreferences::sPrefs->mToolbars.GetValue();;
	char_stream txt(temp.c_str_mod());
	SetInfo(txt, CPreferences::sPrefs->vers);

	// Listen to filter manager
	CPreferences::sPrefs->GetFilterManager()->Add_Listener(this);
	
	// Update all toolbars (will also update the rules names so no need to explicitly do that here)
	CToolbar::PrefsChanged();
}

// Respond to list changes
void CToolbarManager::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CFilterManager::eBroadcast_RulesChanged:
		// Sync with prefs after rules change
		if (CToolbar::RulesChanged() && (CPreferences::sPrefs != NULL))
			CPreferences::sPrefs->ToolbarChanged();
		break;
	default:
		CListener::ListenTo_Message(msg, param);
		break;
	}
}

const char* cToolbarTypeDescriptor[] =
{
	"3PaneMailbox",
	"3PaneContacts",
	"3PaneCalendar",
	"1PaneServer",
	"1PaneMailbox",
	"1PaneSMTP",
	"1PaneMessage",
	"1PaneLetter",
	"1PaneAdbkMgr",
	"1PaneAddressBook",
	"1PaneCalendarStore",
	"1PaneCalendar",
	NULL
};

const char* cToolbarItemsDescriptor[] = 
{
	"Toolbar:Separator",
	"Toolbar:Space",
	"Toolbar:ExpandSpace",

	"File:Draft",
	"Edit:Details",
	"Mailbox:Expunge",
	"Mailbox:Search",
	"Message:Reply",
	"Message:ReplyAll",
	"Message:Forward",
	"Message:Bounce",
	"Message:Reject",
	"Message:SendAgain",
	"Message:Digest",
	"Message:CopyTo",
	"Message:Delete",
	"Message:Header",
	"File:Save",
	"File:Print",

	"Window:ShowList",
	"Window:ShowItems",
	"Window:ShowPreview",

	"File:Connect",
	"File:Login",
	"Mailbox:Create",
	"Mailbox:Check",
	"Mailbox:Cabinet",

	"Mailbox:Match",
	"Mailbox:Select",
	"Mailbox:Type",
	"Mailbox:ApplyRules",

	"SMTP:Hold",
	"SMTP:Priority",
	"SMTP:Accounts",
	"SMTP:Enable",

	"Message:Flags",
	"Message:DeleteNext",
	"Message:Previous",
	"Message:Next",

	"Draft:Attach",
	"Draft:Spell",
	"Draft:Receipt",
	"Draft:DSN",
	"Draft:Signature",
	"Draft:Sign",
	"Draft:Encrypt",
	"Draft:Send",
	"Draft:AddCC",
	"Draft:AddBCC",
	"Draft:AppendNow",
	"Draft:CopyOriginal",
	"Draft:External",

	"Address:NewAdbk",
	"Address:OpenAdbk",
	"Address:DeleteAdbk",
	"Address:SearchAddress",

	"Address:NewAddress",
	"Address:NewGroup",
	"Address:EditItem",
	"Address:DeleteItem",
		
	"Calendar:NewCalendar",
	"Calendar:NewEvent",
	"Calendar:NewToDo",
	"Calendar:ShowToday",
	"Calendar:GoToDate",
	"Calendar:Day",
	"Calendar:WorkWeek",
	"Calendar:Week",
	"Calendar:Month",
	"Calendar:Year",
	"Calendar:Summary",
	"Calendar:ToDo",
		
	"Window:CollapseHeader",
		
	NULL
	
};

const cdstring& CToolbarManager::GetTitle(unsigned long item)
{
	return rsrc::GetIndexedString("Toolbar::Titles", item);
}

const cdstring& CToolbarManager::GetDescriptor(unsigned long item)
{
	return rsrc::GetIndexedString("Toolbar::Descriptors", item);
}

cdstring CToolbarManager::GetInfo(void) const
{
	// Make sure current set is saved if screen list
	const_cast<CToolbarManager*>(this)->MergeScreenToolbars();

	// Generate per-screen settings
	cdstring txt;
	txt += "(";

	for(CScreenToolbarList::const_iterator iter = mScreenToolbars.begin(); iter != mScreenToolbars.end(); iter++)
	{
		txt += "(";
		txt += (*iter).GetInfo();
		txt += ")";
	}

	txt += ")";

	return txt;
}

bool CToolbarManager::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Start with empty set
	mScreenToolbars.clear();

	// Must start with '('
	if (!txt.start_sexpression())
		return false;

	// Get each toolbar typer
	while(!txt.end_sexpression())
	{
		// Must start with '('
		if (!txt.start_sexpression())
			return false;
		
		CScreenToolbars temp;
		temp.SetInfo(txt, vers_prefs);
		mScreenToolbars.push_back(temp);

		// Must end with ')'
		if (!txt.end_sexpression())
			return false;
	}

	// Make sure current set is loaded
	LoadScreenToolbars();

	return true;
}

#if __dest_os == __linux_os
cdstring CToolbarManager::GetPopupMenuItems(unsigned long index)
{
	switch(index)
	{
	case ePopupMenu_Select:
		return cdstring("Seen | Unseen | Important | Unimportant | Answered | Unanswered | "
  			   			"Deleted | Undeleted | Draft | Not Draft %l | Matching | Non Matching %l | "
  			   			"Label1 | Label2 | Label3 | Label4 | Label5 | Label6 | Label7 | Label8");
	case ePopupMenu_Flags:
		return cdstring("Seen %b | Important %b | Answered %b | Deleted %b | Draft %b %l | "
  			   			"Label1 %b | Label2 %b | Label3 %b | Label4 %b | Label5 %b | Label6 %b | Label7 %b | Label8 %b");
	default:
		return cdstring::null_str;
	}
}
#endif

cdstring CToolbarManager::GetToolbarInfo(void) const
{
	cdstring txt;
	txt += "(";

	// Add items for each toolbar
	for(int i = e3PaneMailbox; i < eLastToolbar; i++)
	{
		cdstrvect items;
		items.push_back(cToolbarTypeDescriptor[i]);
		for(CToolbarItem::CToolbarPtrItems::const_iterator iter = mCurrentCommands[i].begin(); iter != mCurrentCommands[i].end(); iter++)
		{
			// Add the descriptor
			items.push_back(cToolbarItemsDescriptor[(*iter).GetItem()->GetTitleID()]);
			
			// Special case rules - have extra info
			switch((*iter).GetItem()->GetTitleID())
			{
			case eToolbar_ApplyRules:
			{
				// Get the actual name of the rule
				cdstring title;

				// The extra info is actually the uid encoded as a string
				unsigned long uid = ::strtoul((*iter).GetExtraInfo(), NULL, 10);
				
				const CFilterItem* filter = CPreferences::sPrefs->GetFilterManager()->GetManualFilter(uid);
				if (filter)
					title = filter->GetName();
				
				items.push_back(title);
				break;
			}
			default:;
			}
		}
		
		cdstring temp;
		temp.CreateSExpression(items);

		txt += temp;
	}
	
	txt += ")";
	
	return txt;
}

bool CToolbarManager::SetToolbarInfo(char_stream& txt)
{
	// Must start with '('
	if (!txt.start_sexpression())
		return false;

	// Get each toolbar typer
	while(!txt.end_sexpression())
	{
		cdstrvect items;
		cdstring::ParseSExpression(txt, items, false);
		
		if (items.size() == 0)
			continue;
		
		// First item is the toolbar type
		unsigned long toolbar_index = ::strindexfind(items.front(), cToolbarTypeDescriptor, eLastToolbar);
		
		// Ignore if unknown
		if (toolbar_index == eLastToolbar)
			continue;

		// Clear out this toolbar
		mCurrentCommands[toolbar_index].clear();
		
		// Now add new items
		for(cdstrvect::const_iterator iter = items.begin() + 1; iter != items.end(); iter++)
		{
			unsigned long item_index = ::strindexfind(*iter, cToolbarItemsDescriptor, eToolbar_Last);
			mCurrentCommands[toolbar_index].push_back(mAllToolbarCommands[item_index]);
			
			// Special case rules - have extra info
			switch(item_index)
			{
			case eToolbar_ApplyRules:
			{
				iter++;
				if (iter != items.end())
				{
					// Extra info is the filter's UID
					const CFilterItem* filter = CPreferences::sPrefs->GetFilterManager()->GetFilter(CFilterItem::eLocal, *iter);
					if (filter != NULL)
					{
						cdstring temp(filter->GetUID());
						mCurrentCommands[toolbar_index].back().SetExtraInfo(temp);
					}
				}
				else
					iter--;
				break;
			}
			default:;
			}
		}
	}

	return true;
}

void CToolbarManager::MergeScreenToolbars()
{
	CScreenToolbars current_data(GetToolbarInfo());

	// Look for exact match to current screen size
	CScreenToolbarList::iterator found = mScreenToolbars.end();
	for(CScreenToolbarList::iterator iter = mScreenToolbars.begin(); iter != mScreenToolbars.end(); iter++)
	{
		// Check for smaller width
		if ((*iter).GetWidth() < current_data.GetWidth())
			continue;

		// Check for equal width
		else if ((*iter).GetWidth() == current_data.GetWidth())
		{
			// Check for smaller height
			if ((*iter).GetHeight() < current_data.GetHeight())
				// Smaller height => mark as possible candidate
				continue;

			// Check for equal height
			else if ((*iter).GetHeight() == current_data.GetHeight())
			{
				// Equal width & height => exact match
				found = iter;
				break;
			}
			else
				// Larger height => gone past possible matches
				break;
		}
		else
			// Large width => gone past possible matches
			break;
	}

	// Replace data in the one found
	if (found != mScreenToolbars.end())
		*found = current_data;
	else
	{
		mScreenToolbars.push_back(current_data);
		std::sort(mScreenToolbars.begin(), mScreenToolbars.end());
	}
	
}

void CToolbarManager::LoadScreenToolbars()
{
	CScreenToolbars current_screen_size;

	// Find closest matching screen resolution
	CScreenToolbarList::const_iterator found = mScreenToolbars.end();
	for(CScreenToolbarList::const_iterator iter = mScreenToolbars.begin(); iter != mScreenToolbars.end(); iter++)
	{
		// Check for smaller width
		if ((*iter).GetWidth() < current_screen_size.GetWidth())
			// Smaller width => mark as possible candidate
			found = iter;

		// Check for equal width
		else if ((*iter).GetWidth() == current_screen_size.GetWidth())
		{
			// Check for smaller height
			if ((*iter).GetHeight() < current_screen_size.GetHeight())
				// Smaller height => mark as possible candidate
				found = iter;

			// Check for equal height
			else if ((*iter).GetHeight() == current_screen_size.GetHeight())
			{
				// Equal width & height => exact match
				found = iter;
				break;
			}
			else
				// Larger height => gone past possible matches
				break;
		}
		else
			// Large width => gone past possible matches
			break;
	}

	// Do not load anything if none found
	if (found == mScreenToolbars.end())
		return;
	
	// Load the data from the one found
	cdstring temp((*found).GetData());
	char_stream txt(temp.c_str_mod());
	SetToolbarInfo(txt);
}
