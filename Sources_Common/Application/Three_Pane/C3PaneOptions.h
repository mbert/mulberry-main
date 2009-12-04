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


// Header for C3PaneOptions class

#ifndef __C3PANEOPTIONS__MULBERRY__
#define __C3PANEOPTIONS__MULBERRY__

#include "CPreferenceItem.h"

#include "C3PaneWindowFwd.h"
#include "CMailViewOptions.h"
#include "CAddressViewOptions.h"
#include "CCalendarViewOptions.h"
#include "CUserAction.h"

// Classes

class C3PaneOptions : public CPreferenceItem
{
public:
	class C3PaneViewOptions
	{
		friend class C3PaneOptions;

	public:
		enum EUserAction
		{
			eListPreview = 0,
			eListFullView,
			eItemsPreview,
			eItemsFullView,
			eUserAction_Total
		};

		enum EViewFocus
		{
			eListView = 0,
			eItemsView,
			ePreviewView
		};

		C3PaneViewOptions();
		C3PaneViewOptions(const C3PaneViewOptions& copy)
			{ _copy(copy); }
		~C3PaneViewOptions();
	
		C3PaneViewOptions& operator=(const C3PaneViewOptions& copy)			// Assignment with same type
			{ if (this != &copy) _copy(copy); return *this; }

		void	SetGeometry(N3Pane::EGeometry geometry)
			{ mGeometry = geometry; }
		N3Pane::EGeometry	GetGeometry() const
			{ return mGeometry; }

		void	SetSplit1Pos(unsigned long pos)
			{ mSplit1Pos = pos; }
		unsigned long	GetSplit1Pos() const
			{ return mSplit1Pos; }

		void	SetSplit2Pos(unsigned long pos)
			{ mSplit2Pos = pos; }
		unsigned long	GetSplit2Pos() const
			{ return mSplit2Pos; }

		void	SetListVisible(bool visible)
			{ mListVisible = visible; }
		bool	GetListVisible() const
			{ return mListVisible; }

		void	SetItemsVisible(bool visible)
			{ mItemsVisible = visible; }
		bool	GetItemsVisible() const
			{ return mItemsVisible; }

		void	SetPreviewVisible(bool visible)
			{ mPreviewVisible = visible; }
		bool	GetPreviewVisible() const
			{ return mPreviewVisible; }

		void	SetFocusedPanel(EViewFocus focus)
			{ mFocusedPanel = focus; }
		EViewFocus	GetFocusedPanel() const
			{ return mFocusedPanel; }

		CUserAction& GetUserAction(EUserAction i)
			{ return mUserActions[i]; }
		const CUserAction& GetUserAction(EUserAction i) const
			{ return mUserActions[i]; }

	protected:
		N3Pane::EGeometry	mGeometry;			// Default geometry
		
		unsigned long	mSplit1Pos;			// Position for split1
		unsigned long	mSplit2Pos;			// Position for split2

		bool			mListVisible;		// List visible
		bool			mItemsVisible;		// Items visible
		bool			mPreviewVisible;	// Preview visible
		
		EViewFocus		mFocusedPanel;		// Which panel has focus - this is not stored in the prefs - just a runtime value

		CUserAction		mUserActions[eUserAction_Total];		// User actions to trigger viewing

		cdstring GetInfo() const;
		bool SetInfo(char_stream& info, NumVersion vers_prefs);

	private:
		void _copy(const C3PaneViewOptions& copy);
	};

	C3PaneOptions();
	C3PaneOptions(const C3PaneOptions& copy)
		{ _copy(copy); }
	virtual ~C3PaneOptions();

	C3PaneOptions& operator=(const C3PaneOptions& copy)			// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	void	SetInitialView(N3Pane::EViewType view)
		{ mInitialView = view; }
	N3Pane::EViewType	GetInitialView() const
		{ return mInitialView; }

	C3PaneViewOptions& GetViewOptions(N3Pane::EViewType view)
		{ return mViewOptions[view]; }
	const C3PaneViewOptions& GetViewOptions(N3Pane::EViewType view) const
		{ return mViewOptions[view]; }

	CMailViewOptions& GetMailViewOptions()
		{ return mMailViewOptions; }
	const CMailViewOptions& GetMailViewOptions() const
		{ return mMailViewOptions; }

	CAddressViewOptions& GetAddressViewOptions()
		{ return mAddressViewOptions; }
	const CAddressViewOptions& GetAddressViewOptions() const
		{ return mAddressViewOptions; }

	CCalendarViewOptions& GetCalendarViewOptions()
		{ return mCalendarViewOptions; }
	const CCalendarViewOptions& GetCalendarViewOptions() const
		{ return mCalendarViewOptions; }

	virtual cdstring GetInfo() const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

protected:
	N3Pane::EViewType		mInitialView;
	C3PaneViewOptions		mViewOptions[N3Pane::eView_Total];		// Array of options
	CMailViewOptions		mMailViewOptions;						// Options specific to mail view
	CAddressViewOptions		mAddressViewOptions;					// Options specific to address view
	CCalendarViewOptions	mCalendarViewOptions;					// Options specific to calendar view

	bool				mGeometriesPerView;			// Use separate geometries for each type of view
	bool				mActionsPerView;			// Use separate actions for each type of view

private:
	void _copy(const C3PaneOptions& copy);
};

#endif
