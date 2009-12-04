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


// Header for CCommands.h class

#ifndef __CCOMMANDS__MULBERRY__
#define __CCOMMANDS__MULBERRY__

// Commands
const	CommandT	cmd_AboutPlugins = 5999;

const	CommandT	cmd_HelpMulberryOnline = 6001;
const	CommandT	cmd_HelpOnlineFAQ = 6002;
const	CommandT	cmd_HelpCheckUpdates = 6003;
const	CommandT	cmd_HelpMulberrySupport = 6004;
const	CommandT	cmd_HelpBuyMulberry = 6005;
const	CommandT	cmd_HelpDetailedTooltips = 6006;


//const	CommandT	cmd_Preferences = 27;
const	CommandT	cmd_Disconnected = 1501;
const	CommandT	cmd_OpenSMTPQueues = 1502;
const	CommandT	cmd_Properties = 5001;
const	CommandT	cmd_ChangePassword = 5007;
const	CommandT	cmd_Status = 8000;
const	CommandT	cmd_SetDefaultSize = 8001;
const	CommandT	cmd_ResetDefaultWindow = 8002;
const	CommandT	cmd_ExpandWindow = 8003;
const	CommandT	cmd_ShowParts = 8004;
const	CommandT	cmd_ShowQuickSearch = 8006;

const	CommandT	cmd_EditMacros = 3320;

const	CommandT	cmd_SpeakSelection = 11000;

const	CommandT	cmd_SpellCheck = 5002;

// Mailbox Info Window
const	CommandT	cmd_CreateMailbox = 1000;
const	CommandT	cmd_RenameMailbox = 1001;
const	CommandT	cmd_PuntUnseen = 1002;
const	CommandT	cmd_DeleteMailbox = 1003;
const	CommandT	cmd_ExpungeMailbox = 1004;
const	CommandT	cmd_SynchroniseMailbox = 1005;
const	CommandT	cmd_ClearDisconnectMailbox = 1006;
const	CommandT	cmd_Hierarchy = 1007;
const	CommandT	cmd_Cabinets = 1008;
const	CommandT	cmd_SubscribeMailbox = 1009;
const	CommandT	cmd_UnsubscribeMailbox = 1010;
const	CommandT	cmd_CheckMail = 1011;
const	CommandT	cmd_AutoCheckMail = 1012;
const	CommandT	cmd_SortMailbox = 1013;
const	CommandT	cmd_GotoMessage = 1014;
const	CommandT	cmd_CacheAll = 1015;
const	CommandT	cmd_SearchMailbox = 1016;
const	CommandT	cmd_SearchAgainMailbox = 1017;
const	CommandT	cmd_NextSearchMailbox = 1018;
const	CommandT	cmd_RulesMailbox = 1019;
const	CommandT	cmd_ApplyRuleMailbox = 1020;
const	CommandT	cmd_MakeRuleMailbox = 1021;
const	CommandT	cmd_UpdateMatch = 1022;

const	CommandT	cmd_NewHierarchy = 1200;
const	CommandT	cmd_EditHierarchy = 1201;
const	CommandT	cmd_DeleteHierarchy = 1202;
const	CommandT	cmd_ResetHierarchy = 1203;
const	CommandT	cmd_RefreshHierarchy = 1204;

const	CommandT	cmd_NewCabinet = 1300;
const	CommandT	cmd_EditCabinet = 1301;
const	CommandT	cmd_DeleteCabinet = 1302;
const	CommandT	cmd_AddWildcardItem = 1303;
const	CommandT	cmd_EditWildcardItem = 1304;
const	CommandT	cmd_RemoveItem = 1305;

const	CommandT	cmd_SortMailboxTo = 1100;
const	CommandT	cmd_SortMailboxFrom = 1101;
const	CommandT	cmd_SortMailboxReplyTo = 1102;
const	CommandT	cmd_SortMailboxSender = 1103;
const	CommandT	cmd_SortMailboxCC = 1104;
const	CommandT	cmd_SortMailboxSubject = 1105;
const	CommandT	cmd_SortMailboxThread = 1106;
const	CommandT	cmd_SortMailboxDateSent = 1107;
const	CommandT	cmd_SortMailboxDateReceived = 1108;
const	CommandT	cmd_SortMailboxSize = 1109;
const	CommandT	cmd_SortMailboxFlags = 1110;
const	CommandT	cmd_SortMailboxNumber = 1111;
const	CommandT	cmd_SortMailboxSmart = 1112;
const	CommandT	cmd_SortMailboxAttachment = 1113;
const	CommandT	cmd_SortMailboxParts = 1114;
const	CommandT	cmd_SortMailboxMatch = 1115;
const	CommandT	cmd_SortMailboxDisconnected = 1116;

