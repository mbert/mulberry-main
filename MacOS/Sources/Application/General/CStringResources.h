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


// Header for CStringResources.h class

#ifndef __CSTRINGRESOURCES__MULBERRY__
#define __CSTRINGRESOURCES__MULBERRY__

// Maps Windows/Unix string ids to Mac OS STR# resource specs

#define IDCANCEL					kCancel_Btn

// These are CErrorDialog
#define IDE_ExpungeOnClose			str_ExpungeOnClose
#define IDE_ExpungeManual			str_ExpungeManual
#define IDE_ClearINBOX				str_ClearINBOX
#define IDE_ClearINBOXDeleted		str_ClearINBOXDeleted
#define IDE_ServerClose				str_ServerClose
#define IDE_ServerLogoutQuit		str_ServerLogoutQuit
#define IDE_LetterServerLogon		str_LetterServerLogon
#define IDE_MultiSave				str_MultiSave
#define IDE_MultiReply				str_MultiReply
#define IDE_MultiForward			str_MultiForward
#define IDE_MultiDraft				str_MultiDraft
#define IDE_CachedSort				str_CachedSort
#define IDE_NoLocalDraftSave		str_NoLocalDraftSave
#define IDE_PromptDisconnect		str_PromptDisconnect
#define IDE_PromptWaitPostpone		str_PromptWaitPostpone
#define IDE_DeleteMboxOrFavourite	str_DeleteMboxOrFavourite
#define IDE_ReparseLocal			str_ReparseLocal
#define IDE_RulesMerge				str_RulesMerge
#define IDE_WarnReplySubject		str_WarnReplySubject

// These come from CServerTableCommon
#define IDR_MAINFRAME				STRx_Standards, 1

// These come from CLetterWindowCommon
#define IDE_MissingAttachments			STRx_LetterErrors, str_MissingAttachments
#define IDE_MissingAttachmentsDontShow	str_MissingAttachmentsDontShow
#define IDE_UnencryptedSend				STRx_LetterErrors, str_UnencryptedSend
#define IDE_UnencryptedSendDontShow		str_UnencryptedSendDontShow

#endif
