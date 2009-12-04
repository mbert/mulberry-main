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

/******************************************************************************
 JXVertLockPartition.h

 ******************************************************************************/

#ifndef _H_JXVertLockPartition
#define _H_JXVertLockPartition

#include <JXVertPartition.h>

class JXVertLockPartition : public JXVertPartition
{
public:

	JXVertLockPartition(const JArray<JCoordinate>& heights, const JIndex elasticIndex,
					const JArray<JCoordinate>& minHeights, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);

	virtual ~JXVertLockPartition();

	bool	GetLocked() const
		{ return mLocked; }
	void	SetLocked(bool lockit)
		{ mLocked = lockit; }

protected:
	bool mLocked;

	virtual void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);

	virtual void	AdjustCursor(const JPoint& pt, const JXKeyModifiers& modifiers);
};

#endif
