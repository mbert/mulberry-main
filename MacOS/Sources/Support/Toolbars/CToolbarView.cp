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


// Source for CToolbarView class

#include "CToolbarView.h"

#include "CPreferences.h"
#include "CToolbar.h"

// Static members
CToolbarView::CToolbarViewList CToolbarView::sToolbarViews;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CToolbarView::CToolbarView(LStream *inStream)
		: LWindowHeader(inStream)
{
	// Add to list
	sToolbarViews.push_back(this);

	mSibling = NULL;
	mSplitter = NULL;
	mSideBySideWidth = 0;
}

// Default destructor
CToolbarView::~CToolbarView()
{
	// Remove from list
	CToolbarViewList::iterator found = std::find(sToolbarViews.begin(), sToolbarViews.end(), this);
	if (found != sToolbarViews.end())
		sToolbarViews.erase(found);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CToolbarView::UpdateAllToolbars()
{
	// Change state of each toolbar
	for(CToolbarViewList::iterator iter = sToolbarViews.begin(); iter != sToolbarViews.end(); iter++)
		(*iter)->UpdateToolbars();
}

// Do various bits
void CToolbarView::FinishCreateSelf()
{
	// Do inherited
	LWindowHeader::FinishCreateSelf();

	// Get splitter
	mSplitter = (LView*) FindPaneByID(paneid_CToolbarView_Splitter);
	if (mSplitter)
		mSplitter->Hide();
}

void CToolbarView::AdaptToSuperFrameSize(SInt32 inSurrWidthDelta, SInt32 inSurrHeightDelta, Boolean inRefresh)
{
	// Get size of this one
	SDimension16 size_before;
	GetFrameSize(size_before);

	// Do inherited
	LWindowHeader::AdaptToSuperFrameSize(inSurrWidthDelta, inSurrHeightDelta, inRefresh);
	
	// Get size of this one
	SDimension16 size_after;
	GetFrameSize(size_after);
	
	if ((size_before.width > mSideBySideWidth) ^ (size_after.width > mSideBySideWidth))
		AdjustSize();
}

void CToolbarView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CToolbar::eBroadcast_ToolbarActivate:
		// Show the toolbar
		ActivateToolbar(reinterpret_cast<CToolbar*>(param), true);
		break;
	case CToolbar::eBroadcast_ToolbarDeactivate:
		// Show the toolbar
		ActivateToolbar(reinterpret_cast<CToolbar*>(param), false);
		break;
	}
}		

void CToolbarView::AddToolbar(CToolbar* toolbar, LCommander* cmdr, unsigned long group)
{
	// Always starts out hidden
	toolbar->Hide();

	// Give commander to toolbar
	if (cmdr)
		toolbar->AddCommander(cmdr);

	// Make toolbar a child of the view
	toolbar->PutInside(this);
	ExpandSubPane(toolbar, true, false);

	// Find group entry
	SToolbarGroup* grp = FindGroup(group);
	
	// If none present, create one
	if (!grp)
	{
		// Create new group and add to list
		grp = new SToolbarGroup;
		mGroups.push_back(grp);
		
		// Initialise it
		grp->mGroupID = group;
		grp->mActiveIndex = 0;
		grp->mIsVisible = false;
	}

	// Add toolbar to group
	grp->mToolbars.push_back(toolbar);
	
	// Must listen to the toolbar for activate/deactivate
	toolbar->Add_Listener(this);

	// Now adjust size for toolbar
	AdjustSize();
}

void CToolbarView::ActivateToolbar(CToolbar* tb, bool show)
{
	// Find group entry
	unsigned long index = 0;
	SToolbarGroup* grp = FindGroup(tb, index);
	
	// Must be part of a group
	if (!grp)
		return;

	// Only activate - the previously active one will be hidden at the same time
	// so that the toolbar view size shouldn't need to be adjusted
	if (show && (grp->mActiveIndex != index))
	{
		// Now switch the toolbars over
		grp->mActiveIndex = index;

		// Only do visual updates if group is visible
		if (grp->mIsVisible)
		{		
			// We need to update the state of the new activate toolbar
			grp->mToolbars[index]->UpdateToolbarState();

			// Now readjust the size
			AdjustSize();
		}
	}
}

void CToolbarView::ShowToolbar(unsigned long group, bool show)
{
	// Find group entry
	SToolbarGroup* grp = FindGroup(group);
	
	// Must be a group
	if (!grp)
		return;

	// Change state if different
	if (grp->mIsVisible != show)
	{
		// Change it
		grp->mIsVisible = show;
		
		// If its now visible we need to update its state
		if (show)
		{
			// Show it then update it
			// Update only works if the toolbar is visible
			grp->mToolbars[grp->mActiveIndex]->ShowToolbar(true);
			grp->mToolbars[grp->mActiveIndex]->UpdateToolbarState();
		}

		// Now readjust the size
		AdjustSize();
	}
}

bool CToolbarView::IsVisible(unsigned long group) const
{
	// Find group entry
	SToolbarGroup* grp = FindGroup(group);
	
	// Must be a group
	if (!grp)
		return false;

	// Return state
	return grp->mIsVisible;
}

CToolbarView::SToolbarGroup* CToolbarView::FindGroup(unsigned long group) const
{
	// Look at each item in the list trying to find a match to the group id
	for(SToolbarGroups::const_iterator iter = mGroups.begin(); iter != mGroups.end(); iter++)
	{
		if ((*iter)->mGroupID == group)
			return *iter;
	}
	
	return NULL;
}

