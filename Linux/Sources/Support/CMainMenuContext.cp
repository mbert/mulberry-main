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

const SMenuBuilder cEditContext[] = 
{
	{"Undo", CCommand::cEditUndo},
	{"Redo %l", CCommand::cEditRedo},
	{"Cut", CCommand::cEditCut},
	{"Copy", CCommand::cEditCopy},
	{"Paste", CCommand::cEditPaste},
	{"Delete %l", CCommand::cEditDelete},
	{"Select All %l", CCommand::cEditSelectAll},
	{"Capture Address", CCommand::cAddressesCaptureAddress},
	{NULL, NULL}
};

const SMenuBuilder cServerTableContext[] = 
{
	{"Create...", CCommand::cMailboxCreate},
	{"Open Mailbox...", CCommand::cFileOpenMailbox},
	{"Rename...%l", CCommand::cMailboxRename},
	{"Display Hierarchy", CCommand::cMailboxDisplayHierarchy},
	{"Cabinets %l", CCommand::cMailboxCabinets},
	{"Subscribe", CCommand::cMailboxSubscribe},
	{"Unsubscribe %l", CCommand::cMailboxUnsubscribe},
	{"Check Email", CCommand::cMailboxCheckMail},
	{"Check Favourites %l", CCommand::cMailboxCheckFavourites},
	{"Mark Contents Seen", CCommand::cMailboxMarkContentsSeen},
	{"Delete Mailbox... %l", CCommand::cMailboxDelete},
	{"Search Mailbox... %l", CCommand::cMailboxSearch},
	{"Details...", CCommand::cEditProperties},
	{NULL, NULL}
};

const SMenuBuilder cMailboxTableContext[] = 
{
	{"Open Message", CCommand::cFileOpenMessage},
	{"Save", CCommand::cFileSave},
	{"Print... %l", CCommand::cFilePrint},
	{"Search Mailbox...", CCommand::cMailboxSearch},
	{"Search Again", CCommand::cMailboxSearchAgain},
	{"Next Search %l", CCommand::cMailboxNextSearch},
	{"Copy to Mailbox %l", CCommand::cMessagesCopyTo},
	{"Check Email", CCommand::cMailboxCheckMail},
	{"Check Favourites %l", CCommand::cMailboxCheckFavourites},
	{"Reply", CCommand::cMessagesReply},
	{"Forward", CCommand::cMessagesForward},
	{"Bounce", CCommand::cMessagesBounce},
	{"Send Again", CCommand::cMessagesSendAgain},
	{"Create Digest %l", CCommand::cMessagesCreateDigest},
	{"Synchronise... %l", CCommand::cMailboxSynchronise},
	{"Flags", CCommand::cMessagesFlags},
	{"Delete", CCommand::cMessagesDelete},
	{NULL, NULL}
};

const SMenuBuilder cMessageAddrContext[] = 
{
	{"Copy %l", CCommand::cEditCopy},
	{"Select All %l", CCommand::cEditSelectAll},
	{"Save", CCommand::cFileSave},
	{"Print... %l", CCommand::cFilePrint},
	{"Copy to Mailbox", CCommand::cMessagesCopyTo},
	{"Copy Now %l", CCommand::cMessagesCopyNow},
	{"Reply", CCommand::cMessagesReply},
	{"Forward", CCommand::cMessagesForward},
	{"Bounce", CCommand::cMessagesBounce},
	{"Send Again %l", CCommand::cMessagesSendAgain},
	{"Flags %l", CCommand::cMessagesFlags},
	{"Capture Address %l", CCommand::cAddressesCaptureAddress},
	{"Delete", CCommand::cMessagesDelete},
	{NULL, NULL}
};

const SMenuBuilder cMessageSubjContext[] = 
{
	{"Save", CCommand::cFileSave},
	{"Print... %l", CCommand::cFilePrint},
	{"Copy %l", CCommand::cEditCopy},
	{"Select All %l", CCommand::cEditSelectAll},
	{"Copy to Mailbox", CCommand::cMessagesCopyTo},
	{"Copy Now %l", CCommand::cMessagesCopyNow},
	{"Reply", CCommand::cMessagesReply},
	{"Forward", CCommand::cMessagesForward},
	{"Bounce", CCommand::cMessagesBounce},
	{"Send Again %l", CCommand::cMessagesSendAgain},
	{"Flags %l", CCommand::cMessagesFlags},
	{"Delete", CCommand::cMessagesDelete},
	{NULL, NULL}
};