const	CommandT	cmd_OpenMessage = 1500;

const	CommandT	cmd_ThreadSelectAll = 1600;
const	CommandT	cmd_ThreadMarkSeen = 1601;
const	CommandT	cmd_ThreadMarkImportant = 1602;
const	CommandT	cmd_ThreadMarkDeleted = 1603;
const	CommandT	cmd_ThreadSkip = 1604;

const	CommandT	cmd_MatchUnseen = 1650;
const	CommandT	cmd_MatchImportant = 1651;
const	CommandT	cmd_MatchDeleted = 1652;
const	CommandT	cmd_MatchSentToday = 1653;
const	CommandT	cmd_MatchFrom = 1654;
const	CommandT	cmd_MatchSubject = 1655;
const	CommandT	cmd_MatchDate = 1656;

// Rules Manager window
const	CommandT	cmd_NewRules = 1200;
const	CommandT	cmd_EditRules = 1201;
const	CommandT	cmd_DeleteRules = 1202;

// Message Window
const	CommandT	cmd_ReadPrevMessage = 2000;
const	CommandT	cmd_ReadNextMessage = 2001;
const	CommandT	cmd_DeleteReadNextMessage = 2002;
const	CommandT	cmd_ReplyMessage = 2003;
const	CommandT	cmd_ReplyToSenderMessage = 2004;
const	CommandT	cmd_ReplyToFromMessage = 2005;
const	CommandT	cmd_ReplyToAllMessage = 2006;
const	CommandT	cmd_ForwardMessage = 2007;
const	CommandT	cmd_BounceMessage = 2008;
const	CommandT	cmd_RejectMessage = 2021;
const	CommandT	cmd_CopyToMessage = 2009;
const	CommandT	cmd_FlagsMessage = 2010;
const	CommandT	cmd_ExtractPartsMessage = 2011;
const	CommandT	cmd_ViewPartsMessage = 2020;
const	CommandT	cmd_DecodeEmbeddedMessage = 2012;
const	CommandT	cmd_DeleteMessage = 2013;
const	CommandT	cmd_SendAgainMessage = 2014;
const	CommandT	cmd_HeaderToggleMessage = 2015;
const	CommandT	cmd_CopyReadNextMessage = 2016;
const	CommandT	cmd_CreateDigestMessage = 2017;
const	CommandT	cmd_CopyNowMessage = 2018;
const	CommandT	cmd_PartsToggleMessage = 2019;

const	CommandT	cmd_DecodeBinHexMessage = 2030;
const	CommandT	cmd_DecodeUUMessage = 2031;

const	CommandT	cmd_VerifyDecrypt = 2040;

const	CommandT	cmd_MessageCopyExplicit = 2050;
const	CommandT	cmd_MessageMoveExplicit = 2051;

const	CommandT	cmd_FlagsSeen = 2100;
const	CommandT	cmd_FlagsImportant = 2101;
const	CommandT	cmd_FlagsAnswered = 2102;
const	CommandT	cmd_FlagsDeleted = 2103;
const	CommandT	cmd_FlagsDraft = 2104;

const	CommandT	cmd_FlagsLabel1 = 2200;
const	CommandT	cmd_FlagsLabel2 = 2201;
const	CommandT	cmd_FlagsLabel3 = 2202;
const	CommandT	cmd_FlagsLabel4 = 2203;
const	CommandT	cmd_FlagsLabel5 = 2204;
const	CommandT	cmd_FlagsLabel6 = 2205;
const	CommandT	cmd_FlagsLabel7 = 2206;
const	CommandT	cmd_FlagsLabel8 = 2207;

// Letter Window
const	CommandT	cmd_IncludeOriginal = 3000;
const	CommandT	cmd_AttachFile = 3001;
const	CommandT	cmd_Send = 3002;
const	CommandT	cmd_PasteQuote = 3003;
const	CommandT	cmd_NewLetter = 3004;
const	CommandT	cmd_OpenLetter = 3005;
const	CommandT	cmd_NewTextPart = 3006;
const	CommandT	cmd_MultipartChoice = 3007;
const	CommandT	cmd_FileImport = 3008;
const	CommandT	cmd_AppendToMailbox = 3009;
const	CommandT	cmd_AppendNow = 3010;
const	CommandT	cmd_InsertSignature = 3011;
const	CommandT	cmd_SignDraft = 3012;
const	CommandT	cmd_EncryptDraft = 3013;
const	CommandT	cmd_DraftNotifications = 3014;
const	CommandT	cmd_FileExport = 3015;
const	CommandT	cmd_CurrentTextPart = 3016;
const	CommandT	cmd_AddCC = 3017;
const	CommandT	cmd_AddBcc = 3018;
const	CommandT	cmd_CopyOriginal = 3019;

