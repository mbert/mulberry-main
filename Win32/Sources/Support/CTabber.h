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


//	CTabber.h

#ifndef __CTABBER__MULBERRY__
#define __CTABBER__MULBERRY__

// Classes

class CTabber
{
public:
						CTabber();
	virtual				~CTabber();

	virtual void	SetTabOrder(CWnd* next,
								CWnd* prev,
								CView* viewNext,
								CView* viewPrev)
					{ mTabNext = next; mTabPrev = prev; 
						mTabViewNext = viewNext; mTabViewPrev = viewPrev; }

	virtual void	SetTabSelectAll(bool tab_select_all)
						{ mTabSelectAll = tab_select_all; }				// Set tab select all
	virtual bool	GetTabSelectAll(void) const {return mTabSelectAll;}	// Get tab select all

	virtual CWnd*	GetNextWnd()
		{ return mTabNext; }
	virtual CView*	GetNextView()
		{ return mTabViewNext; }
	virtual CWnd*	GetPrevWnd()
		{ return mTabPrev; }
	virtual CView*	GetPrevView()
		{ return mTabViewPrev; }

protected:
	virtual	void	DoTab();		// Handle tab

private:
	CWnd*	mTabNext;
	CWnd*	mTabPrev;
	CView*	mTabViewNext;
	CView*	mTabViewPrev;
	bool	mTabSelectAll;								// Tab selects all
};

#endif
