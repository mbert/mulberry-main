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


// Source for attach files CustomGetFile

#include "AttachSFFilters.h"

#include "CAttachmentList.h"
#include "CFileAttachment.h"
#include "CMulberryCommon.h"


#include <string.h>

UserItemUPP gUserItemUPP = nil;
ListHandle  gList = nil;
LArray*		gTempList = nil;

// File filter
pascal Boolean attachFileFilter(CInfoPBPtr cpb,
								void *vattach)
{
	// Get the FSSpec of the file to test
	TAttachRec *attach = (TAttachRec *)vattach;
	attach->last_filtered.vRefNum = cpb->hFileInfo.ioVRefNum;
	attach->last_filtered.parID = cpb->hFileInfo.ioFlParID;
	::BlockMoveData(cpb->hFileInfo.ioNamePtr, attach->last_filtered.name,
						cpb->hFileInfo.ioNamePtr[0]+1);

	// See whether its already in the list
	for(SInt32 i=1; i<=gTempList->GetCount(); i++)
	{
		FSSpec compareFile;
		gTempList->FetchItemAt(i, &compareFile);
		if ((attach->last_filtered.vRefNum == compareFile.vRefNum) &&
			 (attach->last_filtered.parID == compareFile.parID) &&
			 (::memcmp(attach->last_filtered.name, compareFile.name,
			 					attach->last_filtered.name[0]+1)==0))
		{

			 return true;
		}
	}

	return false;
}