CToolbarView::SToolbarGroup* CToolbarView::FindGroup(CToolbar* tb, unsigned long& index) const
{
	// Look at each item in the list trying to find a match to the toolbar
	for(SToolbarGroups::const_iterator iter1 = mGroups.begin(); iter1 != mGroups.end(); iter1++)
	{
		index = 0;
		for(std::vector<CToolbar*>::const_iterator iter2 = (*iter1)->mToolbars.begin();
				iter2 != (*iter1)->mToolbars.end(); iter2++, index++)
		{
			if (*iter2 == tb)
				return *iter1;
		}
	}
	
	return NULL;
}

void CToolbarView::AdjustSize()
{
	// Get size of this one
	SDimension16 psize;
	GetFrameSize(psize);
	
	// Get visible toolbars and minimum widths
	std::vector<CToolbar*> tbs;
	unsigned long sidebyside_width = 0;
	for(SToolbarGroups::const_iterator iter1 = mGroups.begin(); iter1 != mGroups.end(); iter1++)
	{
		unsigned long index = 0;
		for(std::vector<CToolbar*>::const_iterator iter2 = (*iter1)->mToolbars.begin();
				iter2 != (*iter1)->mToolbars.end(); iter2++, index++)
		{
			// Only bother if group is visible and this is the active one
			if ((*iter1)->mIsVisible && ((*iter1)->mActiveIndex == index))
			{
				tbs.push_back(*iter2);
				sidebyside_width += (*iter2)->GetMinimumWidth();
			}
			else
				// Hide inactive ones
				(*iter2)->ShowToolbar(false);
		}
	}

	// Determine toolbar processing order
	std::vector<CToolbar*> tbrs;
	bool sidebyside = false;
	if (tbs.size() == 1)
		// Single bar all the way across
		tbrs.push_back(tbs[0]);

	else if (tbs.size() == 2)
	{
		// Check for side-by-side
		if (sidebyside_width <= psize.width)
		{
			// Two bars - second one first, first one next to it
			tbrs.push_back(tbs[1]);
			tbrs.push_back(tbs[0]);
			sidebyside = true;
		}
		else
		{
			// Two bars - first above second
			tbrs.push_back(tbs[0]);
			tbrs.push_back(tbs[1]);
		}
	}

	// Position each visible pane and note the total height
	SInt16 total_height = 0;
	SInt16 vpos = 0;
	SInt16 hpos = 0;
	for(unsigned long i = 0; i < tbrs.size(); i++)
	{
		// Get its size
		SDimension16 tbsize;
		tbrs[i]->GetFrameSize(tbsize);
		
		// Need extra space at the top
		if (i == 0)
		{
			total_height += 2;
			vpos += 2;
		}

		// Add splitter between two toolbars
		if (!sidebyside && (i > 0) && mSplitter)
		{
			mSplitter->PlaceInSuperFrameAt(0, vpos, false);
			mSplitter->Show();
			total_height += 3;
			vpos += 3;
		}

		// Position it below the last one
		tbrs[i]->ResizeFrameTo(sidebyside ? tbrs[i]->GetActualWidth() : psize.width, tbsize.height, false);
		tbrs[i]->PlaceInSuperFrameAt(hpos, vpos, false);
		tbrs[i]->ShowToolbar(true);

		// Adjust total height for next toolbar
		total_height = std::max((int) total_height, vpos + tbsize.height);
		vpos += (sidebyside ? 0 : tbsize.height);
		hpos += (sidebyside ? tbrs[i]->GetActualWidth() : 0);
	}
	
	// Hide splitter if less than two
	if ((sidebyside || (tbrs.size() < 2)) && mSplitter)
		mSplitter->Hide();
	
	// Need extra space at the bottom
	if (tbrs.size())
		total_height += 2;

	// Now adjust the size of this one
	SInt16 resizeby = total_height - psize.height;
	if (resizeby)
	{
		ResizeFrameBy(0, resizeby, true);
		
		// Adjust sibling
		if (mSibling)
		{
			mSibling->ResizeFrameBy(0, -resizeby, false);
			mSibling->MoveBy(0, resizeby, true);
		}
	}
	
	// Cache the minimum width
	mSideBySideWidth = (tbrs.size() > 1) ? sidebyside_width : 0;
	
	Refresh();
}

// Force update of state of all visible toolbars
void CToolbarView::UpdateToolbars()
{
	for(SToolbarGroups::const_iterator iter1 = mGroups.begin(); iter1 != mGroups.end(); iter1++)
	{
		for(std::vector<CToolbar*>::const_iterator iter2 = (*iter1)->mToolbars.begin();
				iter2 != (*iter1)->mToolbars.end(); iter2++)
		{
			// Change state of each toolbar
			(*iter2)->SmallIcons(CPreferences::sPrefs->mToolbarSmallIcons.GetValue());
			(*iter2)->ShowIcons(CPreferences::sPrefs->mToolbarShowIcons.GetValue());
			(*iter2)->ShowCaptions(CPreferences::sPrefs->mToolbarShowCaptions.GetValue());
		}
		
		// Change entire visible state
		ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());
	}
}

// Force update of state of all visible toolbars
void CToolbarView::UpdateToolbarState()
{
	// Refresh the active toolbar in each visible group
	for(SToolbarGroups::const_iterator iter = mGroups.begin(); iter != mGroups.end(); iter++)
	{
		// Only bother if group is visible
		if ((*iter)->mIsVisible)
			(*iter)->mToolbars[(*iter)->mActiveIndex]->UpdateToolbarState();
	}
}