const	CommandT	cmd_DSNSuccess = 3050;
const	CommandT	cmd_DSNFailure = 3051;
const	CommandT	cmd_DSNDelay = 3052;
const	CommandT	cmd_DSNFull = 3053;
const	CommandT	cmd_MDNRead = 3060;

const	CommandT	cmd_MultipartMixed = 3100;
const	CommandT	cmd_MultipartParallel = 3101;
const	CommandT	cmd_MultipartDigest = 3102;
const	CommandT	cmd_MultipartAlternative = 3103;

const 	CommandT	cmd_NewPlainTextPart = 3200;
const 	CommandT	cmd_NewEnrichedTextPart = 3201;
const 	CommandT	cmd_NewHTMLTextPart = 3202;

const 	CommandT	cmd_CurrentTextPlain = 3210;
const 	CommandT	cmd_CurrentTextPlainWrapped = 3211;
const 	CommandT	cmd_CurrentTextEnriched = 3212;
const 	CommandT	cmd_CurrentTextHTML = 3213;

// Formatted Text Display
const	CommandT	cmd_Font = 3033;

const 	CommandT	cmd_Align = 3034;
const 	CommandT	cmd_Color = 3035;
const 	CommandT	cmd_Style = 3036;
const	CommandT	cmd_Size = 3037;

const 	CommandT	cmd_Red = 2500;
const 	CommandT	cmd_Blue = 2501;
const 	CommandT	cmd_Green = 2502;
const 	CommandT	cmd_Yellow = 2503;
const 	CommandT	cmd_Cyan = 2504;
const 	CommandT	cmd_Magenta = 2505;
const 	CommandT	cmd_Black = 2506;
const 	CommandT	cmd_White = 2507;
const 	CommandT	cmd_Mulberry = 2508;
const 	CommandT	cmd_ColorOther = 2509;

// Text Display
const	CommandT	cmd_FindText = 3300;
const	CommandT	cmd_FindNextText = 3301;
const	CommandT	cmd_FindNextBackText = 3302;
const	CommandT	cmd_FindSelectionText = 3303;
const	CommandT	cmd_FindSelectionBackText = 3304;
const	CommandT	cmd_ReplaceText = 3305;
const	CommandT	cmd_ReplaceFindText = 3306;
const	CommandT	cmd_ReplaceFindBackText = 3307;
const	CommandT	cmd_ReplaceAllText = 3308;

const	CommandT	cmd_WrapLines = 3310;
const	CommandT	cmd_UnwrapLines = 3311;
const	CommandT	cmd_QuoteLines = 3312;
const	CommandT	cmd_UnquoteLines = 3313;
const	CommandT	cmd_RequoteLines = 3314;
const	CommandT	cmd_ShiftLeft = 3315;
const	CommandT	cmd_ShiftRight = 3316;

// 3-Pane Window
const	CommandT	cmd_ShowList = 8010;
const	CommandT	cmd_ShowItems = 8011;
const	CommandT	cmd_ShowPreview = 8012;

const	CommandT	cmd_WindowOptions = 8005;
const	CommandT	cmd_Geometry = 8020;
const	CommandT	cmd_3Pane = 8021;
const	CommandT	cmd_1Pane = 8022;
const	CommandT	cmd_GeometryS__M_P = 8023;
const	CommandT	cmd_GeometryS_P__M = 8024;
const	CommandT	cmd_GeometryS__M__P = 8025;
const	CommandT	cmd_GeometryM_P__S = 8026;
const	CommandT	cmd_GeometryS__P__M = 8027;
const	CommandT	cmd_GeometryS_M__P = 8028;
const	CommandT	cmd_GeometryS_M_P = 8029;
const	CommandT	cmd_GeometryS_M_P_ = 8030;

// Toolbars
const	CommandT	cmd_Toolbar = 8040;
const	CommandT	cmd_ToolbarButtons = 8041;
const	CommandT	cmd_ToolbarSmallIcons = 8043;
const	CommandT	cmd_ToolbarShowIcons = 8044;
const	CommandT	cmd_ToolbarShowCaptions = 8045;
const	CommandT	cmd_ToolbarShowBoth = 8046;

// Layout Toolbar
const	CommandT	cmd_ToolbarShowList = 8110;
const	CommandT	cmd_ToolbarShowItems = 8111;
const	CommandT	cmd_ToolbarShowPreview = 8112;

