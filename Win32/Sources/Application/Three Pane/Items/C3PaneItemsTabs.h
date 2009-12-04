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


// Header for C3PaneItemsTabs class

#ifndef __C3PANEITEMSTABS__MULBERRY__
#define __C3PANEITEMSTABS__MULBERRY__

// Classes

class C3PaneItems;

class C3PaneItemsTabs : public CTabCtrl
{
public:
		
	C3PaneItemsTabs()
		{ mOwner = NULL; mImages = NULL; mActive = false; }
	virtual ~C3PaneItemsTabs()
		{ delete mImages; }

	void	SetOwner(C3PaneItems* owner)
		{ mOwner = owner; }

	int		GetLastClicked() const
		{ return mLastClicked; }

	void	MoveTab(unsigned long oldindex, unsigned long newindex);

	void	SetActive(bool active)
		{ mActive = active; }
	bool	IsActive() const
		{ return mActive; }

protected:
	C3PaneItems*	mOwner;
	int				mLastClicked;
	CImageList*		mImages;
	bool			mActive;

	// Generated message map functions
	//{{AFX_MSG(CEditIdentities)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg	void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg	void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
