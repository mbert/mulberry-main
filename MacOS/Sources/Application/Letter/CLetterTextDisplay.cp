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


#include "CLetterTextDisplay.h"

#include "CAdminLock.h"
#include "CFileTable.h"
#include "CLetterWindow.h"
#include "CMulberryCommon.h"

CLetterTextDisplay::CLetterTextDisplay(LStream *inStream) : CEditFormattedTextDisplay(inStream)
{
}

CLetterTextDisplay::~CLetterTextDisplay()
{
}

// Get details of sub-panes
void CLetterTextDisplay::FinishCreateSelf(void)
{
	// Do inherited
	CEditFormattedTextDisplay::FinishCreateSelf();

	// Find window in super view chain
	mLetterWnd = (CLetterWindow*) mSuperView;
	while(mLetterWnd->GetPaneID() != paneid_LetterWindow)
		mLetterWnd = (CLetterWindow*) mLetterWnd->GetSuperView();

	// Set Drag & Drop info
	AddDropFlavor(cDragMsgType);
	AddDropFlavor(cDragAtchType);
	AddDropFlavor(cDragMsgAtchType);
	
	// Only if not locked out
	if (!CAdminLock::sAdminLock.mNoAttachments)
		AddDropFlavor(flavorTypeHFS);
}

// Handle multiple text items
void CLetterTextDisplay::DoDragReceive(DragReference inDragRef)
{
	bool did_attachment = false;

	// Toggle based on modifiers
	short mouseModifiers;
	::GetDragModifiers(inDragRef, &mouseModifiers, NULL, NULL);
	bool option_key = mouseModifiers & optionKey;

	// Don't do parts/attachment add if option key is down
	if (!option_key)
	{
		UInt16	itemCount;				// Number of Items in Drag
		::CountDragItems(inDragRef, &itemCount);

		// Look at all items in the drag
		for (UInt16 item = 1; item <= itemCount; item++)
		{
			ItemReference	itemRef;
			FlavorFlags		theFlags;		// We actually only use the flags to see if a flavor exists
			::GetDragItemReferenceNumber(inDragRef, item, &itemRef);

			// Look for attachment items
			FlavorType fileFlavors[4] = 
			{
				cDragMsgType, cDragAtchType, cDragMsgAtchType, flavorTypeHFS
			};
			
			for(int i = 0; i < 4; i++)
			{
				if (::GetFlavorFlags(inDragRef, itemRef, fileFlavors[i], &theFlags) == noErr)
				{
					// Calculate size
					Size theDataSize;
					ThrowIfOSErr_(::GetFlavorDataSize(inDragRef, itemRef, fileFlavors[i], &theDataSize));

					// Create data
					std::auto_ptr<char> theFlavorData(new char[theDataSize]);

					// Get the data about the attachment we are receiving.
					::GetFlavorData(inDragRef, itemRef, fileFlavors[i], theFlavorData.get(), &theDataSize, 0L);

					mLetterWnd->mPartsTable->DropData(fileFlavors[i], theFlavorData.get(), theDataSize);
					did_attachment = true;
					break;
				}
			}
		}
	}
	
	if (!did_attachment)
		CEditFormattedTextDisplay::DoDragReceive(inDragRef);
	else
		mLetterWnd->mPartsTable->ExposePartsList();
}