const SMenuBuilder cMessagePartsContext[] = 
{
	{"View Parts", CCommand::cMessagesViewParts},
	{"Extract Parts %l", CCommand::cMessagesExtractParts},
	{"Details...", CCommand::cEditProperties},
	{NULL, NULL}
};

const SMenuBuilder cMessageTextContext[] = 
{
	{"Copy %l", CCommand::cEditCopy},
	{"Select All %l", CCommand::cEditSelectAll},
	{"Print... %l", CCommand::cFilePrint},
	{"Find...", CCommand::cEditFind},
	{"Find Next", CCommand::cEditFindNext},
	{"Find Selection %l", CCommand::cEditFindSel},
	{"Check Spelling %l", CCommand::cEditCheckSpelling},
	{"View Current Part %l", CCommand::cMessagesViewParts},
	{"Capture Address", CCommand::cAddressesCaptureAddress},
	{NULL, NULL}
};

const SMenuBuilder cMessageTextURLContext[] = 
{
	{"Open Link", CCommand::cEditOpenLink},
	{"Copy Link to Clipboard %l", CCommand::cEditCopyLink},
	{"Select All %l", CCommand::cEditSelectAll},
	{"Print... %l", CCommand::cFilePrint},
	{"Find...", CCommand::cEditFind},
	{"Find Next", CCommand::cEditFindNext},
	{"Find Selection %l", CCommand::cEditFindSel},
	{"Check Spelling %l", CCommand::cEditCheckSpelling},
	{"View Current Part %l", CCommand::cMessagesViewParts},
	{"Capture Address", CCommand::cAddressesCaptureAddress},
	{NULL, NULL}
};

const SMenuBuilder cLetterAddrContext[] = 
{
	{"Undo", CCommand::cEditUndo},
	{"Redo %l", CCommand::cEditRedo},
	{"Cut", CCommand::cEditCut},
	{"Copy", CCommand::cEditCopy},
	{"Paste", CCommand::cEditPaste},
	{"Delete %l", CCommand::cEditDelete},
	{"Select All %l", CCommand::cEditSelectAll},
	{"Expand Addresses...", CCommand::cAddressesExpandAddresses},
	{"Capture Address", CCommand::cAddressesCaptureAddress},
	{NULL, NULL}
};

const SMenuBuilder cLetterPartsContext[] = 
{
	{"Attach File...", CCommand::cDraftAttachFile},
	{"New Text Part", CCommand::cDraftNewTextPart},
	{"New Multipart %l", CCommand::cDraftNewMultipart},
	{"Delete %l", CCommand::cEditDelete},
	{"Details...", CCommand::cEditProperties},
	{NULL, NULL}
};

const SMenuBuilder cLetterTextContext[] = 
{
	{"Undo", CCommand::cEditUndo},
	{"Redo %l", CCommand::cEditRedo},
	{"Cut", CCommand::cEditCut},
	{"Copy", CCommand::cEditCopy},
	{"Paste", CCommand::cEditPaste},
	{"Paste as Quotation", CCommand::cEditPasteAsQuote},
	{"Delete %l", CCommand::cEditDelete},
	{"Select All %l", CCommand::cEditSelectAll},
	{"Print... %l", CCommand::cFilePrint},
	{"Find...", CCommand::cEditFind},
	{"Find Next", CCommand::cEditFindNext},
	{"Find Selection", CCommand::cEditFindSel},
	{"Replace", CCommand::cEditReplace},
	{"Replace & Find Next", CCommand::cEditReplaceFindNext},
	{"Replace All %l", CCommand::cEditReplaceAll},
	{"Wrap Lines", CCommand::cDraftWrap},
	{"Unwrap Lines", CCommand::cDraftUnwrap},
	{"Quote Lines", CCommand::cDraftQuote},
	{"Unquote Lines", CCommand::cDraftUnquote},
	{"Requote Lines", CCommand::cDraftRequote},
	{"Shift Left", CCommand::cDraftShiftLeft},
	{"Shift Right %l", CCommand::cDraftShiftRight},
	{"Check Spelling %l", CCommand::cEditCheckSpelling},
	{"Capture Address", CCommand::cAddressesCaptureAddress},
	{NULL, NULL}
};

