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

const SMenuBuilder cFileMenuBuilder[] = 
{
	{"New Message %h n %k Meta-N", CCommand::cFileNewMessage},
	{"Open Mailbox... %h m %k Meta-Shift-O", CCommand::cFileOpenMailbox},
	{"Open Message %h o %k Meta-O", CCommand::cFileOpenMessage},
	{"Open Draft... %h d %l", CCommand::cFileOpenDraft},
	{"Import... %h i", CCommand::cFileImport},
	{"Export... %h e %l", CCommand::cFileExport},
	{"Close %h c %k Meta-W", CCommand::cFileClose},
	{"Save %h s %k Meta-S", CCommand::cFileSave},
	{"Save As... %h a", CCommand::cFileSaveAs},
	{"Revert %h r %l", CCommand::cFileRevert},
	{"Preferences... %h f %l", CCommand::cFilePreferences},
	{"Disconnect... %h t", CCommand::cFileDisconnect},
	{"Show Outgoing Queues %h g %l", CCommand::cFileShowOutgoingQueues},
	{"Page Setup... %h u", CCommand::cFilePageSetup},
	{"Print... %h p %k Meta-P %l", CCommand::cFilePrint},
	{"Quit %h q %k Meta-Q", CCommand::cFileExit},
	{NULL, NULL}
};

const SMenuBuilder cEditMenuBuilder[] = 
{
	{"Undo %h u %k Meta-Z", CCommand::cEditUndo},
	{"Redo %h r %k Meta-Shift-Z %l", CCommand::cEditRedo},
	{"Cut %h t %k Meta-X", CCommand::cEditCut},
	{"Copy %h c %k Meta-C", CCommand::cEditCopy},
	{"Paste %h p %k Meta-V", CCommand::cEditPaste},
	{"Paste as Quotation %h q %k Meta-'", CCommand::cEditPasteAsQuote},
	{"Delete %h d %k Del", CCommand::cEditDelete},
	{"Select All %h a %k Meta-A %l", CCommand::cEditSelectAll},
	{"Find... %h f %k Meta-F", CCommand::cEditFind},
	{"Find Next %h n %k Meta-G", CCommand::cEditFindNext},
	{"Find Selection %h s %k Meta-H", CCommand::cEditFindSel},
	{"Replace %h e %k Meta-B", CCommand::cEditReplace},
	{"Replace & Find Next %h x %k Meta-T", CCommand::cEditReplaceFindNext},
	{"Replace All %h l %l", CCommand::cEditReplaceAll},
	{"Text Macros... %h m %k %l", CCommand::cEditTextMacros},
	{"Details... %h i %k Meta-I %l", CCommand::cEditProperties},
	{"Change Password... %h h %l", CCommand::cEditChangePassword},
	{"Check Spelling  %h g %k Meta-Ctrl-L", CCommand::cEditCheckSpelling},
	{NULL, NULL}
};

