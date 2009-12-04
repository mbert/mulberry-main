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


#ifndef __UNX_LHIERARCHYTABLE__MULBERRY__
#define __UNX_LHIERARCHYTABLE__MULBERRY__

#include "CTableDragAndDrop.h"
#include <UNX_LCollapsableTree.h>

class JXImage;
typedef std::vector<JXImage*> CIconArray;

class	LHierarchyTable : public CTableDragAndDrop {

    enum
    {
      eHierUp = 0,
      eHierDown
    };

public:

	LHierarchyTable(JXScrollbarSet* scrollbarSet, 
					JXContainer* enclosure,
					const HSizingOption hSizing, 
					const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual				~LHierarchyTable();
	
	void				GetWideOpenTableSize(
								UInt32		&outRows) const;
								
	UInt32				GetWideOpenIndex(
								UInt32		inExposedIndex) const
							{
								return mCollapsableTree->GetWideOpenIndex(inExposedIndex);
							}
						
	UInt32				GetExposedIndex(
								UInt32		inWideOpenIndex) const
							{
								return mCollapsableTree->GetExposedIndex(inWideOpenIndex);
							}
	
	UInt32				GetParentIndex(
								UInt32			inWideOpenIndex) const
							{
								return mCollapsableTree->GetParentIndex(inWideOpenIndex);
							}
						
	UInt32				GetNestingLevel(
								UInt32			inWideOpenIndex) const
							{
								return mCollapsableTree->GetNestingLevel(inWideOpenIndex);
							}
						
	bool				IsCollapsable(
								UInt32			inWideOpenIndex) const
							{
								return mCollapsableTree->IsCollapsable(inWideOpenIndex);
							}
						
	void				MakeCollapsable(
								UInt32			inWideOpenIndex,
								bool			inCollapsable)
							{
								mCollapsableTree->MakeCollapsable(inWideOpenIndex, inCollapsable);
							}
						
	bool				IsExpanded(
								UInt32			inWideOpenIndex) const
							{
								return mCollapsableTree->IsExpanded(inWideOpenIndex);
							}
								
	UInt32				CountAllDescendents(
								UInt32			inWideOpenIndex) const
							{
								return mCollapsableTree->CountAllDescendents(inWideOpenIndex);
							}
						
	UInt32				CountExposedDescendents(
								UInt32			inWideOpenIndex) const
							{
								return mCollapsableTree->CountExposedDescendents(inWideOpenIndex);
							}
							
	virtual void		SetCollapsableTree(
								LCollapsableTree	*inTree);
								
	virtual void		InsertRows(
								UInt32			inHowMany,
								UInt32			inAfterRow,
								const void		*inDataPtr = nil,
								UInt32			inDataSize = 0,
								bool			inRefresh = false);
	virtual UInt32		InsertSiblingRows(
								UInt32			inHowMany,
								UInt32			inAfterRow,
								const void		*inDataPtr = nil,
								UInt32			inDataSize = 0,
								bool			inCollapsable = false,
								bool			inRefresh = false);
	virtual UInt32		InsertChildRows(
								UInt32			inHowMany,
								UInt32			inParentRow,
								const void		*inDataPtr = nil,
								UInt32			inDataSize = 0,
								bool			inCollapsable = false,
								bool			inRefresh = false);
	virtual UInt32		AddLastChildRow(
								UInt32			inParentRow,
								const void		*inDataPtr = nil,
								UInt32			inDataSize = 0,
								bool			inCollapsable = false,
								bool			inRefresh = false);
	virtual void		RemoveRows(
								UInt32			inHowMany,
								UInt32			inFromRow,
								bool			inRefresh);
	virtual void		RemoveAllRows(
								bool			inRefresh);
	
	virtual void		CollapseRow(
								UInt32			inWideOpenRow);
	virtual void		DeepCollapseRow(
								UInt32			inWideOpenRow);
	
	virtual void		ExpandRow(
								UInt32			inWideOpenRow);
	virtual void		DeepExpandRow(
								UInt32			inWideOpenRow);
	
	virtual void		RevealRow(
								UInt32					inWideOpenRow);

	// Key/Mouse related
	virtual void HandleMouseDown(const JPoint& pt, const JXMouseButton button,
			       const JSize clickCount, 
			       const JXButtonStates& buttonStates,
			       const JXKeyModifiers& modifiers);

protected:
	LCollapsableTree*	mCollapsableTree;
	Rect				mFlagRect;
	TableIndexT			mHierarchyCol;
	static CIconArray	sIcons;
	
	virtual void		RefreshRowsDownFrom(
								UInt32			inWideOpenRow);
	virtual void		RefreshRowsBelow(
								UInt32			inWideOpenRow);
	
	virtual void		ConcealRowsBelow(
								UInt32			inHowMany,
								UInt32			inWideOpenRow);
	virtual void		RevealRowsBelow(
								UInt32			inHowMany,
								UInt32			inWideOpenRow);
	
	virtual bool		LClickHierarchyRow(UInt32 inAtRow, JPoint point, const JRect& inLocalRect, const JXKeyModifiers& modifiers);
	virtual bool		LClickHierarchyRow(UInt32 inAtRow, JPoint point, const JRect& inLocalRect);

	virtual int			DrawHierarchyRow(JPainter* pDC, UInt32 inAtRow,
											const JRect& inLocalRect,
											const char* title,
											JXImage* iconID,
											bool clip_center = false);
};


#endif
