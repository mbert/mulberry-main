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


// Source for CDragIt class

#include "CDragIt.h"

#include "CTableWindow.h"

// __________________________________________________________________________________________________
// C L A S S __ C D R A G I T
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CDragIt::CDragIt(const EventRecord	&inEventRecord)
		: LDragTask(inEventRecord)
{
}

// Default destructor
CDragIt::~CDragIt()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Do drag and return its status
OSErr CDragIt::DoDragIt(void)
{
	// Modified to get the mouse up location from the drag to enable delayedSelect dragging
	AddFlavors(mDragRef);
	MakeDragRegion(mDragRef, mDragRegion);
	OSErr err = ::TrackDrag(mDragRef, &mEventRecord, mDragRegion);
	::GetDragMouse(mDragRef, &CTableWindow::sMouseUp, NULL);
	return err;
}

// Add flavor to drag
void CDragIt::AddFlavorItem(
	const Rect& inItemRect,
	ItemReference inItemRef,
	FlavorType inFlavor,
	void* inDataPtr,
	Size inDataSize,
	FlavorFlags inFlags,
	Boolean addRect)
{
	::AddDragItemFlavor(mDragRef, inItemRef, inFlavor, inDataPtr, inDataSize, inFlags);

	if (addRect) {
		Rect	globalRect = inItemRect;
		::LocalToGlobal(&topLeft(globalRect));
		::LocalToGlobal(&botRight(globalRect));
		AddRectDragItem(inItemRef, globalRect);
	}
}

// Set new drag region
void CDragIt::SetDragRegion(RgnHandle dragRgn)
{
	mDragRegion = dragRgn;
}