const SMenuBuilder cMailboxMenuBuilder[] = 
{
	{"Create... %h c", CCommand::cMailboxCreate},
	{"Rename... %h r %l", CCommand::cMailboxRename},
	{"Mark Contents Seen %h a %k Meta-Ctrl-P", CCommand::cMailboxMarkContentsSeen},
	{"Delete Mailbox... %h d", CCommand::cMailboxDelete},
	{"Expunge %h e %k Meta-E", CCommand::cMailboxExpunge},
	{"Synchronise...", CCommand::cMailboxSynchronise},
	{"Clear Disconnected... %h i %l", CCommand::cMailboxClearDisconnected},
	{"Display Hierarchy %h h", CCommand::cMailboxDisplayHierarchy},
	{"Cabinets %h t %l", CCommand::cMailboxCabinets},
	{"Subscribe %h b %k Meta-=", CCommand::cMailboxSubscribe},
	{"Unsubscribe %h n %k Meta-- %l", CCommand::cMailboxUnsubscribe},
	{"Check Email %h m %k Meta-L", CCommand::cMailboxCheckMail},
	{"Check Favourites %h f %k Meta-Shift-L %l", CCommand::cMailboxCheckFavourites},
	{"Sort by %h y %l", CCommand::cMailboxSortBy},
	{"Goto Message... %h o %k Meta-Ctrl-G", CCommand::cMailboxGotoMessage},
	{"Load All Messages %h l", CCommand::cMailboxLoadAllMessages},
	{"Update Current Match %k Meta-U %l", CCommand::cMailboxUpdateMatch},
	{"Search Mailbox... %h s %k Meta-Y", CCommand::cMailboxSearch},
	{"Search Again %h g %k Meta-Shift-Y", CCommand::cMailboxSearchAgain},
	{"Next Search %h x %k Meta-Ctrl-Y %l", CCommand::cMailboxNextSearch},
	{"Rules... %h u", CCommand::cMailboxRules},
	{"Apply Rules %h p", CCommand::cMailboxApplyRules},
	{"Make Rule %h k", CCommand::cMailboxMakeRule},
	{NULL, NULL}
};

const SMenuBuilder cDisplayHierarchyMenuBuilder[] = 
{
	{"New Display Hierarchy... %h n", CCommand::cDispHNew},
	{"Edit Display Hierarchy... %h e", CCommand::cDispHEdit},
	{"Remove Display Hierarchy... %h r %l", CCommand::cDispHRemove},
	{"Reset All %h a %l", CCommand::cDispHReset},
	{"Refresh %h f", CCommand::cDispHRefresh},
	{NULL, NULL}
};

const SMenuBuilder cCabinetsMenuBuilder[] = 
{
	{"New Cabinet... %h n", CCommand::cCabNew},
	{"Edit Cabinet... %h e", CCommand::cCabEdit},
	{"Remove Cabinet... %h r %l", CCommand::cCabRemove},
	{"Add Wildcard Item... %h a", CCommand::cCabAddWild},
	{"Edit Wildcard Item %h w %l", CCommand::cCabEditWild},
	{"Remove Item %h i", CCommand::cCabRemoveItem},
	{NULL, NULL}
};

const SMenuBuilder cSortByMenuBuilder[] = 
{
	{"To %r %h t", CCommand::cSortTo},
	{"From %r %h f", CCommand::cSortFrom},
	{"Reply To %r %h r", CCommand::cSortReply},
	{"Sender %r %h e", CCommand::cSortSender},
	{"CC %r %h c", CCommand::cSortCC},
	{"Subject %r %h s", CCommand::cSortSubject},
	{"Thread %r %h h", CCommand::cSortThread},
	{"Date Sent %r %h d", CCommand::cSortDateSent},
	{"Date Received %r %h v", CCommand::cSortDateReceived},
	{"Size %r %h i", CCommand::cSortSize},
	{"Flags %r %h l", CCommand::cSortFlags},
	{"Number %r %h n", CCommand::cSortNumber},
	{"Smart Address %r %h a", CCommand::cSortSmartAddress},
	{"Attachments %r %h m", CCommand::cSortAttachments},
	{"Parts %r %h p", CCommand::cSortParts},
	{"Matching %r %h g", CCommand::cSortMatching},
	{"Disconnected %r %h c", CCommand::cSortDisconnected},
	{NULL, NULL}
};

const SMenuBuilder cApplyRulesMenuBuilder[] = 
{
	{"All %l", NULL},
	{NULL, NULL}
};

