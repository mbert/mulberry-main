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

// CSplitter.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 19-May-2001
// Author: Cyrus Daboo
// Platforms: Unix
//
// Description:
// This class implements horiz or vert two pane splitter.
//

#ifndef __CSPLITTER__MULBERRY__
#define __CSPLITTER__MULBERRY__

#include <JXPartition.h>

class CSplitter : public JXPartition
{
public:

	CSplitter(const JArray<JCoordinate>& sizes, const JIndex elasticIndex,
					const JArray<JCoordinate>& minSizes, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);

	virtual ~CSplitter();

			void	InitViews(bool horiz);

			JXContainer* GetView1() const
				{ return mSub1; }
			JXContainer* GetView2() const
				{ return mSub2; }

			void	ShowView(bool view1, bool view2);
			void	SetMinima(long min1, long min2);
			void	SetLocks(bool lock1, bool lock2);
			void	SetElastic(bool elastic1, bool elastic2);

			void	SetLockSize(JCoordinate size);
			void	MoveSplitPos(JCoordinate moveby);
			
			long	GetRelativeSplitPos() const;
			void	SetRelativeSplitPos(long split);
			void	ChangedDividerPos(long change);

protected:

	virtual void	Draw(JXWindowPainter& p, const JRect& rect);
	virtual void	ApertureResized(const JCoordinate dw, const JCoordinate dh);

	virtual void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
	virtual void	HandleMouseDrag(const JPoint& pt, const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
	virtual void	HandleMouseUp(const JPoint& pt, const JXMouseButton button,
								  const JXButtonStates& buttonStates,
								  const JXKeyModifiers& modifiers);
	virtual void	AdjustCursor(const JPoint& pt, const JXKeyModifiers& modifiers);

	virtual JCoordinate		GetTotalSize() const;
	virtual JXWidgetSet*	CreateCompartment(const JCoordinate position,
									  const JCoordinate size);
	virtual void			SetCompartmentSizes();

protected:
	JXContainer*	mSub1;
	JXContainer*	mSub2;

	long			mDividerPos;
	long			mMin1;
	long			mMin2;
	bool			mHorizontal;
	bool			mVisible1;
	bool			mVisible2;
	bool			mLock1;
	bool			mLock2;
	bool			mElastic1;
	bool			mElastic2;

			void	GetSplitbarRect(JRect& splitter);

private:

	enum DragType
	{
		kInvalidDrag,
		kDragOne,
		kDragAll
	};

private:

	JCursorIndex	itsDragAllLineCursor;

	// used during drag

	DragType	itsDragType;
	JPoint		itsPrevPt;
	JCoordinate	itsMinDrag, itsMaxDrag;

private:

	// not allowed

	CSplitter(const CSplitter& source);
	const CSplitter& operator=(const CSplitter& source);
};

#endif
