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


// Header for attach files CustomGetFile

#ifndef __ATTACHSFFILTERS__MULBERRY__
#define __ATTACHSFFILTERS__MULBERRY__

#include "CAttachment.h"

// Consts
const short item_add = 1;
const short item_cancel = 2;
const short item_filelist = 7;
const short item_attachlist = 11;
const short item_addall = 12;
const short item_remove = 13;
const short item_removeall = 14;
const short item_done = 15;

// Types

class CAttachmentList;

typedef struct
{
	StandardFileReply	reply;
	CAttachmentList*	attachList;
	FSSpec				last_filtered;
} TAttachRec;

// Protos

pascal Boolean attachFileFilter(CInfoPBPtr pb,
								void *attach);				// File filter
pascal short attachDlgHook(short item,
							DialogPtr theDialog,
							void *attachList);				// Dialog hook
pascal Boolean attachModalFilter(DialogPtr theDialog,
									EventRecord *theEvent,
									short *itemHit,
									void *attach);			// Modal dialog filter
pascal void attachActivate(DialogPtr theDialog,
							short itemNo,
							Boolean activating,
							void *attach);					// Activate item

void CreateListItem(DialogPtr theDialog);							// Setup list item
pascal void ListItem(WindowPtr theWindow, short item);				// The list item draw callback

#endif