const	CommandT	cmd_ToolbarZoomList = 8115;
const	CommandT	cmd_ToolbarZoomItems = 8116;
const	CommandT	cmd_ToolbarZoomPreview = 8117;

// Common toolbar
const	CommandT	cmd_ToolbarNewLetter = 8200;
const	CommandT	cmd_ToolbarNewLetterOption = 8201;
const	CommandT	cmd_ToolbarDetailsBtn = 8202;
const	CommandT	cmd_ToolbarMailboxExpungeBtn = 8203;
const	CommandT	cmd_ToolbarSearchMailboxBtn = 8204;
const	CommandT	cmd_ToolbarMessageReplyBtn = 8205;
const	CommandT	cmd_ToolbarMessageReplyOptionBtn = 8206;
const	CommandT	cmd_ToolbarMessageReplyAllBtn = 8207;
const	CommandT	cmd_ToolbarMessageForwardBtn = 8208;
const	CommandT	cmd_ToolbarMessageForwardOptionBtn = 8209;
const	CommandT	cmd_ToolbarMessageBounceBtn = 8210;
const	CommandT	cmd_ToolbarMessageRejectBtn = 8211;
const	CommandT	cmd_ToolbarMessageSendAgainBtn = 8212;
const	CommandT	cmd_ToolbarMessageDigestBtn = 8213;
const	CommandT	cmd_ToolbarMessageCopyBtn = 8214;
const	CommandT	cmd_ToolbarMessageCopyOptionBtn = 8215;
const	CommandT	cmd_ToolbarMessageCopyPopup = 8216;
const	CommandT	cmd_ToolbarMessageCopyOptionPopup = 8217;
const	CommandT	cmd_ToolbarMessageDeleteBtn = 8218;
const	CommandT	cmd_ToolbarMessageHeaderBtn = 8219;
const	CommandT	cmd_ToolbarFileSaveBtn = 8220;
const	CommandT	cmd_ToolbarMessagePrintBtn = 8221;
const	CommandT	cmd_ToolbarCollapseHeader = 8222;

// Server toolbar
const	CommandT	cmd_ToolbarServerConnectBtn = 8230;
const	CommandT	cmd_ToolbarServerLoginBtn = 8231;
const	CommandT	cmd_ToolbarCreateMailboxBtn = 8232;
const	CommandT	cmd_ToolbarCheckMailboxBtn = 8233;
const	CommandT	cmd_ToolbarCabinetPopup = 8234;

// Mailbox toolbar
const	CommandT	cmd_ToolbarMailboxMatchBtn = 8240;
const	CommandT	cmd_ToolbarMailboxMatchOptionBtn = 8241;
const	CommandT	cmd_ToolbarMailboxSelectPopup = 8242;
const	CommandT	cmd_ToolbarMailboxApplyRules = 8243;

// SMTP toolbar
const	CommandT	cmd_ToolbarSMTPHoldBtn = 8250;
const	CommandT	cmd_ToolbarSMTPPriorityBtn = 8251;
const	CommandT	cmd_ToolbarSMTPAccountsPopup = 8252;
const	CommandT	cmd_ToolbarSMTPEnableBtn = 8253;

// Message toolbar
const	CommandT	cmd_ToolbarMessageFlagsBtn = 8260;
const	CommandT	cmd_ToolbarMessageDeleteNextBtn = 8261;
const	CommandT	cmd_ToolbarMessagePreviousBtn = 8262;
const	CommandT	cmd_ToolbarMessageNextBtn = 8263;

// Letter toolbar
const	CommandT	cmd_ToolbarLetterAttachBtn = 8270;
const	CommandT	cmd_ToolbarLetterSpellBtn = 8271;
const	CommandT	cmd_ToolbarLetterReceiptBtn = 8272;
const	CommandT	cmd_ToolbarLetterDSNBtn = 8273;
const	CommandT	cmd_ToolbarLetterSignatureBtn = 8274;
const	CommandT	cmd_ToolbarLetterSignPopup = 8275;
const	CommandT	cmd_ToolbarLetterEncryptPopup = 8276;
const	CommandT	cmd_ToolbarLetterSendBtn = 8277;
const	CommandT	cmd_ToolbarLetterAddCCBtn = 8278;
const	CommandT	cmd_ToolbarLetterAddBccBtn = 8279;
const	CommandT	cmd_ToolbarLetterAppendNowBtn = 8280;
const	CommandT	cmd_ToolbarLetterAppendNowOptionBtn = 8281;
const	CommandT	cmd_ToolbarLetterCopyOriginalBtn = 8282;

