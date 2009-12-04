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


#ifndef __UNX_LCOLLAPSABLETREE__MULBERRY__
#define __UNX_LCOLLAPSABLETREE__MULBERRY__

// ---------------------------------------------------------------------------

class	LCollapsableTree {
public:
						LCollapsableTree() { };
	virtual				~LCollapsableTree() { };
	
	virtual UInt32		CountNodes() const = 0;
	virtual UInt32		CountExposedNodes() const = 0;
	
	virtual UInt32		GetWideOpenIndex(
								UInt32			inExposedIndex) const = 0;
	virtual UInt32		GetExposedIndex(
								UInt32			inWideOpenIndex) const = 0;
	
	virtual UInt32		GetParentIndex(
								UInt32			inWideOpenIndex) const = 0;
	virtual UInt32		GetNestingLevel(
								UInt32			inWideOpenIndex) const = 0;
	virtual bool		IsCollapsable(
								UInt32			inWideOpenIndex) const = 0;
	virtual void		MakeCollapsable(
								UInt32			inWideOpenIndex,
								bool			inCollapsable) = 0;
	virtual bool		IsExpanded(
								UInt32			inWideOpenIndex) const = 0;
								
	virtual UInt32		CountAllDescendents(
								UInt32			inWideOpenIndex) const = 0;
	virtual UInt32		CountExposedDescendents(
								UInt32			inWideOpenIndex) const = 0;
	
	virtual UInt32		InsertSiblingNodes(
								UInt32			inHowMany,
								UInt32			inAfterIndex,
								bool			inCollapsable) = 0;
	virtual void		InsertChildNodes(
								UInt32			inHowMany,
								UInt32			inParentIndex,
								bool			inCollapsable) = 0;
	virtual UInt32		AddLastChildNode(
								UInt32			inParentIndex,
								bool			inCollapsable) = 0;
	virtual void		RemoveNode(
								UInt32			inWideOpenIndex,
								UInt32			&outTotalRemoved,
								UInt32			&outExposedRemoved) = 0;
	
	virtual UInt32		CollapseNode(
								UInt32			inWideOpenIndex) = 0;
	virtual UInt32		DeepCollapseNode(
								UInt32			inWideOpenIndex) = 0;
	
	virtual UInt32		ExpandNode(
								UInt32			inWideOpenIndex) = 0;
	virtual UInt32		DeepExpandNode(
								UInt32			inWideOpenIndex) = 0;
	virtual bool		HasSibling(
								UInt32			inWideOpenIndex) = 0;
	virtual bool		HasParent(
								UInt32			inWideOpenIndex) = 0;
	virtual bool		HasChildren(
								UInt32			inWideOpenIndex) = 0;
};

#endif