const SMenuBuilder cMessagesMenuBuilder[] = 
{
	{"Read Previous... %h p %k Meta-[", CCommand::cMessagesReadPrev},
	{"Read Next... %h n %k Meta-]", CCommand::cMessagesReadNext},
	{"Delete and Read Next... %h d %k Meta-/", CCommand::cMessagesDelAndReadNext},
	{"Copy and Read Next... %h c %k Meta-\\ %l", CCommand::cMessagesCopyAndReadNext},
	{"Reply %h r %k Meta-R", CCommand::cMessagesReply},
	{"Reply to Sender %h s", CCommand::cMessagesReplyToSender},
	{"Reply to From %h y", CCommand::cMessagesReplyToFrom},
	{"Reply to All %h a %k Meta-K", CCommand::cMessagesReplyToAll},
	{"Forward %h f %k Meta-J", CCommand::cMessagesForward},
	{"Bounce %h b %k Meta-Shift-J", CCommand::cMessagesBounce},
	{"Reject %h j", CCommand::cMessagesReject},
	{"Send Again %h n", CCommand::cMessagesSendAgain},
	{"Create Digest %h i %l", CCommand::cMessagesCreateDigest},
	{"Copy to Mailbox %h t", CCommand::cMessagesCopyTo},
	{"Copy Now %h n %k Meta-Shift-C %l", CCommand::cMessagesCopyNow},
	{"Flags %h g %l", CCommand::cMessagesFlags},
	{"View Parts %h e", CCommand::cMessagesViewParts},
	{"Extract Parts %h e %k Meta-;", CCommand::cMessagesExtractParts},
	{"Decode Embedded %h o", CCommand::cMessagesDecodeEmbedded},
	{"Verify/Decrypt %h v %l", CCommand::cMessagesVerifyDecrypt},
	{"Delete %h e %k Meta-D %l", CCommand::cMessagesDelete},
	{"Show Header %h h %k Meta-Ctrl-H", CCommand::cMessagesShowHeader},
	{NULL, NULL}
};

const SMenuBuilder cCopyToMenuBuilder[] = 
{
	{"Choose... %l", NULL},
	{NULL, NULL}
};

const SMenuBuilder cFlagsMenuBuilder[] = 
{
	{"Seen %b %h s %k Meta-Ctrl-1", CCommand::cFlagsSeen},
	{"Important %b %h i %k Meta-Ctrl-2", CCommand::cFlagsImportant},
	{"Answered %b %h a %k Meta-Ctrl-3", CCommand::cFlagsAnswered},
	{"Delete %b %h d %k Meta-Ctrl-4", CCommand::cFlagsDeleted},
	{"Draft %b %h r %k Meta-Ctrl-5 %l", CCommand::cFlagsDraft},
	{"Label1 %b %h r", CCommand::cFlagsLabel1},
	{"Label2 %b %h r", CCommand::cFlagsLabel2},
	{"Label3 %b %h r", CCommand::cFlagsLabel3},
	{"Label4 %b %h r", CCommand::cFlagsLabel4},
	{"Label5 %b %h r", CCommand::cFlagsLabel5},
	{"Label6 %b %h r", CCommand::cFlagsLabel6},
	{"Label7 %b %h r", CCommand::cFlagsLabel7},
	{"Label8 %b %h r", CCommand::cFlagsLabel8},
	{NULL, NULL}
};

const SMenuBuilder cFlagsMenuNoShortcutBuilder[] = 
{
	{"Seen %b %h s", CCommand::cFlagsSeen},
	{"Important %b %h i", CCommand::cFlagsImportant},
	{"Answered %b %h a", CCommand::cFlagsAnswered},
	{"Delete %b %h d", CCommand::cFlagsDeleted},
	{"Draft %b %h r %l", CCommand::cFlagsDraft},
	{"Label1 %b %h r", CCommand::cFlagsLabel1},
	{"Label2 %b %h r", CCommand::cFlagsLabel2},
	{"Label3 %b %h r", CCommand::cFlagsLabel3},
	{"Label4 %b %h r", CCommand::cFlagsLabel4},
	{"Label5 %b %h r", CCommand::cFlagsLabel5},
	{"Label6 %b %h r", CCommand::cFlagsLabel6},
	{"Label7 %b %h r", CCommand::cFlagsLabel7},
	{"Label8 %b %h r", CCommand::cFlagsLabel8},
	{NULL, NULL}
};