const SMenuBuilder cAdbkManagerContext[] = 
{
	{"New Address Book...", CCommand::cAddressesNew},
	{"Open Address Book...", CCommand::cAddressesOpen},
	{"Rename Addressbook... %l", CCommand::cAddressesRename},
	{"Import Addresses", CCommand::cAddressesImport},
	{"Export Addresses%l", CCommand::cAddressesExport},
	{"Delete Address Book... %l", CCommand::cAddressesDelete},
	{"Login...", CCommand::cAddressesLogin},
	{"Logout...", CCommand::cAddressesLogout},
	{"Refresh List %l", CCommand::cAddressesRefresh},
	{"Synchronise... %l", CCommand::cAddressesSynchronise},
	{"Search for Addresses... %l", CCommand::cAddressSearch},
	{"Details...", CCommand::cEditProperties},
	{NULL, NULL}
};

const SMenuBuilder cAdbkSearchContext[] = 
{
	{"Copy", CCommand::cEditCopy},
	{"Delete %l", CCommand::cEditDelete},
	{"Select All %l", CCommand::cEditSelectAll},
	{"New Message", CCommand::cAddressNewMessage},
	{NULL, NULL}
};

const SMenuBuilder cAdbkTableContext[] = 
{
	{"New Address...", CCommand::cAddressNew},
	{"Edit Address... %l", CCommand::cAddressEdit},
	{"Cut", CCommand::cEditCut},
	{"Copy", CCommand::cEditCopy},
	{"Paste", CCommand::cEditPaste},
	{"Delete %l", CCommand::cAddressDelete},
	{"New Message", CCommand::cAddressNewMessage},
	{NULL, NULL}
};

const SMenuBuilder cGroupTableContext[] = 
{
	{"New Group...", CCommand::cAddressNew},
	{"Edit Group... %l", CCommand::cAddressEdit},
	{"Cut", CCommand::cEditCut},
	{"Copy", CCommand::cEditCopy},
	{"Paste", CCommand::cEditPaste},
	{"Delete %l", CCommand::cAddressDelete},
	{"New Message", CCommand::cAddressNewMessage},
	{NULL, NULL}
};

const SMenuBuilder cCalendarStoreContext[] = 
{
	{"New Calendar...", CCommand::cCalendarCreate},
	{"Rename Calendar...", CCommand::cCalendarRename},
	{"Delete Calendar... %l", CCommand::cCalendarDelete},
	{"Add Web Calendar...", CCommand::cCalendarAddWeb},
	{"Refresh Web Calendar", CCommand::cCalendarRefreshWeb},
	{"Upload Web Calendar %l", CCommand::cCalendarUploadWeb},
	{"Refresh List %l", CCommand::cCalendarRefresh},
	{"Get Free/Busy Information... %l", CCommand::cCalendarFreeBusy},
	{"Send via Email %l", CCommand::cCalendarNewMessage},
	{"Import...", CCommand::cFileImport},
	{"Export... %l", CCommand::cFileExport},
	{"Details...", CCommand::cEditProperties},
	{NULL, NULL}
};

const SMenuBuilder cEventTableContext[] = 
{
	{"New Event...", CCommand::cCalendarNewEvent},
	{NULL, NULL}
};

const SMenuBuilder cToDoTableContext[] = 
{
	{"New To Do...", CCommand::cCalendarNewToDo},
	{NULL, NULL}
};

const SMenuBuilder cEventItemContext[] = 
{
	{"Edit", CCommand::cCalendarEditItem},
	{"Duplicate", CCommand::cCalendarDuplicateItem},
	{"Delete %l", CCommand::cCalendarDeleteItem},
	{"Send via Email", CCommand::cCalendarNewMessage},
	{"Invite Attendees", CCommand::cCalendarInvite},
	{NULL, NULL}
};

const SMenuBuilder cToDoItemContext[] = 
{
	{"Edit", CCommand::cCalendarEditItem},
	{"Duplicate", CCommand::cCalendarDuplicateItem},
	{"Delete %l", CCommand::cCalendarDeleteItem},
	{"Completed %l", CCommand::cCalendarCompleted},
	{"Send via Email", CCommand::cCalendarNewMessage},
	{"Invite Attendees", CCommand::cCalendarInvite},
	{NULL, NULL}
};