// Address Book Manager toolbar
const	CommandT	cmd_ToolbarAdbkMgrNewBtn = 8285;
const	CommandT	cmd_ToolbarAdbkMgrOpenBtn = 8286;
const	CommandT	cmd_ToolbarAdbkMgrDeleteBtn = 8287;
const	CommandT	cmd_ToolbarAdbkMgrSearchBtn = 8288;

// Address Book toolbar
const	CommandT	cmd_ToolbarAddressBookAddressBtn = 8290;
const	CommandT	cmd_ToolbarAddressBookGroupBtn = 8291;
const	CommandT	cmd_ToolbarAddressBookEditBtn = 8292;
const	CommandT	cmd_ToolbarAddressBookDeleteBtn = 8293;

// Calendar toolbar

// Calendar store items
const	CommandT	cmd_ToolbarNewCalendarBtn = 8300;
const	CommandT	cmd_ToolbarRenameCalendarBtn = 8301;
const	CommandT	cmd_ToolbarRefreshCalendarBtn = 8302;
const	CommandT	cmd_ToolbarDeleteCalendarBtn = 8303;

// Calendar view tiems
const	CommandT	cmd_ToolbarNewEventBtn = 8350;
const	CommandT	cmd_ToolbarNewToDoBtn = 8351;
const	CommandT	cmd_ToolbarShowTodayBtn = 8352;
const	CommandT	cmd_ToolbarGoToDateBtn = 83053;
const	CommandT	cmd_ToolbarCalendarDayBtn = 8354;
const	CommandT	cmd_ToolbarCalendarWorkWeekBtn = 8355;
const	CommandT	cmd_ToolbarCalendarWeekBtn = 8356;
const	CommandT	cmd_ToolbarCalendarMonthBtn = 8357;
const	CommandT	cmd_ToolbarCalendarYearBtn = 8358;
const	CommandT	cmd_ToolbarCalendarSummaryBtn = 8359;
const	CommandT	cmd_ToolbarCalendarToDoBtn = 8360;

// Address Book Manager
const	CommandT	cmd_NewAddressBook = 9000;
const	CommandT	cmd_OpenAddressBook = 9001;
const	CommandT	cmd_RenameAddressBook = 9002;
const	CommandT	cmd_ImportAddressBook = 9003;
const	CommandT	cmd_ExportAddressBook = 9004;
const	CommandT	cmd_DeleteAddressBook = 9005;
const	CommandT	cmd_RefreshAddressBook = 9008;
const	CommandT	cmd_SearchAddressBook = 9009;
const	CommandT	cmd_AddressBookManager = 9010;
const	CommandT	cmd_SynchroniseAddressBook = 9012;
const	CommandT	cmd_ClearDisconnectAddressBook = 9013;
const	CommandT	cmd_ExpandAddress = 9014;
const	CommandT	cmd_CaptureAddress = 9015;

// Address Book Window
const	CommandT	cmd_NewAddress = 9100;
const	CommandT	cmd_EditAddress = 9101;
const	CommandT	cmd_DeleteAddress = 9102;
const	CommandT	cmd_NewGroup = 9103;
const	CommandT	cmd_EditGroup = 9104;
const	CommandT	cmd_DeleteGroup = 9105;

// Calendar Store Window
const	CommandT	cmd_CalendarManager = 9499;
const	CommandT	cmd_SubscribedCalendars = 9498;
const	CommandT	cmd_NewCalendar = 9500;
const	CommandT	cmd_RenameCalendar = 9501;
const	CommandT	cmd_DeleteCalendar = 9502;
const	CommandT	cmd_RefreshCalendarList = 9503;
const	CommandT	cmd_SendCalendar = 9504;
const	CommandT	cmd_NewWebCalendar = 9510;
const	CommandT	cmd_RefreshWebCalendar = 9511;
const	CommandT	cmd_UploadWebCalendar = 9512;
const	CommandT	cmd_FreeBusyCalendar = 9513;
const	CommandT	cmd_CheckCalendar = 9514;

// Calendar Window
const	CommandT	cmd_NewEvent = 9600;
const	CommandT	cmd_NewToDo = 9601;
const	CommandT	cmd_EditCalendarItem = 9602;
const	CommandT	cmd_DuplicateCalendarItem = 9603;
const	CommandT	cmd_DeleteCalendarItem = 9604;
const	CommandT	cmd_CompletedToDo = 9605;
const	CommandT	cmd_GotoToday = 9606;
const	CommandT	cmd_GotoDate = 9607;
const	CommandT	cmd_InviteAttendees = 9608;
const	CommandT	cmd_ProcessInvitation = 9609;


// Messages


#endif
