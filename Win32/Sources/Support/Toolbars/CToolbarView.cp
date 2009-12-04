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

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CToolbar.h"

#include "StValueChanger.h"

BEGIN_MESSAGE_MAP(CToolbarView, CControlBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// Static members
CToolbarView::CToolbarViewList CToolbarView::sToolbarViews;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CToolbarView::CToolbarView()
{
	// Add to list
	sToolbarViews.push_back(this);

	mSibling = NULL;
	mHeight = 0;
	mSideBySideWidth = 0;
	mShowDivider = false;
}

// Default destructor
CToolbarView::~CToolbarView()
{
	// Remove from list
	CToolbarViewList::iterator found = ::find(sToolbarViews.begin(), sToolbarViews.end(), this);
	if (found != sToolbarViews.end())
		sToolbarViews.erase(found);
	
	// Always delete the toolbars
	for(SToolbarGroups::const_iterator iter1 = mGroups.begin(); iter1 != mGroups.end(); iter1++)
	{
		for(vector<CToolbar*>::iterator iter2 = (*iter1)->mToolbars.begin(); iter2 != (*iter1)->mToolbars.end(); iter2++)
			delete *iter2;
		(*iter1)->mToolbars.clear();
	}
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CToolbarView::UpdateAllToolbars()
{
	// Change state of each toolbar
	for(CToolbarViewList::iterator iter = sToolbarViews.begin(); iter != sToolbarViews.end(); iter++)
		(*iter)->UpdateToolbars();
}

int CToolbarView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set initial size for control repositioner
	InitResize(lpCreateStruct->cx, lpCreateStruct->cy);

	// Create divider
	mDivider.Create(_T(""), WS_CHILD | SS_ETCHEDHORZ, CRect(0, 0, lpCreateStruct->cx, 2), this, IDC_STATIC);
	AddAlignment(new CWndAlignment(&mDivider, CWndAlignment::eAlign_TopWidth));

	return 0;
}

// Resize sub-views
void CToolbarView::OnSize(UINT nType, int cx, int cy)
{
	// Adjust toolbars after resizing this window
	CControlBar::OnSize(nType, cx, cy);

	int width_before = mCurrentWidth;

	// Resize child windows
	SizeChanged(cx, cy);

	int width_after = mCurrentWidth;

	if ((width_before > mSideBySideWidth) ^ (width_after > mSideBySideWidth))
		AdjustSize();
}

CSize CToolbarView::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CRect rect;
	GetWindowRect(rect);
	if (bStretch) // if not docked stretch to fit
		return CSize(bHorz ? 32767 : rect.Width(), bHorz ? mHeight : 32767);
	else
		return CSize(rect.Width(), mHeight);
}

void CToolbarView::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	// Does nothing - CToolbar class deals with updating itself
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

void CToolbarView::AddToolbar(CToolbar* toolbar, CCommander* cmdr, unsigned long group)
{
	// Always starts out hidden
	toolbar->ShowToolbar(false);

	// Give commander to toolbar
	if (cmdr)
		toolbar->AddCommander(cmdr);

	// Make toolbar a child of the view
	toolbar->SetParent(this);
	::ExpandChildToFit(this, toolbar, true, false);

	// Add aligner for toolbar
	AddAlignment(new CWndAlignment(toolbar, CWndAlignment::eAlign_TopWidth));

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
	else if (show && grp->mIsVisible)
		// We need to update the state of the active toolbar
		grp->mToolbars[grp->mActiveIndex]->UpdateToolbarState();
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

void CToolbarView::ShowDivider(bool show)
{
	if (mShowDivider != show)
	{
		mShowDivider = show;
		
		// Always hide it here
		if (!mShowDivider)
			mDivider.ShowWindow(SW_HIDE);

		// Now readjust the size
		AdjustSize();
	}
		
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
		for(vector<CToolbar*>::const_iterator iter2 = (*iter1)->mToolbars.begin();
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
	CRect client;
	GetClientRect(client);
	
	// Get visible toolbars and minimum widths
	vector<CToolbar*> tbs;
	unsigned long sidebyside_width = 0;
	for(SToolbarGroups::const_iterator iter1 = mGroups.begin(); iter1 != mGroups.end(); iter1++)
	{
		unsigned long index = 0;
		for(vector<CToolbar*>::const_iterator iter2 = (*iter1)->mToolbars.begin();
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
	vector<CToolbar*> tbrs;
	bool sidebyside = false;
	if (tbs.size() == 1)
		// Single bar all the way across
		tbrs.push_back(tbs[0]);

	else if (tbs.size() == 2)
	{
		// Check for side-by-side
		if (sidebyside_width <= client.Width())
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
	long total_height = 0;
	long vpos = 0;
	long hpos = 0;
	for(unsigned long i = 0; i < tbrs.size(); i++)
	{
		// Get its size
		CRect tbsize;
		tbrs[i]->GetWindowRect(tbsize);
		
		// Position it below the last one
		tbrs[i]->SetWindowPos(NULL, hpos, vpos, sidebyside ? tbrs[i]->GetActualWidth() : client.Width(), tbsize.Height(), SWP_NOZORDER);
		tbrs[i]->ShowToolbar(true);

		// Adjust total height for next toolbar
		total_height = max(total_height, vpos + tbsize.Height());
		vpos += (sidebyside ? 0 : tbsize.Height());
		hpos += (sidebyside ? tbrs[i]->GetActualWidth() : 0);
	}
	
	// Adjust for divider
	if (mShowDivider)
	{
		mDivider.ShowWindow(SW_SHOW);
		mDivider.SetWindowPos(NULL, 0, vpos, client.Width(), 2, SWP_NOZORDER);
		total_height += 4;
		vpos += 4;
	}

	// Add one pixel at bottom if one or more visible
	mHeight = total_height + (tbrs.size() ? 1 : 0);
		
	// Now adjust the size of this one
	long resizeby = total_height - client.Height();
	if (resizeby)
	{
		::ResizeWindowBy(this, 0, resizeby, true);

		GetParentFrame()->RecalcLayout();
	}
	
	// Cache the minimum width
	mSideBySideWidth = (tbrs.size() > 1) ? sidebyside_width : 0;
}

// Force update of state of all visible toolbars
void CToolbarView::UpdateToolbars()
{
	// Stop redraws on parent frame
	BOOL locked = GetParentFrame()->LockWindowUpdate();

	for(SToolbarGroups::const_iterator iter1 = mGroups.begin(); iter1 != mGroups.end(); iter1++)
	{
		for(vector<CToolbar*>::const_iterator iter2 = (*iter1)->mToolbars.begin();
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

	if (locked)
		GetParentFrame()->UnlockWindowUpdate();
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