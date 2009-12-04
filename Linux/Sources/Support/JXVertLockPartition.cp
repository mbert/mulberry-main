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


#include "JXVertLockPartition.h"

#include <jXGlobals.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

JXVertLockPartition::JXVertLockPartition
	(
	const JArray<JCoordinate>&	heights,
	const JIndex				elasticIndex,
	const JArray<JCoordinate>&	minHeights,
	JXContainer*				enclosure,
	const HSizingOption			hSizing,
	const VSizingOption			vSizing,
	const JCoordinate			x,
	const JCoordinate			y,
	const JCoordinate			w,
	const JCoordinate			h
	)
	:
	JXVertPartition(heights, elasticIndex, minHeights,
				enclosure, hSizing, vSizing, x,y, w,h)
{
	mLocked = false;
}

/******************************************************************************
 Destructor

 ******************************************************************************/

JXVertLockPartition::~JXVertLockPartition()
{
}

void
JXVertLockPartition::HandleMouseDown
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JSize				clickCount,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	// Don't do resize if locked
	if (mLocked)
		return;

	// Do inherited
	JXVertPartition::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
}

void
JXVertLockPartition::AdjustCursor
	(
	const JPoint&			pt,
	const JXKeyModifiers&	modifiers
	)
{
	// Don't change cursor if locked
	if (mLocked)
		DisplayCursor(kJXDefaultCursor);
	else
		JXVertPartition::AdjustCursor(pt, modifiers);
}