// Dialog hook
pascal short attachDlgHook(short item,
							DialogPtr theDialog,
							void *vattach)
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	Cell	theCell;
	FSSpec	aFile;
	SInt32	i;
	TAttachRec *attach = (TAttachRec *)vattach;

	switch (item) {

		case sfHookFirstCall:
			// Create new temp list and copy items from real list
			gTempList = new LArray(sizeof(FSSpec));
			for(i = 1; i <= attach->attachList->size(); i++) {
				CFileAttachment* attached = (CFileAttachment*) attach->attachList->at(i-1);
				aFile = *attached->GetFSSpec();
				gTempList->InsertItemsAt(1, LArray::index_Last, &aFile);
			}

			// Create display list
			CreateListItem(theDialog);
			gUserItemUPP = NewUserItemUPP(ListItem);
			::GetDialogItem(theDialog, item_attachlist, &iType, &iHandle, &iRect);
			::SetDialogItem(theDialog, item_attachlist, iType, (Handle) gUserItemUPP, &iRect);
			break;

		case sfHookLastCall:
			// Dispose of all lists
			::LDispose(gList);
			gList = nil;
			delete gTempList;
			gTempList = nil;
			DisposeUserItemUPP(gUserItemUPP);
			break;

		case sfItemOpenButton:
			// Get selected file
			aFile.vRefNum = attach->reply.sfFile.vRefNum;
			aFile.parID = attach->reply.sfFile.parID;
			::BlockMoveData(attach->reply.sfFile.name, aFile.name, attach->reply.sfFile.name[0]+1);

			// Add to temp list class
			gTempList->InsertItemsAt(1, LArray::index_Last, &aFile);

			// Add to display list
			theCell.v = ::LAddRow(1, 0x7FFF, gList);
			theCell.h = 0;
			::LSetCell(&aFile.name[1], aFile.name[0], theCell, gList);
			return sfHookRebuildList;

		case sfHookOpenAlias:
			// Get selected file
			aFile.vRefNum = attach->reply.sfFile.vRefNum;
			aFile.parID = attach->reply.sfFile.parID;
			::BlockMoveData(attach->reply.sfFile.name, aFile.name, attach->reply.sfFile.name[0]+1);

			// Resolve alias
			Boolean folder;
			Boolean wasAliased;
			OSErr err = ::ResolveAliasFile(&aFile,true,&folder,&wasAliased);

			// Check whether alias was folder or for error
			if (!folder && (err==noErr)) {
				// Alias was file

				// See whether its already in the list
				Boolean duplicate = false;
				for(SInt32 i=1; i<=gTempList->GetCount(); i++) {
					FSSpec compareFile;
					gTempList->FetchItemAt(i, &compareFile);
					if ((aFile.vRefNum == compareFile.vRefNum) &&
						 (aFile.parID == compareFile.parID) &&
						 (::memcmp(aFile.name, compareFile.name, aFile.name[0]+1)==0)) {

						 duplicate = true;
						 break;
					}
				}

				// Add file if not duplicate
				if (!duplicate) {
					// Add to temp list class
					gTempList->InsertItemsAt(1, LArray::index_Last, &aFile);

					// Add to display list
					theCell.v = ::LAddRow(1, 0x7FFF, gList);
					theCell.h = 0;
					::LSetCell(&aFile.name[1], aFile.name[0], theCell, gList);
				}
				return sfHookRebuildList;
			}
			else
				// Follow directory alias to its directory in normal way
				return sfHookOpenAlias;

		case item_addall:
			// Loop over all items in directory
			CInfoPBRec cpb;
			Str255 name;

			cpb.hFileInfo.ioVRefNum = attach->last_filtered.vRefNum;
			cpb.hFileInfo.ioNamePtr = name;

			Boolean added = false;
			short index = 1;

			while (true) {
				OSErr err;
				cpb.hFileInfo.ioDirID = LMGetCurDirStore();
				cpb.hFileInfo.ioFDirIndex = index++;
				err = ::PBGetCatInfoSync(&cpb);
				if (err) break;

				// Try adding if not directory
				if (!(cpb.hFileInfo.ioFlAttrib & (1 << ioDirFlg))) {
					aFile.vRefNum = cpb.hFileInfo.ioVRefNum;
					aFile.parID = cpb.hFileInfo.ioFlParID;
					::PLstrcpy(aFile.name, cpb.hFileInfo.ioNamePtr);
					// See whether its already in the list
					Boolean duplicate = false;
					for(SInt32 i=1; i<=gTempList->GetCount(); i++) {
						FSSpec compareFile;
						gTempList->FetchItemAt(i, &compareFile);
						if ((aFile.vRefNum == compareFile.vRefNum) &&
							 (aFile.parID == compareFile.parID) &&
							 (::memcmp(aFile.name,
							 			compareFile.name,
							 			cpb.hFileInfo.ioNamePtr[0]+1)==0)) {

							 duplicate = true;
							 break;
						}
					}

					// Add file if not duplicate
					if (!duplicate) {
						// Add to temp list class
						gTempList->InsertItemsAt(1, LArray::index_Last, &aFile);

						// Add to display list
						theCell.v = ::LAddRow(1, 0x7FFF, gList);
						theCell.h = 0;
						::LSetCell(&aFile.name[1], aFile.name[0], theCell, gList);

						added = true;
					}
				}
			}

			return (added) ? sfHookRebuildList : sfHookNullEvent;

		case item_remove:
			// Find a selection
			theCell.v = 0;
			theCell.h = 0;
			if (::LGetSelect(true, &theCell, gList)) {

				// Delete it from each list
				gTempList->RemoveItemsAt(1, theCell.v+1);
				::LDelRow(1, theCell.v, gList);
				return sfHookRebuildList;
			}
			return sfHookNullEvent;

		case item_removeall:
			SInt32 num = gTempList->GetCount();
			for(i = num; i >= 1; i--) {

				// Delete it from each list
				gTempList->RemoveItemsAt(1, i);
				::LDelRow(1, i-1, gList);
			}
			if (num>0)
				return sfHookRebuildList;
			else
				return sfHookNullEvent;

		case item_done:
			// Copy items in temp list to real list
			attach->attachList->DeleteAll();
			for(i = 1; i <= gTempList->GetCount(); i++)
			{
				gTempList->FetchItemAt(i, &aFile);

				// Create new file attachment and add to list
				CFileAttachment* attached;
				attached = new CFileAttachment(aFile);
				attach->attachList->push_back(attached);
			}
			return sfItemOpenButton;

	}

	return item;
}

