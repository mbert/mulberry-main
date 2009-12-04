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


#ifndef __WIN_LHIERARCHYTABLE__MULBERRY__
#define __WIN_LHIERARCHYTABLE__MULBERRY__

#include "CTableDragAndDrop.h"
#include <WIN_LCollapsableTree.h>

class	LHierarchyTable : public CTableDragAndDrop {

	DECLARE_DYNCREATE(LHierarchyTable)

public:

						LHierarchyTable();
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
						
	Boolean				IsCollapsable(
								UInt32			inWideOpenIndex) const
							{
								return mCollapsableTree->IsCollapsable(inWideOpenIndex);
							}
						
	void				MakeCollapsable(
								UInt32			inWideOpenIndex,
								Boolean			inCollapsable)
							{
								mCollapsableTree->MakeCollapsable(inWideOpenIndex, inCollapsable);
							}
						
	Boolean				IsExpanded(
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
								Boolean			inRefresh = false);
	virtual UInt32		InsertSiblingRows(
								UInt32			inHowMany,
								UInt32			inAfterRow,
								const void		*inDataPtr = nil,
								UInt32			inDataSize = 0,
								Boolean			inCollapsable = false,
								Boolean			inRefresh = false);
	virtual UInt32		InsertChildRows(
								UInt32			inHowMany,
								UInt32			inParentRow,
								const void		*inDataPtr = nil,
								UInt32			inDataSize = 0,
								Boolean			inCollapsable = false,
								Boolean			inRefresh = false);
	virtual UInt32		AddLastChildRow(
								UInt32			inParentRow,
								const void		*inDataPtr = nil,
								UInt32			inDataSize = 0,
								Boolean			inCollapsable = false,
								Boolean			inRefresh = false);
	virtual void		RemoveRows(
								UInt32			inHowMany,
								UInt32			inFromRow,
								Boolean			inRefresh);
	virtual void		RemoveAllRows(
								Boolean			inRefresh);
	
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
	afx_msg	void OnLButtonDblClk(UINT nFlags, CPoint point);			// Double-clicked item
	afx_msg	void OnLButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere

protected:
	LCollapsableTree*	mCollapsableTree;
	Rect				mFlagRect;
	int					mHierarchyCol;
	
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
	
	virtual Boolean		LClickHierarchyRow(UInt32 inAtRow, CPoint point, const CRect& inLocalRect);

	virtual int			DrawHierarchyRow(CDC* pDC, UInt32 inAtRow,
											const CRect& inLocalRect,
											const char* title,
											UINT iconID,
											bool clip_center = false);
	virtual int			DrawHierarchyRow(CDC* pDC, UInt32 inAtRow,
											const CRect& inLocalRect,
											const char* title,
											HICON hIcon,
											bool clip_center = false);

	DECLARE_MESSAGE_MAP()
};


#endif