const SMenuBuilder cDecodeMenuBuilder[] = 
{
	{"BinHex %h B", CCommand::cDecodeBinHex},
	{"UU %h u ", CCommand::cDecodeUU},
	{NULL, NULL}
};

const SMenuBuilder cDraftMenuBuilder[] = 
{
	{"Quote Original Message%h i %k Meta-Shift-I", CCommand::cDraftIncludeOrig},
	{"Insert Signature %h e %k Meta-Shift-S %l", CCommand::cDraftInsertSig},
	{"Add Cc Address Field", CCommand::cDraftAddCc},
	{"Add Bcc Address Field %l", CCommand::cDraftAddBcc},
	{"Attach File... %h f", CCommand::cDraftAttachFile},
	{"New Text Part %h t", CCommand::cDraftNewTextPart},
	{"New Multipart %h m", CCommand::cDraftNewMultipart},
	{"Current Text Part %l", CCommand::cDraftCurrentTextPart},
	{"Send %h s %k Meta-M", CCommand::cDraftSend},
	{"Append To Mailbox %h p", CCommand::cDraftAppendTo},
	{"Append Now %h n %k Meta-Shift-M ", CCommand::cDraftAppendNow},
	{"Also Copy Original Message %l", CCommand::cDraftCopyOriginal},
	{"Sign %b %h g", CCommand::cDraftSign},
	{"Encrypt %b %h p %l", CCommand::cDraftEncrypt},
	{"Notifications %l", CCommand::cDraftNotifications},
	{"Wrap Lines", CCommand::cDraftWrap},
	{"Unwrap Lines", CCommand::cDraftUnwrap},
	{"Quote Lines", CCommand::cDraftQuote},
	{"Unquote Lines", CCommand::cDraftUnquote},
	{"Requote Lines %k Meta-Ctrl-'", CCommand::cDraftRequote},
	{"Shift Left", CCommand::cDraftShiftLeft},
	{"Shift Right %l", CCommand::cDraftShiftRight},
	{"Font", CCommand::cDraftFont},
	{"Size", CCommand::cDraftSize},
	{"Style", CCommand::cDraftStyle},
	{"Alignment", CCommand::cDraftAlignment},
	{"Colour", CCommand::cDraftColour},
	{NULL, NULL}
};

const SMenuBuilder cNewTextPartMenuBuilder[] = 
{
	{"Plain %h p", CCommand::cNewTextPlain},
	{"Enriched %h e", CCommand::cNewTextEnriched},
	{"HTML %h h ", CCommand::cNewTextHTML},
	{NULL, NULL}
};

const SMenuBuilder cNewMultipartMenuBuilder[] = 
{
	{"Mixed %h m", CCommand::cNewMultiMixed},
	{"Parallel %h p", CCommand::cNewMultiParallel},
	{"Digest %h d", CCommand::cNewMultiDigest},
	{"Alternative %h A", CCommand::cNewMultiAlt},
	{NULL, NULL}
};

const SMenuBuilder cCurrentTextPartMenuBuilder[] = 
{
	{"Plain %h p", CCommand::cCurrentTextPlain},
	{"Plain Wrapped %h w", CCommand::cCurrentTextPlainWrapped},
	{"Enriched %h e", CCommand::cCurrentTextEnriched},
	{"HTML %h h ", CCommand::cCurrentTextHTML},
	{NULL, NULL}
};

const SMenuBuilder cAppendToMenuBuilder[] = 
{
	{"Choose... %l", NULL},
	{NULL, NULL}
};

const SMenuBuilder cNotificationsMenuBuilder[] = 
{
	{"Return Receipt%b  %h r %l", CCommand::cNotifyReceipt},
	{"Delivery Success%b  %h s", CCommand::cNotifySuccess},
	{"Delivery Failure %b %h f", CCommand::cNotifyFailure},
	{"Delivery Delay %b %h d %l", CCommand::cNotifyDelay},
	{"Return Entire Message %b %h r", CCommand::cNotifyEntireMsg},
	{NULL, NULL}
};

