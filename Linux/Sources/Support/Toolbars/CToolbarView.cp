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

#include "CDivider.h"
#include "CPreferences.h"
#include "CToolbar.h"

#include <algorithm>

// Static members
CToolbarView::CToolbarViewList CToolbarView::sToolbarViews;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CToolbarView::CToolbarView(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
	: JXEngravedRect(enclosure, hSizing, vSizing, x, y, w, h)
{
	// Add to list
	sToolbarViews.push_back(this);

	mSibling = NULL;
	mIsVisible = false;
	mActiveIndex = 0xFFFFFFFF;
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
void CToolbarView::OnCreate()
{
#if 0
	// Bottom divider
    CDivider* sep = new CDivider(this, JXWidget::kHElastic, JXWidget::kFixedBottom, 0, 52, 100, 2);
	sep->FitToEnclosure(kTrue, kFalse);
	JRect rect = GetAperture();
    sep = new CDivider(this, JXWidget::kFixedRight, JXWidget::kVElastic, GetApertureWidth() - 2, 52, 2, 52);
#endif
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
	toolbar->Hide();

	// Give commander to toolbar
	if (cmdr)
		toolbar->AddCommander(cmdr);

	// Make toolbar fit
	toolbar->FitToEnclosure(kTrue, kFalse);

	// Add toolbar
	mToolbars.push_back(toolbar);
	
	// Must listen to the toolbar for activate/deactivate
	toolbar->Add_Listener(this);

	// Now adjust size for toolbar
	AdjustToolbarSize();
}

void CToolbarView::ActivateToolbar(CToolbar* tb, bool show)
{
	// Find its index (it must be one we know about)
	SToolbars::const_iterator found = std::find(mToolbars.begin(), mToolbars.end(), tb);
	if (found == mToolbars.end())
		return;
	unsigned long index = found - mToolbars.begin();

	// Only activate - the previously active one will be hidden at the same time
	// so that the toolbar view size shouldn't need to be adjusted
	if (show && (mActiveIndex != index))
	{
		// Now switch the toolbars over
		mActiveIndex = index;

		// We need to update the state of the new activate toolbar
		tb->UpdateToolbarState();

		// Now readjust the size
		AdjustToolbarSize();
	}
}

void CToolbarView::ShowToolbar(unsigned long group, bool show)
{
	// Change state if different
	if (mIsVisible != show)
	{
		// Change it
		mIsVisible = show;
		
		// If its now visible we need to update its state
		if (show && (mActiveIndex != 0xFFFFFFFF))
		{
			// Show it then update it
			// Update only works if the toolbar is visible
			mToolbars[mActiveIndex]->ShowToolbar(true);
			mToolbars[mActiveIndex]->UpdateToolbarState();
		}

		// Now readjust the size
		AdjustToolbarSize();
	}
}

bool CToolbarView::IsVisible(unsigned long group) const
{
	// Return state
	return mIsVisible;
}

void CToolbarView::AdjustToolbarSize()
{
	// Get size of this one
	JRect psize = GetApertureGlobal();
	
	// Get visible toolbar
	CToolbar* tb = NULL;
	unsigned long index = 0;
	for(SToolbars::const_iterator iter = mToolbars.begin(); iter != mToolbars.end(); iter++, index++)
	{
		// Only bother if group is visible and this is the active one
		if (mIsVisible && (mActiveIndex == index))
			tb = *iter;
		else
			// Hide inactive ones
			(*iter)->ShowToolbar(false);
	}

	// Position each visible pane and note the total height (with extra for bottom space)
	JSize total_height = 1;
	JSize vpos = 0;
	JSize hpos = 0;
	if (tb)
	{
		// Position it
		tb->SetSize(psize.width(), tb->GetFrameHeight());
		tb->Place(0, 0);
		tb->ShowToolbar(true);
		total_height += tb->GetFrameHeight();
	}
	
	// Now adjust the size of this one
	JSize resizeby = total_height - psize.height();
	if (resizeby)
	{
		AdjustSize(0, resizeby);
		
		// Adjust sibling
		if (mSibling)
		{
			mSibling->AdjustSize(0, -resizeby);
			mSibling->Move(0, resizeby);
		}
	}
	
	Refresh();
}

// Force update of state of all visible toolbars
void CToolbarView::UpdateToolbars()
{
	for(SToolbars::const_iterator iter = mToolbars.begin(); iter != mToolbars.end(); iter++)
	{
		// Change state of each toolbar
		(*iter)->SmallIcons(CPreferences::sPrefs->mToolbarSmallIcons.GetValue());
		(*iter)->ShowIcons(CPreferences::sPrefs->mToolbarShowIcons.GetValue());
		(*iter)->ShowCaptions(CPreferences::sPrefs->mToolbarShowCaptions.GetValue());
	}
	
	// Change entire visible state
	ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());
}

// Force update of state of all visible toolbars
void CToolbarView::UpdateToolbarState()
{
	// Only bother if visible
	if (mIsVisible && (mActiveIndex != 0xFFFFFFFF))
		mToolbars[mActiveIndex]->UpdateToolbarState();
}