// Modal dialog filter
pascal Boolean attachModalFilter(DialogPtr theDialog,
									EventRecord *theEvent,
									short *itemHit,
									void *vattach)
{
	GrafPtr	savePort;
	Point	locationPt;
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	TAttachRec *attach = (TAttachRec *)vattach;

	// Save & set ports
	::GetPort(&savePort);
	::SetPort(theDialog);

    if ((theEvent->what == updateEvt) && (theEvent->message == (long) theDialog)) {

        // Update the list.
        ::LUpdate(theDialog->visRgn, gList);

	}

	// Handle click in user List
	else if (theEvent->what == mouseDown) {
		locationPt = theEvent->where;
		::GlobalToLocal(&locationPt);

		::GetDialogItem(theDialog, item_attachlist, &iType, &iHandle, &iRect);

		if (::PtInRect(locationPt, &iRect))
			::LClick(locationPt, theEvent->modifiers, gList);
	}

	// Reset port
	::SetPort(savePort);

	return false;
}

// Acivate items
pascal void attachActivate(DialogPtr theDialog,
							short itemNo,
							Boolean activating,
							void *vattach)
{
	GrafPtr	savePort;
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	TAttachRec *attach = (TAttachRec *)vattach;

	// Save & set ports
	::GetPort(&savePort);
	::SetPort(theDialog);

	// Get frame of item
	::GetDialogItem(theDialog,itemNo,&iType,&iHandle,&iRect);

	// Expand
	::InsetRect(&iRect, -3, -3);

	// Fix for color icon LDEF
	if ((itemNo==7) && (UEnvironment::HasGestaltAttribute(gestaltStandardFileAttr,
											gestaltStandardFileHasColorIcons)))
		iRect.bottom -= 2;

	// Make pen bigger
	::PenSize(2,2);

	// Set fore color based on activate/deactivate
	RGBColor	color;
	if (activating) {
		// Make it black
		color.red = 0x0000;
		color.green = 0x0000;
		color.blue = 0x0000;
	}
	else {
		// Make it white
		color.red = 0xFFFF;
		color.green = 0xFFFF;
		color.blue = 0xFFFF;
	}
	::RGBForeColor(&color);

	// Frame it
	::FrameRect(&iRect);

	// Reset port
	::SetPort(savePort);
}

// Setup list item
void CreateListItem(DialogPtr theDialog)
{
	short		iType;
	Handle		iHandle;
	Rect		iRect;

	// Get details of user item
	::GetDialogItem(theDialog, item_attachlist, &iType, &iHandle, &iRect);
	Rect rView = iRect;
	rView.right -= 15;
	::InsetRect(&rView, 1, 1);

	Point cSize;
	cSize.h = rView.right - rView.left;
	cSize.v = 16;

	Rect dataBounds = {0, 0, 0, 0};

	// Create new list with 1 column & various flags
	gList = ::LNew(&rView, &dataBounds, cSize, 0, theDialog, true, false, false, true);
	(**gList).selFlags = lNoNilHilite + lNoExtend + lUseSense;
	(**gList).listFlags = lDoVAutoscroll;
	::LAddColumn(1, 0, gList);
	::LActivate(true, gList);

	// Add the attached file names to the list
	for(SInt32 i=1; i<=gTempList->GetCount(); i++) {
		FSSpec	theFile;
		Cell	theCell;

		gTempList->FetchItemAt(i, &theFile);
		theCell.v = ::LAddRow(1, 0x7FFF, gList);
		theCell.h = 0;
		::LSetCell(&theFile.name[1], theFile.name[0], theCell, gList);
	}

}

// The list item draw callback
pascal void ListItem(WindowPtr theWindow, short item)
{
	short		iType;
	Handle		iHandle;
	Rect		iRect;

	// Get details of user item
	::GetDialogItem(theWindow, item_attachlist, &iType, &iHandle, &iRect);
	::FrameRect(&iRect);
}