const SMenuBuilder cStyleMenuBuilder[] = 
{
	{"Plain%b %h p", CCommand::cStylePlain},
	{"Bold %b %h b", CCommand::cStyleBold},
	{"Italic %b %h i", CCommand::cStyleItalic},
	{"Underline %b %h u", CCommand::cStyleUnderline},
	{NULL, NULL}
};

const SMenuBuilder cAlignmentMenuBuilder[] = 
{
	{"Align Left %b %h l", CCommand::cAlignLeft},
	{"Centered %b %h c", CCommand::cAlignCenter},
	{"Align Right %b %h r", CCommand::cAlignRight},
	//{"Justified %b %h j", CCommand::cAlignJustified},
	{NULL, NULL}
};

const SMenuBuilder cColourMenuBuilder[] = 
{
	{"Black %b %h p", CCommand::cColourBlack},
	{"Red %b %h p", CCommand::cColourRed},
	{"Green %b %h p", CCommand::cColourGreen},
	{"Blue %b %h p", CCommand::cColourBlue},
	{"Yellow %b %h p", CCommand::cColourYellow},
	{"Cyan %b %h p", CCommand::cColourCyan},
	{"Magenta %b %h p", CCommand::cColourMagenta},
	{"Mulberry %b %h p", CCommand::cColourMulberry},
	{"White %b %h p", CCommand::cColourWhite},
	{"Other... %b %h p", CCommand::cColourOther},
	{NULL, NULL}
};

const SMenuBuilder cAddressesMenuBuilder[] = 
{
	{"Address Book Manager %h a %l", CCommand::cAddressesAddBookMan},
	{"New Address Book... %h n", CCommand::cAddressesNew},
	{"Open Address Book... %h o", CCommand::cAddressesOpen},
	{"Rename Addressbook... %h r %l", CCommand::cAddressesRename},
	{"Import Addresses %h i", CCommand::cAddressesImport},
	{"Export Addresses %h e %l", CCommand::cAddressesExport},
	{"Delete Address Book... %h d %l", CCommand::cAddressesDelete},
	{"Login... %h l", CCommand::cAddressesLogin},
	{"Logout... %h u", CCommand::cAddressesLogout},
	{"Refresh List %h f %l", CCommand::cAddressesRefresh},
	{"Synchronise... %h y", CCommand::cAddressesSynchronise},
	{"Clear Disconnected... %h c %l", CCommand::cAddressesClearDisconnected},
	{"Expand Address... %h x %k Meta-Return", CCommand::cAddressesExpandAddresses},
	{"Capture Address %h p %k Meta-Shift-A %l", CCommand::cAddressesCaptureAddress},
	{"Search for Addresses... %h s", CCommand::cAddressSearch},
	{NULL, NULL}
};

const SMenuBuilder cCalendarMenuBuilder[] = 
{
	{"Calendar Manager %h m", CCommand::cCalendarManager},
	{"Subscribed Calendars... %h s %l", CCommand::cCalendarSubscribe},
	{"Create... %h c", CCommand::cCalendarCreate},
	{"Rename... %h r", CCommand::cCalendarRename},
	{"Check Calendars %h h", CCommand::cCalendarCheck},
	{"Refresh List %h l %l", CCommand::cCalendarRefresh},
	{"Delete Calendar... %h d %l", CCommand::cCalendarDelete},
	{"Add Web Calendar... %h a", CCommand::cCalendarAddWeb},
	{"Refresh Web Calendar %h w", CCommand::cCalendarRefreshWeb},
	{"Upload Web Calendar %h p %l", CCommand::cCalendarUploadWeb},
	{"New Event... %h n", CCommand::cCalendarNewEvent},
	{"New To Do... %h t", CCommand::cCalendarNewToDo},
	{"Edit Item... %h e", CCommand::cCalendarEditItem},
	{"Duplicate Item... %h u", CCommand::cCalendarDuplicateItem},
	{"Delete Item... %h i %l", CCommand::cCalendarDeleteItem},
	{"Goto Today %h g", CCommand::cCalendarGotoToday},
	{"Goto Date... %h o", CCommand::cCalendarGotoDate},
	{NULL, NULL}
};

const SMenuBuilder cWindowsMenuBuilder[] = 
{
	{"Cascade %h c", CCommand::cWindowsCascade},
	{"Tile Horizontally %h h", CCommand::cWindowsTileHor},
	{"Tile Vertically %h t %l", CCommand::cWindowsTileVert},
	{"Save Default Window %h w", CCommand::cWindowsSaveDefault},
	{"Reset to Default %h r %l", CCommand::cWindowsResetToDefault},
	{"Expand Header %h e %k Meta-Ctrl-A", CCommand::cWindowsExpandHeader},
	{"Show Parts %h p %k Meta-Ctrl-B", CCommand::cWindowsShowParts},
	{"Show Quick Search %h q %k Shift-Ctrl-S %l", CCommand::cWindowsShowQuickSearch},
	{"Toolbars %l", CCommand::cWindowsToolbars},
	{"Layout", CCommand::cWindowsLayout},
	{"Show List %h l %b", CCommand::cWindowsShowList},
	{"Show Items %h i %b", CCommand::cWindowsShowItems},
	{"Show Preview %h v %b", CCommand::cWindowsShowPreview},
	{"Options... %h o %l", CCommand::cWindowsOptions},
	{"Status %b", CCommand::cWindowsStatus},
	{NULL, NULL}
};

const SMenuBuilder cToolbarsMenuBuilder[] = 
{
	{"Show Toolbar %h o %b %l", CCommand::cToolbarsButtons},
	{"Small Icons %h s %b %l", CCommand::cToolbarsSmall},
	{"Show Icons %h i %b", CCommand::cToolbarsIcons},
	{"Show Text %h t %b", CCommand::cToolbarsCaptions},
	{"Show Icons & Text %h c %b", CCommand::cToolbarsBoth},
	{NULL, NULL}
};

const SMenuBuilder cLayoutMenuBuilder[] = 
{
	{"Use 3-pane Window %h 3 %b", CCommand::cLayout3Pane},
	{"Use Separate Windows %h s %b %l", CCommand::cLayoutSingle},
	{"List Vertical %h l %b", CCommand::cLayoutListVert},
	{"List Horizontal %h t %b", CCommand::cLayoutListHoriz},
	{"Items Vertical %h i %b", CCommand::cLayoutItemsVert},
	{"Items Horizontal %h e %b", CCommand::cLayoutItemsHoriz},
	{"Preview Vertical %h p %b", CCommand::cLayoutPreviewVert},
	{"Preview Horizontal %h r %b", CCommand::cLayoutPreviewHoriz},
	{"All Vertical %h v %b", CCommand::cLayoutAllVert},
	{"All Horizontal %h h %b", CCommand::cLayoutAllHoriz},
	{NULL, NULL}
};

const SMenuBuilder cHelpMenuBuilder[] = 
{
	{"Help Topics %h h", CCommand::cHelpHelpTopics},
	{"About Mulberry... %h a", CCommand::cHelpAboutMul},
	{"About Plugins... %h p %l", CCommand::cHelpAboutPlugins},
	{"Detailed Tooltips %h d %b %l", CCommand::cHelpDetailedTooltips},
	{"Mulberry Online %h o", CCommand::cHelpMulberryOnline},
	{"Online FAQ %h q", CCommand::cHelpMulberryFAQ},
	{"Check for Updates %h u", CCommand::cHelpMulberryUpdates},
	{"Mulberry Support %h s", CCommand::cHelpMulberrySupport},
	{NULL, NULL}
};
